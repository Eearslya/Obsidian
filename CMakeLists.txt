cmake_minimum_required(VERSION 3.18)
project(Obsidian
	VERSION 0.0.1
	DESCRIPTION "Obsidian Game Engine"
	LANGUAGES C)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
endif()

add_subdirectory(Engine)
add_subdirectory(Sandbox)
