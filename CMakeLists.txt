cmake_minimum_required(VERSION 3.4.3)
project(gemoc_kernel)

set(EXECUTABLE_NAME gemoc_kernel)

# Configuration
# =============

include(GNUInstallDirs)

# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/gemoc_kernel/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/gemoc_kernel/kernel.json"
)

option(XEUS_STATIC_DEPENDENCIES "link statically with xeus dependencies" OFF)
if (XEUS_STATIC_DEPENDENCIES)
    set(xeus_target "xeus-static")
else ()
    set(xeus_target "xeus")
endif ()

# Dependencies
# ============

# Be sure to use recent versions
set(xeus_REQUIRED_VERSION 1.0.4)
set(cppzmq_REQUIRED_VERSION 4.3.0)

find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
find_package(Threads)

# Flags
# =====

include(CheckCXXCompilerFlag)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Unsupported compiler -- xeus requires C++14 support!")
    endif()
endif()

# Target and link
# ===============

# my_kernel source files
set(MY_KERNEL_SRC
    src/custom_interpreter.cpp
    src/custom_interpreter.hpp
)

# My kernel executable
add_executable(${EXECUTABLE_NAME} src/main.cpp ${MY_KERNEL_SRC} )
find_package(CURL REQUIRED)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE xeus Threads::Threads ${CURL_LIBRARIES})

if (APPLE)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Installation
# ============

# Install my_kernel
install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and my_kernel
set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

# Install Jupyter kernelspecs
set(MY_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
install(DIRECTORY ${MY_KERNELSPEC_DIR}
        DESTINATION ${XJUPYTER_DATA_DIR}
        PATTERN "*.in" EXCLUDE)
