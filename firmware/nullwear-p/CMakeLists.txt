#
# Project NULLWEAR — Application-core CMake build script.
#
# Builds against Nordic nRF Connect SDK (NCS) v2.5+ with Zephyr v3.4+.
# Target board: nrf5340dk_nrf5340_cpuapp (development) or
# nullwear_p_cpuapp (production board, devicetree provided in board/).
#
# Typical build:
#
#   west build -b nrf5340dk_nrf5340_cpuapp \
#       -- -DEXTRA_CONF_FILE=overlay-release.conf
#
# Flash:
#
#   west flash --erase
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.20.0)

# Multi-image build: app core + network core. The network core image is
# specified via NRF5340_CHILD_IMAGE_PATH so NCS builds them together.
set(NRF5340_CHILD_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/network_core")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nullwear_p
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Project NULLWEAR — Personal-issue BLE annihilation device")

target_sources(app PRIVATE
    src/main.c
    src/battery_mgmt.c
    src/led_driver.c
    src/state_machine.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

zephyr_compile_definitions(NULLWEAR_VERSION="${PROJECT_VERSION}")
