# The name of our project is "VULKAN". CMakeLists files in this project can
# refer to the root source directory of the project as ${VULKANTOOLS_SOURCE_DIR} and
# to the root binary directory of the project as ${VULKANTOOLS_BINARY_DIR}.
cmake_minimum_required(VERSION 2.8.11)
project (VULKANTOOLS)
# set (CMAKE_VERBOSE_MAKEFILE 1)



# The MAJOR number of the version we're building, used in naming
# vulkan-<major>.dll (and other files).
set(MAJOR "1")


if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
    set(DisplayServer Win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
    set(DisplayServer Android)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    set(DisplayServer Xcb)

#   TODO: Basic support is present for Xlib but is untested.
#         Wayland/Mir support is stubbed in but unimplemented and untested.

#   add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
#   set(DisplayServer Xlib)

#   add_definitions(-DVK_USE_PLATFORM_MIR_KHR)
#   set(DisplayServer Mir)

#   add_definitions(-DVK_USEPLATFORM_WAYLAND_KHR)
#   set(DisplayServer Wayland)

else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()





set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Header file for CMake settings
#hacky use layer include directory from loader repo
set (LOADER_INCLUDE_DIR ${VULKANTOOLS_SOURCE_DIR}/../LoaderAndValidationLayers/include)
include_directories(${PROJECT_SOURCE_DIR}/include ${LOADER_UNCLUDE_DIR})

if(NOT WIN32)
    include(FindPkgConfig)
endif()

set (CMAKE_INSTALL_PREFIX "")

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(COMMON_COMPILE_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
    set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -fno-strict-aliasing -fno-builtin-memcmp")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ${COMMON_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_FLAGS} -std=c++11")
    if (UNIX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

if(WIN32)
    option(BUILD_ICD "Build LunarG intel icd" OFF)
else()
    option(BUILD_ICD "Build LunarG intel icd" ON)
endif()
option(BUILD_TESTS "Build tests" ON)
option(BUILD_LAYERS "Build layers" ON)
option(BUILD_VKTRACE "Build VkTrace" ON)

if (BUILD_ICD OR BUILD_TESTS)
    # Hard code our glslang path for now
    get_filename_component(GLSLANG_PREFIX ../glslang ABSOLUTE)

    if(NOT EXISTS ${GLSLANG_PREFIX})
        message(FATAL_ERROR "Necessary glslang components do not exist: " ${GLSLANG_PREFIX})
    endif()
endif()

if (BUILD_ICD)
    # Hard code our LunarGLASS path for now
    get_filename_component(LUNARGLASS_PREFIX ../LunarGLASS ABSOLUTE)

    if(NOT EXISTS ${LUNARGLASS_PREFIX})
        message(FATAL_ERROR "Necessary LunarGLASS components do not exist: " ${LUNARGLASS_PREFIX})
    endif()
endif()

if(NOT WIN32)
    include(GNUInstallDirs)
    add_definitions(-DSYSCONFDIR="${CMAKE_INSTALL_SYSCONFDIR}")
    add_definitions(-DDATADIR="${CMAKE_INSTALL_DATADIR}")
    if (CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    elseif (CMAKE_INSTALL_PREFIX STREQUAL "")
    else()
        add_definitions(-DLOCALPREFIX="${CMAKE_INSTALL_PREFIX}")
    endif()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(PYTHON_CMD "python3")
    endif()
else()
    set(PYTHON_CMD "py")
endif()

set(LOADER_PY_DIR ${VULKANTOOLS_SOURCE_DIR}/../LoaderAndValidationLayers)

# icd: Device dependent (DD) VULKAN components
# tests: VULKAN tests

if(BUILD_ICD)
    add_subdirectory(icd)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_LAYERS)
    add_subdirectory(layers)
endif()


if(BUILD_VKTRACE)
    add_subdirectory(vktrace)
endif()

