add_executable(ply_read read.cc)
add_test(NAME ply_read COMMAND ply_read WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_options(ply_read
  PRIVATE "-DASSETS_DIR=\"${CMAKE_SOURCE_DIR}/assets\"")
target_link_libraries(ply_read PRIVATE tinyply)
