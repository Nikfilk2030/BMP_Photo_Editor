#include "bitmap.h"

#include <fstream>
#include <stdexcept>

//TODO В заголовке плохо подключать except

bool Bitmap::Load(const char* file_name) {
    std::fstream file;
    file.open(file_name, std::ios_base::in | std::ios_base::binary); // Открываем файл на чтение как бинарный
    if (!file.is_open()) {
        return false;
        // TODO обработать эту ситуацию
    }
    Load(file);
    file.close();
    return true;
}

// TODO Проверки, что поток не сломался, правильный файл итп.
// if(stream) - поток неявно может быть приведён к логическому выражению.
// Если этот if возвращает false - поток сломался (мы не можем из него больше читать).
// Работать это может так: while(std::getline(stream, line)) {...}.
// if(stream) не проверяет, закончился ли файл, поэтому пишут так:
// while(std::getline(stream, line) && !stream.eof()) {...}
// eof - end of file.
bool Bitmap::Load(std::istream& stream) {
    // TODO Никак не обрабатывается ситуация со сломанным потоком (assert?)
    stream.read(reinterpret_cast<char*> (&bmp_header_), sizeof(bmp_header_));
    stream.read(reinterpret_cast<char*> (&dib_header_), sizeof(dib_header_));

    const size_t RUBBISH_BETWEEN_PIXEL_LINES = (4 - (dib_header_.width * 3) % 4) % 4;
    pixels_.SetHeight(dib_header_.height);
    pixels_.SetWidth(dib_header_.width);
    for (size_t row = 0; row < dib_header_.height; ++row) {
        for (size_t column = 0; column < dib_header_.width; ++column) {
            Pixel pixel{};
            stream.read(reinterpret_cast<char*> (&pixel), sizeof(pixel));
            pixels_.PushBack(pixel);
        }
        stream.ignore(RUBBISH_BETWEEN_PIXEL_LINES); // Тут тип должен быть long long вроде, а у меня size_t
    }

    return true;
}

bool Bitmap::WriteFile(const char* file_name) {
    std::ofstream file;
    file.open(file_name, std::ios_base::out | std::ios_base::binary); // Открываем файл на вписывание как бинарный
    if (!file.is_open()) {
        return false;
        // TODO обработать эту ситуацию
    }
    WriteFile(file);
    file.close();
    return true;
}

bool Bitmap::WriteFile(std::ostream& stream) {
    stream.write(reinterpret_cast<char*> (&bmp_header_), sizeof(bmp_header_));
    stream.write(reinterpret_cast<char*> (&dib_header_), sizeof(dib_header_));

    const size_t RUBBISH_BETWEEN_PIXEL_LINES = (4 - (dib_header_.width * 3) % 4) % 4;
    for (size_t row = 0; row < dib_header_.height; ++row) {
        for (size_t column = 0; column < dib_header_.width; ++column) {
            stream.write(reinterpret_cast<char*> (&pixels_.GetPixel(row, column)),
                         sizeof(pixels_.GetPixel(row, column)));
        }
        // В оригинальном файле мог быть мусор, поэтому дальше мы должны вернуть мусор на место
        static const uint32_t RUBBISH = 0x55AA55AA;
        stream.write(reinterpret_cast<const char*> (&RUBBISH), RUBBISH_BETWEEN_PIXEL_LINES);
    }
    return true;
}

NewPixelArray::NewPixelArray(const NewPixelArray& other) : NewPixelArray() { // Делегирующий вызов конструктора
    if (!other.storage_) {
        return;
    }
    storage_ = AllocateStorage(other.height_, other.width_);
    height_ = other.height_;
    width_ = other.width_;
    other.CopyStorage(storage_, height_, width_, {0, 0, 0});
}

NewPixelArray& NewPixelArray::operator=(const NewPixelArray& rhv) {
    if (&rhv == this) {
        return *this;
    }
    // текущий this не должен испортиться
    NewPixelArray temporary(rhv);
    // Если ошибка и есть, то до этой строчки мы и дойдём, ниже уже умрём.
    Swap(temporary);
    return *this;
    // TODO дз: проверить, что работает
}

// Конспект от семинара 16.03:
void NewPixelArray::Resize(size_t height, size_t width, Bitmap::Pixel default_pixel) {
    if (height_ == height && width_ == width) {
        return;
    }
    if (height == 0 || width == 0) {
        FreeStorage();
        return;
    }
    Bitmap::Pixel* new_pixels = AllocateStorage(height, width);
    CopyStorage(new_pixels, height, width, default_pixel);
    FreeStorage();
    storage_ = new_pixels;
    height_ = height;
    width_ = width;
}

// При изменении пин точки поправить координаты (Например начинать row не с нуля, смотреть на "ширина минус старая" итп
// Как пин точку сделать? Можно задать 4 угла + центр, координаты или что-то другое, надо думать.
void NewPixelArray::CopyStorage(Bitmap::Pixel* new_storage, size_t new_height, size_t new_width,
                                Bitmap::Pixel default_pixel) const {
    for (size_t row = 0; row < new_height; ++row) {
        for (size_t column = 0; column < new_width; ++column) {
            if (row < height_ && column < width_) {
                GetPixel(new_storage, new_width, row, column) = GetPixel(storage_, width_, row, column);
            } else {
                GetPixel(new_storage, new_width, row, column) = default_pixel;
            }
        }
    }
}

Bitmap::Pixel& NewPixelArray::At(size_t row, size_t column) {
    if (row >= height_ || column >= width_) {
        throw std::out_of_range("Invalid row or column");
    }
    return GetPixel(storage_, width_, row, column);
}

const Bitmap::Pixel& NewPixelArray::At(size_t row, size_t column) const {
    if (row >= height_ || column >= width_) {
        throw std::out_of_range("Invalid row or column");
    }
    return GetPixel(storage_, width_, row, column);
}

NewPixelArray::NewPixelArray(NewPixelArray&& other) {
    storage_ = other.storage_;
    width_ = other.width_;
    height_ = other.height_;
    other.storage_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
}
// TODO потестить



