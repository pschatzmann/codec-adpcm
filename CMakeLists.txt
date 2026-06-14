cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
    # Build as an ESP-IDF component (header-only library)
    idf_component_register(
        INCLUDE_DIRS "src"
    )
else()
    # set the project name
    project(codec-adpcm)

    # lots of warnings and all warnings as errors
    # add_compile_options(-Wall -Wextra )
    
    add_library(codec-adpcm INTERFACE)
    target_include_directories(adpcm INTERFACE src)

    # define location for header files
    add_subdirectory("tests/sine")
endif()

