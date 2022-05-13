#include "base_filters.h"
#include "bitmap.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace ColourHelpers {
    void ColorizePixel(Bitmap::Pixel& pixel, float red = 0, float green = 0, float blue = 0) {
        pixel.red = std::min(255, std::max(0, static_cast<int32_t>(red)));
        pixel.green = std::min(255, std::max(0, static_cast<int32_t>(green)));
        pixel.blue = std::min(255, std::max(0, static_cast<int32_t>(blue)));
    }

    // Для Гауссова размытия нужно уметь работать с матрицой nxn
    template<typename T>
    void ApplyMatrix(std::vector<std::vector<T>>& matrix, Bitmap& bitmap) {

        struct CurrentColour {
            T red = 0;
            T green = 0;
            T blue = 0;
        };

        Bitmap::PixelArray new_pixels;
        new_pixels.SetHeight(bitmap.GetHeight());
        new_pixels.SetWidth(bitmap.GetWidth());

        int32_t edge_value_x = matrix.size() / 2;
        int32_t edge_value_y = matrix[0].size() / 2;
        for (int32_t row = 0; row < bitmap.GetHeight(); ++row) {
            for (int32_t column = 0; column < bitmap.GetWidth(); ++column) {
                Bitmap::Pixel pixel{};
                CurrentColour cur_colour;

                for (int32_t dx = -edge_value_x; dx <= edge_value_x; ++dx) {
                    for (int32_t dy = -edge_value_y; dy <= edge_value_y; ++dy) {
                        int32_t x = row + dx;
                        int32_t y = column + dy;
                        x = std::min(std::max(x, 0), static_cast<int32_t>(bitmap.GetHeight()) - 1);
                        y = std::min(std::max(y, 0), static_cast<int32_t>(bitmap.GetWidth()) - 1);

                        cur_colour.red += bitmap.GetPixel(x, y).red * matrix[dx + edge_value_x][dy + edge_value_y];
                        cur_colour.green += bitmap.GetPixel(x, y).green * matrix[dx + edge_value_x][dy + edge_value_y];
                        cur_colour.blue += bitmap.GetPixel(x, y).blue * matrix[dx + edge_value_x][dy + edge_value_y];
                    }
                }
                ColorizePixel(pixel, cur_colour.red, cur_colour.green, cur_colour.blue);
                new_pixels.PushBack(pixel);
            }
        }
        bitmap.CopyPixels(new_pixels);
    }
}


// Мною было убито два часа на пробование реализовать это без копирования, но нервы важнее.
void CropFilter::Apply(Bitmap& bitmap) {
    if (new_height_ > bitmap.GetHeight()) {
        new_height_ = bitmap.GetHeight();
    } else {
        bitmap.SetFileHeight(new_height_);
    }
    if (new_width_ > bitmap.GetWidth()) {
        new_width_ = bitmap.GetWidth();
    } else {
        bitmap.SetFileWidth(new_width_);
    }
    Bitmap::PixelArray new_pixels;
    new_pixels.SetHeight(new_height_);
    new_pixels.SetWidth(new_width_);
    for (size_t row = bitmap.GetHeight() - new_height_; row < bitmap.GetHeight(); ++row) {
        for (size_t column = 0; column < new_width_; ++column) {
            new_pixels.PushBack(bitmap.GetPixel(row, column));
        }
    }
    bitmap.CopyPixels(new_pixels);
}

void NegativeFilter::Apply(Bitmap& bitmap) {
    for (size_t row = 0; row < bitmap.GetHeight(); ++row) {
        for (size_t column = 0; column < bitmap.GetWidth(); ++column) {
            ColourHelpers::ColorizePixel(bitmap.GetPixel(row, column),
                                         255.0f - bitmap.GetPixel(row, column).red,
                                         255.0f - bitmap.GetPixel(row, column).green,
                                         255.0f - bitmap.GetPixel(row, column).blue);
        }
    }
}

void GreyScaleFilter::Apply(Bitmap& bitmap) {
    for (size_t row = 0; row < bitmap.GetHeight(); ++row) {
        for (size_t column = 0; column < bitmap.GetWidth(); ++column) {
            uint8_t new_colour = bitmap.GetPixel(row, column).red * 0.299 +
                                 bitmap.GetPixel(row, column).blue * 0.114 +
                                 bitmap.GetPixel(row, column).green * 0.587;
            ColourHelpers::ColorizePixel(bitmap.GetPixel(row, column), new_colour, new_colour, new_colour);
        }
    }
}


void SharpeningFilter::Apply(Bitmap& bitmap) {
    ColourHelpers::ApplyMatrix(sharpening_matrix_, bitmap);
}

// После применения грейскейла все поля red, green, blue стали одинаковыми
void EdgeDetectionFilter::Apply(Bitmap& bitmap) {
    GreyScaleFilter::Apply(bitmap);
    ColourHelpers::ApplyMatrix(edge_detection_matrix_, bitmap);
    for (size_t row = 0; row < bitmap.GetHeight(); ++row) {
        for (size_t column = 0; column < bitmap.GetWidth(); ++column) {
            if (bitmap.GetPixel(row, column).red > threshold_) {
                ColourHelpers::ColorizePixel(bitmap.GetPixel(row, column), 255, 255, 255);
            } else {
                ColourHelpers::ColorizePixel(bitmap.GetPixel(row, column)); // Красит в чёрный по дефолту
            }
        }
    }
}

float GaussianFunction(int32_t x, int32_t y, int32_t sigma) {
    return pow(M_E, (-pow(x, 2) - pow(y, 2)) / (2 * pow(sigma, 2)))
           / (2 * M_PI * pow(sigma, 2));
}

// Основная суть: в Гауссовском размытии клетки, стоящие дальше, чем на 3*sigma, совсем уж незначительны,
// поэтому будем составлять матрицу размера [6*sigma + 1][6*sigma + 1] и применять её,
// что при не очень больших сигма явно лучше, чем 2048^8 операций. Время работы при sigma = 4: 2 минуты, зато трушно.
void GaussianBlurFilter::Apply(Bitmap& bitmap) {
    float matrix_sum = 0;
    for (int32_t x = -3 * sigma_; x <= 3 * sigma_; ++x) {
        std::vector<float> temporary_matrix;
        for (int32_t y = -3 * sigma_; y <= 3 * sigma_; ++y) {
            float gaussian_function = GaussianFunction(x, y, sigma_);
            matrix_sum += gaussian_function;
            temporary_matrix.push_back(gaussian_function);
        }
        gaussian_blur_matrix_.push_back(temporary_matrix);
    }
    // Чтобы картинка не поменяла яркость, нужно, чтобы сумма элементов в матрице была равна 1. Для этого matrix_sum.
    for (int32_t x = -3 * sigma_; x <= 3 * sigma_; ++x) {
        for (int32_t y = -3 * sigma_; y <= 3 * sigma_; ++y) {
            gaussian_blur_matrix_[x + 3 * sigma_][y + 3 * sigma_] /= matrix_sum;
        }
    }
    ColourHelpers::ApplyMatrix(gaussian_blur_matrix_, bitmap);
}

void CustomFilter::Apply(Bitmap& bitmap) {
    size_t half_width = bitmap.GetWidth() / 2;
    for (size_t row = 0; row < bitmap.GetHeight(); ++row) {
        for (size_t column = half_width + 1; column < bitmap.GetWidth(); ++column) {
            size_t difference = column - half_width;
            bitmap.GetPixel(row, column) = bitmap.GetPixel(row, half_width - difference);
        }
    }
}
