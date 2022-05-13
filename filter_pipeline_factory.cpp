#include "filter_pipeline_factory.h"

#include <stdexcept>

namespace FilterFactories {
    BaseFilter* MakeCropFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-crop") {
            throw std::invalid_argument("wrong crop filter descriptor");
        }
        if (fd.filter_params.size() != 2) {
            throw std::invalid_argument("wrong crop filter params size");
        }
        // TODO кажется дальше идут какие-то костыли, нужно привести string_view к size_t
        std::string height_str = {fd.filter_params[0].begin(), fd.filter_params[0].end()};
        std::string width_str = {fd.filter_params[1].begin(), fd.filter_params[1].end()};
        return new CropFilter(std::stoi(height_str), std::stoi(width_str));
    }

    BaseFilter* MakeNegativeFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-neg") {
            throw std::invalid_argument("wrong negative filter descriptor");
        }
        if (!fd.filter_params.empty()) {
            throw std::invalid_argument("wrong negative filter params size");
        }
        return new NegativeFilter;
    }

    BaseFilter* MakeGreyScaleFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-gs") {
            throw std::invalid_argument("wrong greyscale filter descriptor");
        }
        if (!fd.filter_params.empty()) {
            throw std::invalid_argument("wrong greyscale filter params size");
        }
        return new GreyScaleFilter;
    }

    BaseFilter* MakeSharpeningFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-sharp") {
            throw std::invalid_argument("wrong sharpening filter descriptor");
        }
        if (!fd.filter_params.empty()) {
            throw std::invalid_argument("wrong sharpening filter params size");
        }
        return new SharpeningFilter;
    }

    BaseFilter* MakeEdgeDetectionFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-edge") {
            throw std::invalid_argument("wrong edge detection filter descriptor");
        }
        if (fd.filter_params.size() != 1) {
            throw std::invalid_argument("wrong edge detection filter params size");
        }
        std::string threshold = {fd.filter_params[0].begin(), fd.filter_params[0].end()};

        return new EdgeDetectionFilter(std::stof(threshold));
    }

    BaseFilter* MakeGaussianBlurFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-blur") {
            throw std::invalid_argument("wrong gaussian blur filter descriptor");
        }
        if (fd.filter_params.size() != 1) {
            throw std::invalid_argument("wrong gaussian blur filter params size");
        }
        std::string sigma = {fd.filter_params[0].begin(), fd.filter_params[0].end()};
        int int_sigma = stoi(sigma);
        if (int_sigma <= 0) {
            throw std::invalid_argument("sigma should be positive!");
        }
        return new GaussianBlurFilter(int_sigma);
    }

    BaseFilter* MakeCustomFilter(const FilterDescriptor& fd) {
        if (fd.filter_name != "-kek") {
            throw std::invalid_argument("wrong custom filter descriptor");
        }
        if (!fd.filter_params.empty()) {
            throw std::invalid_argument("wrong custom filter params size");
        }
        return new CustomFilter;
    }
}

void FilterPipelineFactory::AddFilterMaker(std::string_view name, FilterFactory factory) {
    filter_to_makers_.insert({name, factory});
}

FilterPipelineFactory::FilterFactory FilterPipelineFactory::GetFilterFactoryMaker(std::string_view name) const {
    // auto it = filter_to_makers_.find(name); auto меняем на более понятную штуку:
    FilterToMakerMap::const_iterator it = filter_to_makers_.find(name);
    if (it != filter_to_makers_.end()) {
        return it->second;
    }
    return nullptr;
}

BaseFilter* FilterPipelineFactory::CreateFilter(const FilterDescriptor& descriptor) const {
    // this - это fpf из TEST3
    FilterFactory gear = /*this->*/GetFilterFactoryMaker(descriptor.filter_name);
    if (!gear) {
        return nullptr;
    }
    BaseFilter* current_filter = gear(descriptor);
    return current_filter;
}
