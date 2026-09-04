cmake_minimum_required(VERSION 3.12)

include(ExternalProject)
include(cmake/i686-pc-mingw32.cmake)

option(OPENUSM_DEBUG_SYMBOLS "Build debug" OFF)

if(OPENUSM_DEBUG_SYMBOLS)
    set(OPENUSM_DEBUG_FLAG "-g")
else()
    set(OPENUSM_DEBUG_FLAG "-g0")
endif()

set(CMAKE_C_FLAGS_DEBUG "${OPENUSM_DEBUG_FLAG}" CACHE STRING "Flags used by the C compiler during Debug builds." FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${OPENUSM_DEBUG_FLAG}" CACHE STRING "Flags used by the CXX compiler during Debug builds." FORCE)

string(APPEND CMAKE_C_FLAGS " -static -std=gnu11 ${OPENUSM_DEBUG_FLAG}")
string(APPEND CMAKE_CXX_FLAGS
    " -fomit-frame-pointer -m32 -fno-asynchronous-unwind-tables -masm=intel -static ${OPENUSM_DEBUG_FLAG}")


set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND flags
    "-Wall"
    "-Wextra"
    "-pedantic"
    "-Werror"
    "-Wcast-align"
    "-Wcast-qual"
    "-Wzero-as-null-pointer-constant"
    #"-Wconversion"
    "-Wctor-dtor-privacy"
    "-Wduplicated-branches"
    "-Wduplicated-cond"
    "-Wextra-semi"
    "-Wfloat-equal"
    #"-Weffc++"
    "-Wlogical-op"
    "-Wnon-virtual-dtor"
    #"-Wold-style-cast"
    "-Woverloaded-virtual"
    "-Wredundant-decls"
    #"-Wsign-conversion"
    "-Wsign-promo"
    "-Wshift-negative-value"
    "-Wno-invalid-offsetof"
    "-Wno-deprecated")

project(USM-dev)

if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_MODULE_PREFIX "")
endif()

option(USE_GLM "description" OFF)
option(USE_FSTREAM "" OFF)

option(CHECK_STRUCT_SIZES "" ON)

if(CHECK_STRUCT_SIZES)
    add_compile_definitions(CHECK_STRUCT_SIZES)
endif()

option(SETUP_STANDALONE_SYSTEM "" OFF)
if(SETUP_STANDALONE_SYSTEM)
    add_compile_definitions(SETUP_STANDALONE_SYSTEM)
endif()

add_compile_definitions(CINTERFACE)

if(USE_GLM)
    add_compile_definitions(USE_GLM)
    add_subdirectory(third_party/glm)
endif()

option(TARGET_XBOX "" OFF)
if (TARGET_XBOX)
    add_compile_definitions(TARGET_XBOX)
endif()

option(OPENUSM_XBPACK_MODE "Use xbpack at runtime" OFF)
if (OPENUSM_XBPACK_MODE)
    add_compile_definitions(OPENUSM_XBPACK_MODE)
endif()

option(OPENUSM_XBPACK_V10 "Use the v10 XBPACK layout" OFF)
if (OPENUSM_XBPACK_V10)
    if (NOT OPENUSM_XBPACK_MODE)
        message(FATAL_ERROR "OPENUSM_XBPACK_V10 requires OPENUSM_XBPACK_MODE")
    endif()
    add_compile_definitions(OPENUSM_XBPACK_V10)
endif()

if (TARGET_XBOX AND OPENUSM_XBPACK_MODE)
    message(FATAL_ERROR "TARGET_XBOX and OPENUSM_XBPACK_MODE are mutually exclusive")
endif()

add_library(utility src/utility/trace.cpp)
target_include_directories(utility PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utility>)

add_library(variables src/variables/variables.cpp)
target_include_directories(variables PUBLIC "src/variables")
target_link_libraries(variables PUBLIC utility)


if (TARGET_XBOX OR OPENUSM_XBPACK_MODE)
    add_library(mash src/mash/xbox/src/mash_info_struct.cpp)
    target_include_directories(mash PUBLIC "src/mash/xbox/include/")
else()
    add_library(mash src/mash/pc/src/mash_info_struct.cpp)
    target_include_directories(mash PUBLIC "src/mash/pc/include")
endif()


target_include_directories(mash PUBLIC "src" "include")
target_compile_options(mash
  PRIVATE
    ${flags}
)
target_link_libraries(mash PUBLIC utility variables)

file(GLOB SRC "./src/*.cpp" "./src/*.h")
add_library(usm STATIC ${SRC})
target_compile_options(usm
    PUBLIC
    ${flags}
)

if(NOT SETUP_STANDALONE_SYSTEM)
    add_library(binkw32 SHARED main.cpp)
    if(NOT OPENUSM_DEBUG_SYMBOLS)
        set_property(TARGET binkw32 APPEND_STRING PROPERTY LINK_FLAGS " -s")
    endif()
target_compile_options(binkw32
  PRIVATE
    ${flags}
)
    target_link_libraries(binkw32 PUBLIC dxerr8 usm)
endif()


