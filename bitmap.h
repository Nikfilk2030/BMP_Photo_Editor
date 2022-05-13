#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <tuple>

class NewPixelArray;

class Bitmap {
    // Структуры просто на поиграться с padding:
public:
    struct TestHeader {
        unsigned char field1; // __attribute__((__packed__));
        unsigned short field2; //__attribute__((__packed__));
        unsigned long field3; //__attribute__((__packed__));
    };
    struct TestHeader2 {
        unsigned char field1; // __attribute__((__packed__));
        unsigned short field2; //__attribute__((__packed__));
        unsigned long field3; //__attribute__((__packed__));
    } __attribute__((__packed__));

public:
    struct BMPHeader {
        uint16_t signature;
        uint32_t file_size;
        uint32_t dummy;
        uint32_t bitarray_offset;
    } __attribute__((__packed__));

    struct DIBHeader {
        uint32_t dib_header_size;
        uint32_t width;  // Ширина
        uint32_t height;  // Высота
        uint64_t dummy;  // Это же нам не нужно?
        uint32_t raw_bitmap_data_size;  // (including padding)
        uint64_t dummy2;  // Это же нам не нужно?
        uint64_t dummy3;  // Это же нам не нужно?
    } __attribute__((__packed__));

    struct Pixel {
        uint8_t blue = 0;
        uint8_t green = 0;
        uint8_t red = 0;

        bool operator==(const Pixel& rhv) const {
            return std::tie(blue, green, red) == std::tie(rhv.blue, rhv.green, rhv.red);
        }

        bool operator!=(const Pixel& rhv) const {
            return !(*this == rhv);
        }
    };

public:
    void SetFileWidth(size_t new_width) {
        dib_header_.width = new_width;
    }

    void SetFileHeight(size_t new_height) {
        dib_header_.height = new_height;
    }

    Pixel& GetPixel(size_t row, size_t column) {
        return pixels_.GetPixel(row, column);
    }

    const Pixel& GetPixel(size_t row, size_t column) const {
        return pixels_.GetPixel(row, column);
    }

    uint32_t GetHeight() const {
        return pixels_.GetHeight();
    }

    uint32_t GetWidth() const {
        return pixels_.GetWidth();
    }

    // Возвращает истину, если картина пустая, иначе - ложь
    bool IsEmpty() const {
        return pixels_.IsEmpty();
    }

public:
    class PixelArray {
    public:
        PixelArray() {}

        void PushBack(Pixel pixel) {
            pixel_array_elements_.push_back(pixel);
        }

        Pixel& GetPixel(size_t row, size_t column) {
            return pixel_array_elements_[pixel_array_width_ * row + column];
        }

        void SetHeight(size_t row) {
            pixel_array_height_ = row;
        }

        void SetWidth(size_t column) {
            pixel_array_width_ = column;
        }

        const Pixel& GetPixel(size_t row, size_t column) const {
            return pixel_array_elements_[pixel_array_width_ * row + column];
        }

        uint32_t GetHeight() const {
            return pixel_array_height_;
        }

        uint32_t GetWidth() const {
            return pixel_array_width_;
        }

        // Возвращает истину, если картина пустая, иначе - ложь
        bool IsEmpty() const {
            return pixel_array_elements_.empty();
        }

        // Данный метод вызывается только в случае абсолюнто пустой матрицы, с resize париться нет смысла.
        void Fill(size_t height, size_t width,
                  uint8_t default_red = 0,
                  uint8_t default_green = 0,
                  uint8_t default_blue = 0) {
            pixel_array_width_ = width;
            pixel_array_height_ = height;
            for (size_t i = 0; i < height * width; ++i) {
                Pixel pixel{default_blue, default_green, default_red};
                pixel_array_elements_.push_back(pixel);
            }
        }

    protected:
        size_t pixel_array_height_;
        size_t pixel_array_width_;
        std::vector<Pixel> pixel_array_elements_;
    };

    void CopyPixels(const PixelArray& other) {
        pixels_ = other;
    }

protected:
    PixelArray pixels_;
    BMPHeader bmp_header_;
    DIBHeader dib_header_;

public:
    // Загружает файл из переданного потока чтения (функция под этой как раз возвращает поток)
    bool Load(std::istream& stream);

    // Загружает файл с картинкой по имени в поток
    bool Load(const char* file_name);

    // Создаёт по имени файла bmp файл и загружает туда что-то.
    bool WriteFile(const char* file_name);

