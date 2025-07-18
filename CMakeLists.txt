# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    CMakeLists.txt                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abelov <abelov@student.42london.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/17 22:59:30 by abelov            #+#    #+#              #
#    Updated: 2025/07/17 22:59:31 by abelov           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_COMPILER clang++)
#set(CMAKE_CXX_COMPILER g++)

project(webserv CXX)

set(CMAKE_CXX_STANDARD 98)

include_directories(include)

set(COMMON_LDFLAGS
		-fsanitize=address
		-fsanitize=undefined
		-fno-sanitize-recover=all
		-fsanitize=float-divide-by-zero
		-fsanitize=float-cast-overflow)
set(COMMON_CFLAGS
		-Wwrite-strings -Wnarrowing
		-O0
		-masm=att
		-fno-exceptions
		-ffunction-sections
		-fdata-sections
		-fno-omit-frame-pointer
		-fdwarf2-cfi-asm
		-fcf-protection=none
		-g3 -gdwarf-3
		-fasynchronous-unwind-tables
		-fsanitize=address
		-fsanitize=undefined
		-fno-sanitize-recover=all
		-fsanitize=float-divide-by-zero
		-fsanitize=float-cast-overflow
)

#==============================================================================#

find_package(PkgConfig REQUIRED)

file(GLOB_RECURSE TARGET_HEADERS "src/*.hpp")
file(GLOB_RECURSE TARGET_SOURCES "src/*.cpp")

set(TARGET_NAME "${PROJECT_NAME}_exe")

set(TARGET_LIBRARIES

)

add_executable(${TARGET_NAME}
		${TARGET_HEADERS}
		${TARGET_SOURCES})
target_link_libraries(${TARGET_NAME} "${TARGET_LIBRARIES}")
target_include_directories(${TARGET_NAME} PRIVATE
		${PROJECT_SOURCE_DIR}
		/usr/include
		/usr/include/x86_64-linux-gnu)

#==============================================================================#
# Custom targets
#==============================================================================#
add_custom_target(fclean
		COMMAND make -C ${PROJECT_SOURCE_DIR} fclean)

add_custom_target(re
		COMMAND make -C ${PROJECT_SOURCE_DIR} re)

add_custom_target(norm
		COMMAND make -C ${PROJECT_SOURCE_DIR} norm)
