#include "cmd_arg_parser.h"
#include "filter_pipeline_factory.h"
#include "bitmap.h"

#include <cassert>
#include <iostream>
#include <memory>

// const char* INPUT_FILENAME = "../pictures/dama.bmp";
const char* INPUT_FILENAME = "../pictures/dama.bmp";

void Test1(int argc, char* argv[]) {
    std::cout << "number of args: " << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "arg[" << i << "] = " << argv[i] << std::endl;
    }
}

void Test2() {
    FilterDescriptor desc1{"-blur", {"12", "23", "156"}};
}

// Создаём промежуточный объект и заполняем его создателями индивидуальных фильтров.
// Данный тест является прообразом статической конфигурации приложения, то есть
// в нём задаётся конкретный набор типов фильтров, которые приложение сможет использовать.
void Test3() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-crop", &FilterFactories::MakeCropFilter); // & не обязательно, ибо неявное приведение
    FilterPipelineFactory::FilterFactory gear = fpf.GetFilterFactoryMaker("crop");
    assert(gear /*!= nullptr*/);
    FilterPipelineFactory::FilterFactory non_existing_gear = fpf.GetFilterFactoryMaker("alpha");
    assert(!non_existing_gear);
}

// Имеет один описатель фильтра, проверяем, что с его помощью можем создать объект фильтра
// по средствам промежуточного объекта.
void Test4() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-crop", &FilterFactories::MakeCropFilter);

    FilterDescriptor desc1{"-crop", {"10", "20"}};

    BaseFilter* filter = fpf.CreateFilter(desc1);
    assert(filter);
    delete filter;
}

// Создаст фильтр из дескриптора руками.
void Test5() {
    FilterDescriptor desc1{"-crop", {"12", "23"}};
    BaseFilter* filter = FilterFactories::MakeCropFilter(desc1);
    assert(filter);
    delete filter;

    FilterDescriptor desc2{"-blurrrrr", {"12", "23", "156"}};
    try {
        BaseFilter* filter2 = FilterFactories::MakeCropFilter(desc2);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
    }
}

// Проверяем, что структуры правильно упаковываются (возникает ли проблема из-за padding)
void Test6() {
    Bitmap::TestHeader header = {0x55, 0xAAAA, 0x55555555}; // 0x - 16 ричное представление
    Bitmap::TestHeader2 header2 = {0x55, 0xAAAA, 0x55555555};
    // И тут padding пропал
}

// Тестим считывание файла
void Test7() {
    Bitmap bitmap{};
    if (!bitmap.Load(INPUT_FILENAME)) {
        std::cout << "Cannot find file with this name or some other mistake";
    }
}

// Тестим создание файла
void Test8() {
    Bitmap bitmap{};
    if (!bitmap.Load(INPUT_FILENAME)) {
        std::cout << "Cannot find file with this name or some other mistake";
    }

    const char* output_filename = "../pictures/TEST8_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

void Test9() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-neg", &FilterFactories::MakeNegativeFilter);
    fpf.AddFilterMaker("-crop", &FilterFactories::MakeCropFilter);

    FilterDescriptor desc1{"-neg", {}};

    // BaseFilter* filter = fpf.CreateFilter(desc1) используем умные указатели
    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST9_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
    // Теперь память управляется умным указателем (потому что RAII).
    // delete filter;
}

