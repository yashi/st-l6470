# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD nucleo_f429zi)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(micro-ros-zephyr-demo)

target_sources(app PRIVATE
  src/l64x0.c
  src/main.c)
