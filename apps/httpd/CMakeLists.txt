swagger_target(demo.json demo.json.hh)
add_executable(httpd
  main.cc
  ${CMAKE_CURRENT_BINARY_DIR}/demo.json.hh)
target_link_libraries(httpd seastar)
