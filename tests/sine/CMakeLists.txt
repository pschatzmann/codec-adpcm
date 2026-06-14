
# build executable
add_executable (test test.cpp)

target_compile_options    (test PUBLIC "-g3"  )

# add library
target_link_libraries(test PUBLIC codec-adpcm)
