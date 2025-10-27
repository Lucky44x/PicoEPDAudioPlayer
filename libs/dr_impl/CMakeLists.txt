project(dr_impl)

#add_library(dr_impl STATIC dr_mp3_impl.c)
add_library(dr_impl STATIC dr_wav_impl.c)

target_link_libraries(dr_impl pico_stdlib pico_fatfs)

target_include_directories(dr_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})