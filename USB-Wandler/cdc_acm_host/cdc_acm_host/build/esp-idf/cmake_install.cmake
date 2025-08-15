# Install script for directory: C:/Espressif/frameworks/esp-idf-v5.4

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/cdc_acm_host")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119/xtensa-esp-elf/bin/xtensa-esp32s3-elf-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/xtensa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_driver_gpio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_timer/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_pm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/bootloader/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esptool_py/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/partition_table/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_app_format/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_bootloader_format/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/app_update/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_partition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/efuse/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/bootloader_support/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_mm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/spi_flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_system/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_common/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_rom/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/hal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/log/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/heap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_security/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/esp_hw_support/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/freertos/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/newlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/pthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/cxx/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/espressif__usb_host_cdc_acm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Kilian Peters/OneDrive/Projekte/SoC-Monitor Schwalbe/cdc_acm_host/cdc_acm_host/build/esp-idf/main/cmake_install.cmake")
endif()

