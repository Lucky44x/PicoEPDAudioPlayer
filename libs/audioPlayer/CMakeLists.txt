project(audioPlayer)

add_compile_definitions(CORE1_PROCESS_I2S_CALLBACK)

add_library(audioPlayer STATIC audioPlayer.c)

add_library(pico_util_buffer STATIC
    pico_util_buffer/buffer.c
)
target_include_directories(pico_util_buffer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pico_util_buffer/include
)

add_subdirectory(pico_audio_32b pico_audio_32b_build)
add_subdirectory(pico_audio_i2s_32b pico_audio_i2s_32b_build)

target_link_libraries(audioPlayer pico_stdlib pico_audio_i2s_32b pico_audio_32b )

target_include_directories(audioPlayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})