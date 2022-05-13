cmake_minimum_required(VERSION 3.20)
project(project)

set(CMAKE_CXX_STANDARD 17)

add_executable(project
        main.cpp
        cmd_arg_parser.cpp
        cmd_arg_parser.h
        filter_pipeline_factory.cpp
        filter_pipeline_factory.h
        base_filters.h
        base_filters.cpp
        bitmap.h
        bitmap.cpp
        )
