cmake_minimum_required(VERSION 3.16.0)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(ENABLE_CCACHE "1")
set(DATA_DIR "data")
set(LFS_PARTITION_NAME littlefs)

project(esp_camera_server)

message(STATUS "Creating LittleFS image from ${DATA_DIR}")
littlefs_create_partition_image(${LFS_PARTITION_NAME} ${DATA_DIR} FLASH_IN_PROJECT)