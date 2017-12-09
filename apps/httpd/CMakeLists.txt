swagger_target(demo.json demo.json.hh)
add_executable(httpd main.cc demo.json.hh)
target_link_libraries(httpd seastar)
