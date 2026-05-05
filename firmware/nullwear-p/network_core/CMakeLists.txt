#
# Project NULLWEAR — Network-core CMake build
#
# This image runs on the nRF5340 network core (cpunet) and owns the
# RADIO peripheral. It is invoked automatically by the app-core build
# via NRF5340_CHILD_IMAGE_PATH.
#
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nullwear_p_net
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "NULLWEAR/P network-core radio jammer")

target_sources(app PRIVATE
    src/main.c
    src/radio_jammer.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
