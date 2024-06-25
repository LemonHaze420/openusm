cmake_minimum_required(VERSION 3.12)

include(cmake/i686-pc-mingw32.cmake)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-static -std=gnu11 -g")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
    "-fomit-frame-pointer -m32 -fno-asynchronous-unwind-tables -masm=intel -static -g")


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

add_compile_definitions(CINTERFACE)

if(USE_GLM)
    add_compile_definitions(USE_GLM)
    add_subdirectory(third_party/glm)
endif()

option(TARGET_XBOX "" OFF)
if (TARGET_XBOX)
    add_compile_definitions(TARGET_XBOX)
endif()

add_library(utility src/utility/trace.cpp)
target_include_directories(utility PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utility>)

add_library(variables src/variables/variables.cpp)
target_include_directories(variables PUBLIC "src/variables")
target_link_libraries(variables PUBLIC utility)


if (TARGET_XBOX)
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

add_library(binkw32 SHARED main.cpp)
target_compile_options(binkw32
  PRIVATE
    ${flags}
)
target_link_libraries(binkw32 PUBLIC usm)

add_library(memory "./src/memory.cpp" "./src/memory.h")
target_compile_options(memory
    PRIVATE
      ${flags}
  )
target_include_directories(memory PUBLIC "./include/")
target_link_libraries(memory PUBLIC utility)


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
target_link_libraries(nsl PUBLIC nfl utility)


file(GLOB_RECURSE NAL_SRC "./src/nal_system.cpp" "./src/nal/source/common/*.cpp")
add_library(nal ${NAL_SRC})
target_compile_options(nal
    PRIVATE
      ${flags}
)
target_include_directories(nal PUBLIC "./include/" "./src/" "./src/nal/include/common/")
target_link_libraries(nal PUBLIC nfl nalpanel utility)


target_link_libraries(nalcomp PUBLIC nal)

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
target_link_libraries(tx PUBLIC utility)


file(GLOB_RECURSE SRC_NFL "./src/nfl/src/*.cpp" "./src/nfl/src/*.h")
add_library(nfl ${SRC_NFL})
target_compile_options(nfl
    PRIVATE
      ${flags}
  )
target_include_directories(nfl INTERFACE "./src/nfl/src/")
target_link_libraries(nfl PUBLIC tx)

target_link_libraries(mash PUBLIC nfl)


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
target_link_libraries(physics PUBLIC utility ngl)

target_include_directories(usm PUBLIC "include")
target_include_directories(usm PUBLIC "./src/nfl/src/")

target_include_directories(chuck PUBLIC "include" "src")
target_include_directories(tx PUBLIC "include" "src")
target_include_directories(ngl_dx PUBLIC "include" "src")

target_link_libraries(ngl_dx PUBLIC d3dx9)

target_link_directories(usm PUBLIC ${CMAKE_FIND_ROOT_PATH}/include)
target_link_directories(usm PUBLIC ${CMAKE_FIND_ROOT_PATH}/lib)
target_link_libraries(usm PUBLIC physics)
target_link_libraries(usm PUBLIC charanimtype mash chuck nfl ngl nal nsl utility)
target_link_libraries(usm PUBLIC winmm psapi d3d9 dinput8 dxguid dsound)


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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Ultimate Spider-Man/")
    add_custom_command(TARGET binkw32 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/build/binkw32.dll" "${CMAKE_CURRENT_SOURCE_DIR}/../Ultimate Spider-Man/binkw32.dll"
        COMMAND_EXPAND_LISTS
        )
endif()