// Пробуем применить фильтр GreyScale
void Test10() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-gs", &FilterFactories::MakeGreyScaleFilter);

    FilterDescriptor desc1{"-gs", {}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST10_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

// Пробуем применить фильтр Crop
void Test11() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-gs", &FilterFactories::MakeGreyScaleFilter);
    fpf.AddFilterMaker("-crop", &FilterFactories::MakeCropFilter);

    FilterDescriptor desc1{"-crop", {"1000", "800"}};
    FilterDescriptor desc2{"-gs", {}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    std::shared_ptr<BaseFilter> filter2(fpf.CreateFilter(desc2));
    assert(filter);
    assert(filter2);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);
    filter2->Apply(bitmap);

    const char* output_filename = "../pictures/TEST11_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

void Test12_helper_0(NewPixelArray new_pixel) {
    new_pixel.GetWidth();
}

NewPixelArray Test12_helper() {
    return NewPixelArray(10, 3);
}

// Проверем написанный на семинаре от 16.03.22 класс NewArray, который не использует vector и вообще крутой.
void Test12() {
    NewPixelArray a1;
    // NewPixelArray a2 = a1; // Нельзя, пока не реализовани конструктор копирования
    // NewPixelArray a3(a1); // Это тоже конструктор копирования, тоже нельзя
    NewPixelArray a4(10, 5);
    NewPixelArray a5 = a4;
    assert(a5.GetHeight() == a4.GetHeight());
    assert(a5.GetWidth() == a4.GetWidth());
    assert(a5(2, 3) == a4(2, 3));
    a1 = a4;
//    NewPixelArray a6 = NewPixelArray(6, 7);
    NewPixelArray a6 = std::move(Test12_helper());
    Test12_helper_0(NewPixelArray(4, 5)); //!!!
    int a = 0;
}
// TODO в качестве эксперемента можно переключиться на 14 стандарт и посмотреть, вызовется ли на !!! move конструктор.

// Пробуем применить фильтр Sharpening
void Test13() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-sharp", &FilterFactories::MakeSharpeningFilter);

    FilterDescriptor desc1{"-sharp", {}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST13_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

// Пробуем применить фильтр Edge Detection
void Test14() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-edge", &FilterFactories::MakeEdgeDetectionFilter);

    FilterDescriptor desc1{"-edge", {"10"}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST14_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

// Пробуем в фильтре crop указать слишком большие значения (ничего не должно произойти)
void Test15() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("-crop", &FilterFactories::MakeCropFilter);

    FilterDescriptor desc1{"-crop", {"1000000", "8000000"}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST15_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

// Тестируем фильтр Гауссового размытия
void Test16() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("blur sigma", &FilterFactories::MakeGaussianBlurFilter);

    FilterDescriptor desc1{"blur sigma", {"2"}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load(INPUT_FILENAME));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/TEST16_output_dama.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

// Тестируем кастомный фильтр
void Test17() {
    FilterPipelineFactory fpf;
    fpf.AddFilterMaker("custom", &FilterFactories::MakeCustomFilter);

    FilterDescriptor desc1{"custom", {}};

    std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(desc1));
    assert(filter);

    Bitmap bitmap;
    assert(bitmap.Load("../pictures/shrek.bmp"));
    assert(!bitmap.IsEmpty());

    filter->Apply(bitmap);

    const char* output_filename = "../pictures/kek.bmp";
    if (!bitmap.WriteFile(output_filename)) {
        std::cout << "Cannot write into file with this name or some other mistake";
    }
}

int main(int argc, char* argv[]) {
//    Test1(argc, argv);
//    Test2();
//    Test3();
//    Test4();
//    Test5();
//    Test6();
//    Test7();
//    Test8(); // nothing
//    Test9(); // negative
//    Test10(); // greyscale
//    Test11(); // crop
//    Test12();
//    Test13(); // sharpening
//    Test14(); // Edge Detection
//    Test15(); // crop, где плохие значения
//    Test16(); // Gauss
//    Test17(); // Custom filter
    CmdLineParser parser;
    if (!parser.Parse(argc, argv)) {
        std::cout << "Something went wrong, try again with other values";
    }
    std::cout << "successfully parsed";

    Bitmap bitmap;

    std::string input_filename = {parser.GetInputFileName().begin(), parser.GetInputFileName().end()};
    assert(bitmap.Load(input_filename.c_str()));
    assert(!bitmap.IsEmpty());

    FilterPipelineFactory fpf;
    for (auto descriptor: parser.GetFilterDescriptors()) {
        std::shared_ptr<BaseFilter> filter(fpf.CreateFilter(*descriptor));
        assert(filter);
        filter->Apply(bitmap);
        delete descriptor;
    }

    std::string output_filename = {parser.GetOutputFileName().begin(), parser.GetOutputFileName().end()};
    assert(bitmap.WriteFile(output_filename.c_str()));
}
