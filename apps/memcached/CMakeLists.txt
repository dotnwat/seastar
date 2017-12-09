ragel_target(ascii ascii.rl ascii.hh COMPILE_FLAGS "-G2")
add_executable(memcached memcache.cc ascii.hh)
target_link_libraries(memcached seastar)
