#include "cmd_arg_parser.h"

#include <memory>

bool CmdLineParser::Parse(int argc, char* argv[]) {
    if (argc < MIN_PARAM_NUMBER_) {
        return false; // Недостаточно параметров
    }

    input_file_name_ = argv[INPUT_FILE_NAME_POS_];
    output_file_name_ = argv[OUTPUT_FILE_NAME_POS_];
    for (size_t i = MIN_PARAM_NUMBER_; i < argc; ++i) {
        if (argv[i][0] == '-') {
            FilterDescriptor* descriptor = new FilterDescriptor;
            descriptor->filter_name = argv[i];
            filter_descriptors_.push_back(descriptor);
        } else {
            if (filter_descriptors_.empty()) {
                return false; // Не написали тире перед именем
            }
            filter_descriptors_[filter_descriptors_.size() - 1]->filter_params.push_back(argv[i]);
        }
    }
    return true;
}

