macro(register_test NAME)
  set(test_name ply_${NAME})
  add_executable(${test_name} ${NAME}.cc)
  add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  target_compile_options(${test_name}
    PRIVATE "-DASSETS_DIR=\"${CMAKE_SOURCE_DIR}/assets\"")
  target_link_libraries(${test_name} PRIVATE tinyply)
endmacro()

register_test(read)
register_test(write)