file(GLOB_RECURSE BASE_SRC "./src/base/source/*.cpp")
add_library(base ${BASE_SRC})
target_compile_options(base
    PRIVATE
      ${flags}
  )
target_include_directories(base PUBLIC "./src/base/include/")
target_include_directories(base PUBLIC "src" "include")
target_link_libraries(base PUBLIC utility nal)


file(GLOB_RECURSE PEDANIMTYPE_SRC "./src/pedanimtype/*.cpp")
add_library(pedanimtype ${PEDANIMTYPE_SRC})
target_compile_options(pedanimtype
    PRIVATE
      ${flags}
  )
target_include_directories(pedanimtype PUBLIC "./src/pedanimtype/")
target_include_directories(pedanimtype PUBLIC "./src/")
target_link_libraries(pedanimtype PUBLIC utility)


file(GLOB_RECURSE NALCOMP_SRC "./src/nalcomp/*.cpp")
add_library(nalcomp ${NALCOMP_SRC})
target_compile_options(nalcomp
    PRIVATE
      ${flags}
  )
target_include_directories(nalcomp PUBLIC "./src/nalcomp/")
target_include_directories(nalcomp PUBLIC "./src/")


file(GLOB_RECURSE NALPANEL_SRC "./src/panels/nalpanel/*.cpp")
add_library(nalpanel ${NALPANEL_SRC})
target_compile_options(nalpanel
    PRIVATE
      ${flags}
  )
target_include_directories(nalpanel PUBLIC "./src/panels/nalpanel/")
target_link_libraries(nalpanel PUBLIC nalcomp)

file(GLOB_RECURSE NSL_SRC "./src/nsl/src/nsl/*.cpp")
add_library(nsl ${NSL_SRC})
target_compile_options(nsl
    PRIVATE
      ${flags}
)
target_include_directories(nsl PUBLIC "./include/" "./src/" "./src/nsl/src/nsl/" )
target_link_libraries(nsl PUBLIC nfl utility variables)


file(GLOB_RECURSE NAL_SRC "./src/nal_system.cpp"
    "./src/nal/source/common/*.cpp"
    "./src/nal/samples/spiderman/*.cpp"
)
add_library(nal ${NAL_SRC})
target_compile_options(nal
    PRIVATE
      ${flags}
)
target_include_directories(nal PUBLIC "./include/" "./src/"
    "src/nal/include/common/"
    "src/nal/samples/spiderman/"
)
target_link_libraries(nal PUBLIC nfl nalpanel pedanimtype utility base)


target_link_libraries(nsl PUBLIC nal)

target_link_libraries(nalcomp PUBLIC nal)
target_link_libraries(nalcomp PUBLIC usm)

add_library(test_nal EXCLUDE_FROM_ALL ${NAL_SRC})
target_compile_options(test_nal
    PRIVATE
      ${flags}
)

add_library(debugutil "./src/debugutil.cpp" "./src/debugutil.h")
target_compile_options(debugutil
    PRIVATE
      ${flags}
)
target_include_directories(debugutil PUBLIC "./include/")
target_link_libraries(debugutil PUBLIC utility)


file(GLOB_RECURSE SRC_CHUCK "./src/chuck/vm/*.cpp")
add_library(chuck ${SRC_CHUCK})
target_compile_options(chuck
    PRIVATE
      ${flags}
  )

target_include_directories(chuck INTERFACE "./src/chuck/vm/")
target_link_libraries(chuck PUBLIC variables debugutil)


file(GLOB_RECURSE SRC_TX "./src/tx/src/*.cpp" "./src/tx/src/*.h")
add_library(tx ${SRC_TX})
target_compile_options(tx
    PRIVATE
      ${flags}
  )
target_include_directories(tx INTERFACE "./src/tx/src/")
target_link_libraries(tx PUBLIC utility variables)

add_library(test_tx EXCLUDE_FROM_ALL ${SRC_TX})
target_compile_options(test_tx
    PRIVATE
      ${flags}
  )
target_include_directories(test_tx INTERFACE "./src/tx/src/")
target_link_libraries(test_tx PUBLIC utility variables)


file(GLOB_RECURSE SRC_NFL "./src/nfl/src/*.cpp" "./src/nfl/src/*.h")
add_library(nfl ${SRC_NFL})
target_compile_options(nfl
    PRIVATE
      ${flags}
  )
target_include_directories(nfl INTERFACE "./src/nfl/src/")
target_link_libraries(nfl PUBLIC tx variables)

target_link_libraries(mash PUBLIC nfl)

add_library(test_nfl EXCLUDE_FROM_ALL ${SRC_NFL})
target_compile_options(test_nfl
    PRIVATE
      ${flags}
  )
target_include_directories(test_nfl INTERFACE "./src/nfl/src/")
target_link_libraries(test_nfl PUBLIC test_tx)


file(GLOB_RECURSE SRC_NGL_DX "./src/ngl/src/dx/*.cpp")
add_library(ngl_dx ${SRC_NGL_DX})
target_compile_options(ngl_dx
    PRIVATE
      ${flags}
  )

