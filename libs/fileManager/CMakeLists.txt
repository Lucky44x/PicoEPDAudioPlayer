project(fileManager)

add_library(fileManager STATIC fileManager.cpp)

target_link_libraries(fileManager pico_stdlib pico_fatfs dr_impl)

target_include_directories(fileManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})