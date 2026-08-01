# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD esp32s3_devkitc/esp32s3/procpu)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(node)

target_include_directories(app PRIVATE src/)
FILE(GLOB node_sources src/main.c
                       src/sensor.c
                       src/firmware_update.c
                       src/command.c
)
target_sources(app PRIVATE ${node_sources})