target_include_directories(ngl_dx PUBLIC "./src/ngl/include/dx")
target_include_directories(ngl_dx PUBLIC "./src/ngl/include/")
target_link_libraries(ngl_dx PUBLIC psapi utility variables usm)

file(GLOB_RECURSE SRC_NGL_SHADERS "./src/ngl/shaders/*.cpp")
add_library(ngl_shaders ${SRC_NGL_SHADERS})
target_compile_options(ngl_shaders
    PRIVATE
      ${flags}
  )


target_include_directories(ngl_shaders PUBLIC "./src/ngl/shaders")
target_link_libraries(ngl_shaders PUBLIC ngl_dx usm)

file(GLOB SRC_NGL "./src/ngl/src/*.cpp")
add_library(ngl ${SRC_NGL})
target_compile_options(ngl
    PRIVATE
      ${flags}
  )
target_link_libraries(ngl PUBLIC ngl_dx ngl_shaders)
target_include_directories(ngl PUBLIC "./src/ngl/include/")


file(GLOB SRC_CHARANIMTYPE "./src/charanimtype/*.cpp")
add_library(charanimtype ${SRC_CHARANIMTYPE})
target_compile_options(charanimtype
    PRIVATE
      ${flags}
  )
target_link_libraries(charanimtype PUBLIC nal ngl_dx ngl_shaders)
target_include_directories(charanimtype PUBLIC "./src/charanimtype/")


file(GLOB_RECURSE PHYSICS_SRC "./src/physics/source/*.cpp")
add_library(physics ${PHYSICS_SRC})
target_compile_options(physics
    PRIVATE
      ${flags}
  )
target_include_directories(physics PUBLIC "./src/physics/include/")
target_link_libraries(physics PUBLIC base nal utility)

target_include_directories(usm PUBLIC "include")
target_include_directories(usm PUBLIC "./src/nfl/src/")
target_include_directories(usm PUBLIC "./src/nal/include/common/")

target_include_directories(chuck PUBLIC "include" "src")
target_include_directories(tx PUBLIC "include" "src")
target_include_directories(ngl_dx PUBLIC "include" "src")

# Assimp lib
if(NOT SETUP_STANDALONE_SYSTEM)
    ExternalProject_Add(
        AssimpProject
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/assimp
        BINARY_DIR ${CMAKE_BINARY_DIR}/assimp-build
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/i686-pc-mingw32.cmake
            -DCMAKE_BUILD_TYPE=Release
            -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
            -DASSIMP_BUILD_OBJ_IMPORTER=ON
            -DASSIMP_BUILD_FBX_IMPORTER=ON
            -DASSIMP_BUILD_COLLADA_IMPORTER=ON
            -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
            -DASSIMP_BUILD_TESTS=OFF
            -DASSIMP_NO_EXPORT=ON
            -DBUILD_SHARED_LIBS=OFF
            -DASSIMP_BUILD_ZLIB=ON
            -DASSIMP_BUILD_SAMPLES=OFF
            -DASSIMP_OPT_BUILD_PACKAGES=OFF
            -DCMAKE_CXX_FLAGS=-static-libgcc\ -static-libstdc++
        INSTALL_COMMAND ""
    )
    target_include_directories(usm PUBLIC "./assimp/include/")
    target_link_directories(usm PUBLIC ${CMAKE_BINARY_DIR}/assimp-build/lib)
    add_dependencies(usm AssimpProject)
endif()


target_include_directories(test_tx PUBLIC "include" "src")

target_link_libraries(ngl_dx PUBLIC d3dx9)

target_include_directories(usm PUBLIC ${CMAKE_FIND_ROOT_PATH}/include)
target_link_directories(usm PUBLIC ${CMAKE_FIND_ROOT_PATH}/lib)
target_link_libraries(usm PUBLIC physics)
target_link_libraries(usm PUBLIC charanimtype mash chuck nfl ngl nal nsl utility)
target_link_libraries(usm PUBLIC winmm psapi d3d9 dinput8 dxguid dsound)

if(NOT SETUP_STANDALONE_SYSTEM)
    target_link_libraries(usm PUBLIC assimp z)
endif()


if(USE_GLM)
    target_link_libraries(usm PUBLIC glm::glm)
endif()

option(ENABLE_TESTS "" ON)

if(ENABLE_TESTS)
    set(GOOGLETEST_VERSION 1.1)

    add_subdirectory(third_party/googletest)

    include(CTest)
    enable_testing()
    add_subdirectory(test)

endif()

find_program(DOCKER_EXECUTABLE docker)
if(DOCKER_EXECUTABLE)
    add_custom_target(check-format
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_format.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        USES_TERMINAL
        VERBATIM)
endif()

if(NOT SETUP_STANDALONE_SYSTEM)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Ultimate Spider-Man/")
    add_custom_command(TARGET binkw32 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/build/binkw32.dll" "${CMAKE_CURRENT_SOURCE_DIR}/../Ultimate Spider-Man/binkw32.dll"
        COMMAND_EXPAND_LISTS
        )
endif()
endif()
