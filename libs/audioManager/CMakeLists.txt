project(audioManager)

add_library(audioManager STATIC audioManager.cpp)

add_library(pico_util_buffer STATIC
    pico_util_buffer/buffer.c
)
target_include_directories(pico_util_buffer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pico_util_buffer/include
)

add_subdirectory(pico_audio_32b pico_audio_32b_build)
add_subdirectory(pico_audio_i2s_32b pico_audio_i2s_32b_build)

target_link_libraries(audioManager pico_stdlib fileManager pico_audio_i2s_32b pico_audio_32b dr_impl)

target_include_directories(audioManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})