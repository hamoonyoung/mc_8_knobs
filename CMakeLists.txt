cmake_minimum_required(VERSION 4.2)
include(/home/moonha/pico/pico-sdk/external/pico_sdk_import.cmake)

set(PROJECT_NAME "mc_8_knobs")

project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# WiFi login information here.
set(WIFI_SSID "HA!IoT")
set(WIFI_PASSWORD "EllieChloeAnnMoon3015")

# Set the board name (change if needed)
set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(${PROJECT_NAME} main.c
        usb_descriptors.c
)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC pico_stdlib
        #pico_cyw43_arch_none
        pico_cyw43_arch_lwip_threadsafe_background
        tinyusb_device
        tinyusb_board
)

# Define compile-time variables
target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

# Include directories
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/extra
        ${CMAKE_CURRENT_LIST_DIR}/extra/pico
        ${CMAKE_CURRENT_LIST_DIR}/extra/SEGGER_RTT
        ${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb/src
        ${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb/src/class/midi

)

target_sources(${PROJECT_NAME} PRIVATE
        extra/SEGGER_RTT/SEGGER_RTT.c
        extra/SEGGER_RTT/SEGGER_RTT_printf.c
)

include_directories(${CMAKE_SOURCE_DIR}/extra/SEGGER_RTT)

# ✅ FIXED: use the variable, not literal text
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)