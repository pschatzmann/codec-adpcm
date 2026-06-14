cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
    # Build as an ESP-IDF component (header-only library)
    idf_component_register(
        INCLUDE_DIRS "src"
    )
else()
    # set the project name
    project(adpcm_ffmpeg)

    # lots of warnings and all warnings as errors
    # add_compile_options(-Wall -Wextra )

    # define location for header files
    add_subdirectory("src")
    add_subdirectory("tests/sine")
endif()

