cmake_minimum_required(VERSION 2.8)

# Compiler requirements
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(tinyply LANGUAGES CXX)
set(PROJECT_VERSION 2.2)

include_directories("${CMAKE_SOURCE_DIR}/source")

set(CMAKE_DEBUG_POSTFIX "d")

# Library
set( SHARED_LIB false CACHE BOOL "Build as shared library")
if(${SHARED_LIB})
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_library(tinyply SHARED source/tinyply.cpp source/tinyply.h)
else()
    add_library(tinyply STATIC source/tinyply.cpp source/tinyply.h)
endif()

# Example
add_executable(example source/example.cpp)
target_link_libraries(example PRIVATE tinyply)

# pkg-config support
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
if(UNIX)
  configure_file(cmake/tinyply.pc.in ${generated_dir}/tinyply.pc @ONLY)
  install(FILES "${generated_dir}/tinyply.pc" DESTINATION lib/pkgconfig)
endif()

# CMake package config support
include(CMakePackageConfigHelpers)

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "")

write_basic_package_version_file(
  "${version_config}"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
  "cmake/Config.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}"
)

# Install
install(TARGETS tinyply EXPORT ${targets_export_name}
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES source/tinyply.h
        DESTINATION include)

install(FILES "${project_config}" "${version_config}"
        DESTINATION "${config_install_dir}")

install(EXPORT "${targets_export_name}"
        NAMESPACE "${namespace}"
        DESTINATION "${config_install_dir}")
