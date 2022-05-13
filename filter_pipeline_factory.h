// Этот модуль является посредником между конкретными классами фильтров и некими описаниями
// параметров объектов фильтров (filter descriptor) поэтому он знает всё и про первых, и про вторых.

#pragma once

#include "cmd_arg_parser.h"
#include "base_filters.h"
#include "map"

// функций может быть много, поэтому используем неймспейс
namespace FilterFactories {
    // Тут мы фактически хардкодим список фильтров, доступных нашей программе.
    // Добавление нового фильтра осуществляется путём создания новой продуцирующей функции.
    BaseFilter* MakeCropFilter(const FilterDescriptor& fd);

    BaseFilter* MakeNegativeFilter(const FilterDescriptor& fd);

    BaseFilter* MakeGreyScaleFilter(const FilterDescriptor& fd);

    BaseFilter* MakeSharpeningFilter(const FilterDescriptor& fd);

    BaseFilter* MakeEdgeDetectionFilter(const FilterDescriptor& fd);

    BaseFilter* MakeGaussianBlurFilter(const FilterDescriptor& fd);

    BaseFilter* MakeCustomFilter(const FilterDescriptor& fd);
}

class FilterPipelineFactory {
public:
    FilterPipelineFactory() : filter_to_makers_({{"-crop", &FilterFactories::MakeCropFilter},
                                                 {"-neg", &FilterFactories::MakeNegativeFilter},
                                                 {"-gs", &FilterFactories::MakeGreyScaleFilter},
                                                 {"-sharp", &FilterFactories::MakeSharpeningFilter},
                                                 {"-egde", &FilterFactories::MakeEdgeDetectionFilter},
                                                 {"-blur", &FilterFactories::MakeGaussianBlurFilter},
                                                 {"-kek", &FilterFactories::MakeCustomFilter}}) {}
public:
    // typedef BaseFilter*(*FilterFactory)(const FilterDescriptor&); // Старый вариант, можно через using:
    using FilterFactory = BaseFilter* (*)(const FilterDescriptor&);  // Указатель на функцию
    using FilterToMakerMap = std::map<std::string_view, FilterFactory>;

public:
    void AddFilterMaker(std::string_view name, FilterFactory factory);

    // По заданному имени фильтра возвращает указатель на функцию (фактори)
    FilterFactory GetFilterFactoryMaker(std::string_view name) const;

    // Создаёт фильтр по переданному ему описанию параметров командной строки
    // пользуясь шестерёнками, добавленными ранее
    BaseFilter* CreateFilter(const FilterDescriptor& descriptor) const;

protected:
    FilterToMakerMap filter_to_makers_;
};





