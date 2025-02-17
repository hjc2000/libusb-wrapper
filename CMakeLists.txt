cmake_minimum_required(VERSION 3.8)

if(1)
	get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	set(ProjectName "${CURRENT_DIR_NAME}")

	include($ENV{cpp_lib_build_scripts_path}/cmake-module/setup.cmake)

	project(${ProjectName})
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_C_STANDARD 17)
endif()

include($ENV{cpp_lib_build_scripts_path}/cmake-module/setup.cmake)
include(target_import_libusb)
include(target_import_base)

# 定义生成规则
add_library("${ProjectName}")
target_import_src(${ProjectName})
target_import_libusb(${ProjectName} PUBLIC)
target_import_base(${ProjectName} PUBLIC)
