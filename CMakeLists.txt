# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    # "../common/app_reset"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/matter"
    ${extra_components_dirs_append})

set(COMPONENTS cxx app bootloader bootloader_support
       app_trace app_update efuse esp32 esp_common 
       esp_event esp_rom esp_partition espcoredump esptool_py freertos heap log 
       pthread perfmon vfs spi_flash nvs_flash
       main)

project(mLightComponent)


# WARNING: This is just an example for using key for decrypting the encrypted OTA image
# Please do not use it as is.
if(CONFIG_ENABLE_ENCRYPTED_OTA)
    target_add_binary_data(light.elf "esp_image_encryption_key.pem" TEXT)
endif()

idf_build_set_property(CXX_COMPILE_OPTIONS "-Os;-DCHIP_HAVE_CONFIG_H" APPEND)
#idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For RISCV chips, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
