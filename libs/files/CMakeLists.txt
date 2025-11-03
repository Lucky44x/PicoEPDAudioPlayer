project(files)

add_library(files STATIC files.cpp)

target_link_libraries(files pico_stdlib pico_fatfs dr_impl)

target_include_directories(files PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})