if(NOT TARGET usm)
    message(FATAL_ERROR "Configure SXCC through the OpenUSM root with OPENUSM_BUILD_SXCC=ON")
endif()
if(TARGET_XBOX OR OPENUSM_XBPACK_MODE)
    message(FATAL_ERROR "SXCC requires the retail PC engine layout")
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
target_link_libraries(sxcc PRIVATE usm)
target_compile_features(sxcc PRIVATE cxx_std_20)
target_link_options(sxcc PRIVATE -static-libgcc -static-libstdc++)
set_target_properties(sxcc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
