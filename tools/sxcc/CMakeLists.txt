cmake_minimum_required(VERSION 3.20)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(sxcc LANGUAGES CXX)
endif()

add_executable(sxcc
    main.cpp
    image.cpp
    instruction.cpp
    source.cpp
    decompile.cpp
    language.cpp
    lower.cpp
    semantic.cpp
    lifting.cpp
    type_recovery.cpp
    analysis.cpp)
set_target_properties(sxcc PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(sxcc PRIVATE cxx_std_20)
if(MINGW)
    target_link_options(sxcc PRIVATE -static-libgcc -static-libstdc++)
endif()
set_target_properties(sxcc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
