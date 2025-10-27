# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD weact_studio_rp2040_16mb CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(EInkMp3 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(EInkMp3 EInkMp3.cpp )

target_compile_definitions(EInkMp3 PUBLIC PICO_FLASH_SIZE_BYTES=16777216)

pico_set_program_name(EInkMp3 "EInkMp3")
pico_set_program_version(EInkMp3 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(EInkMp3 0)
pico_enable_stdio_usb(EInkMp3 1)

#Add Subdirs
add_subdirectory(libs/epdDraw)
add_subdirectory(libs/pico_fatfs)
add_subdirectory(libs/fileManager)
add_subdirectory(libs/audioPlayer)
add_subdirectory(libs/dr_impl)

# Add the standard library to the build
target_link_libraries(EInkMp3 pico_stdlib)

# Add the standard include files to the build
target_include_directories(EInkMp3 
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(EInkMp3
    epdDraw
    pico_fatfs
    fileManager
    audioPlayer
)

pico_add_extra_outputs(EInkMp3)