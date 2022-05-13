#pragma once

#include "bitmap.h"

class BaseFilter {
public:
    virtual void Apply(Bitmap& bitmap) = 0;
};

class CropFilter : public BaseFilter {
public:
    CropFilter(size_t height, size_t width) : new_height_(height), new_width_(width) {}

    void Apply(Bitmap& bitmap) override;

protected:
    size_t new_height_;
    size_t new_width_;
};

class NegativeFilter : public BaseFilter {
public:
    void Apply(Bitmap& bitmap) override;
};

class GreyScaleFilter : public BaseFilter {
public:
    void Apply(Bitmap& bitmap) override;
};

class SharpeningFilter : public BaseFilter {
public:
    void Apply(Bitmap& bitmap) override;

protected:
    std::vector<std::vector<int>> sharpening_matrix_ = {{0,  -1, 0},
                                                        {-1, 5,  -1},
                                                        {0,  -1, 0}};
};

class EdgeDetectionFilter : public GreyScaleFilter {
public:
    EdgeDetectionFilter(float threshold) : threshold_(threshold) {}

    void Apply(Bitmap& bitmap) override;

protected:
    float threshold_;
    std::vector<std::vector<int>> edge_detection_matrix_ = {{0,  -1, 0},
                                                            {-1, 4,  -1},
                                                            {0,  -1, 0}};
};

class GaussianBlurFilter : public BaseFilter {
public:
    GaussianBlurFilter(int32_t sigma) : sigma_(sigma) {}

    void Apply(Bitmap& bitmap) override;

protected:
    int32_t sigma_;
    std::vector<std::vector<float>> gaussian_blur_matrix_;
};

class CustomFilter : public BaseFilter {
public:

    void Apply(Bitmap& bitmap) override;
};