    // Мы можем не иметь права записывать что-то на диск или не иметь какой-то папки
    // поэтому делим create file в виде двух функций, по аналогии с load.
    bool WriteFile(std::ostream& stream);
};


// Альтернативный вариант класса PixelArray:
class NewPixelArray {
public:
    NewPixelArray() : storage_(nullptr), height_(0), width_(0) {}

    NewPixelArray(size_t height, size_t width, Bitmap::Pixel default_pixel = Bitmap::Pixel())
            : NewPixelArray() {
        // Избегаеем дублирование кода делегированием конструктора.
        // : storage_(nullptr), pixel_array_height_(height), pixel_array_width_(width) { // Вот так было раньше

        Resize(height, width, default_pixel);
    }

    // Реализация КК нам точно не подходит, пока не додумаемся, как сделать, надо его вообще запретить (как и сделали)
    // Например, в потоках тоже запрещены копирования. То же самое с операцией присваивания
    NewPixelArray(const NewPixelArray& other);

    NewPixelArray(NewPixelArray&& other);

    ~NewPixelArray() {
        FreeStorage();
    }

    NewPixelArray& operator=(const NewPixelArray& rhv);

    // Рассмотреть альтернативный вариант ресайза (можно сделать отдельным методом),
    // котороый при рисайзе точку припинивания (откуда идёт ресайз) позволяет в качестве
    // параметров передавать
    void Resize(size_t height, size_t width, Bitmap::Pixel default_pixel = Bitmap::Pixel());

    size_t GetHeight() const {
        return height_;
    }

    size_t GetWidth() const {
        return width_;
    }

    Bitmap::Pixel& operator()(size_t row, size_t column) {
        return GetPixel(storage_, width_, row, column);
    }

    const Bitmap::Pixel& operator()(size_t row, size_t column) const {
        return GetPixel(storage_, width_, row, column);
    }

    Bitmap::Pixel& At(size_t row, size_t column);

    const Bitmap::Pixel& At(size_t row, size_t column) const;

protected:
    // Просто правильно освобождает массив. Если хотим обнулить картинку, нужно сделать метод
    // clear (один метод вызывает другой + кое что, это нормально)
    // Делает согласованную картинку
    void FreeStorage() {
        delete[] storage_;
        // Важно обнулять указатель НЕМЕДЛЕННО, после того, как он стал невалидным
        // (Например, после удаления памяти)
        storage_ = nullptr;
        height_ = 0;
        width_ = 0;
    }

    // Делаем метод под каждый чих:
    // Маленький метод под распределение памяти необходимого размера под массив

    // Закрытый метод подразумевает, что будет вызван при height и width != 0
    Bitmap::Pixel* AllocateStorage(size_t height, size_t width) {
        return new Bitmap::Pixel[height * width];
    }

    // Копирует содержимое хранилища storage_ в хранилище передаваемое параметром Bitmap::Pixel* new_storage
    // принимая во внимание параметры точки пина и так далее (с этим мы сами играть должны)
    // Это вспомогательный метод, поэтому параметров по умолчанию быть не должно (метод не интерфейсный)
    // default_pixel - цвет пикселя по умолчанию, если вдруг картинка станет больше и пустые пиксели надо
    // чем-то заполнять.
    void CopyStorage(Bitmap::Pixel* new_storage,
                     size_t new_height,
                     size_t new_width,
                     Bitmap::Pixel default_pixel) const;

    // Почему noexcept? это модификатор, который говорит, что функция НЕ будет кидать исключения
    // если внутри функции кинем except, оно просто не даст это скомпилировать, программа рушится.
    // Зачем? copy and swap работает только в том случае, если гарантированно не кидается исключение.
    // Ещё в деструкторе, finalizer не кидают исключений.
    void Swap(NewPixelArray& other) noexcept {
        std::swap(storage_, other.storage_);
        std::swap(height_, other.height_);
        std::swap(width_, other.width_);
    }

    // защищенный метод без лишних проверок, зачем его вытаскивать наружу?
    // TODO константная перегрузка, если понадобится
    // TODO все статические в одном месте
    static Bitmap::Pixel& GetPixel(Bitmap::Pixel* storage, size_t width, size_t row, size_t column) {
        return storage[width * row + column];
    }


// TODO СНачала все паблик, потом все protected
protected:
    Bitmap::Pixel* storage_;
    size_t height_;
    size_t width_;
};
