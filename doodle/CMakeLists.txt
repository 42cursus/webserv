# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    CMakeLists.txt                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abelov <abelov@student.42london.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/07 18:42:16 by abelov            #+#    #+#              #
#    Updated: 2026/01/07 18:42:18 by abelov           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

cmake_minimum_required(VERSION 3.29)

set(CMAKE_C_STANDARD 17)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0075 NEW)
project("doodle" VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(include)

set(PROJECT_LDFLAGS
        -fsanitize=address,undefined
        -fsanitize=float-divide-by-zero,float-cast-overflow
        -fno-sanitize-recover=all)

set(PROJECT_CFLAGS
        -O0
        -fverbose-asm
        -mmanual-endbr
        -minline-all-stringops
        -fno-asynchronous-unwind-tables
        -fno-stack-clash-protection
        -fcf-protection=none
        -fno-stack-protector
        -fno-omit-frame-pointer
        -nostdlib
)

set(TARGET_EXE_NAME "${PROJECT_NAME}")
set(TARGET_CFLAGS ${PROJECT_CFLAGS})

file(GLOB TARGET_HEADERS "src/*.h")
file(GLOB TARGET_SOURCES "src/*.c")

add_executable(${TARGET_EXE_NAME} ${TARGET_HEADERS} ${TARGET_SOURCES})

target_include_directories(${TARGET_EXE_NAME} PRIVATE include src)
target_compile_options(${TARGET_EXE_NAME} PRIVATE ${PROJECT_CFLAGS})
target_link_options(${TARGET_EXE_NAME} PRIVATE ${PROJECT_LDFLAGS})

target_compile_definitions(${TARGET_EXE_NAME} PRIVATE
        CGI_DIR="/cgi-bin/"
)

set_target_properties(${TARGET_EXE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_NAME "${PROJECT_NAME}")
