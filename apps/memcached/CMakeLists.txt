ragel_target(ascii
  ascii.rl
  ${CMAKE_CURRENT_BINARY_DIR}/ascii_raw.hh
  COMPILE_FLAGS "-G2")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ascii.hh
  COMMAND sed
  ARGS -e '1h\;2,$$H\;$$!d\;g' -re 's/static const char _nfa[^;]*\;//g' ${CMAKE_CURRENT_BINARY_DIR}/ascii_raw.hh > ${CMAKE_CURRENT_BINARY_DIR}/ascii.hh
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ascii_raw.hh
  COMMENT "Applying fix to Ragel output")

add_executable(memcached memcache.cc ${CMAKE_CURRENT_BINARY_DIR}/ascii.hh)
target_link_libraries(memcached seastar)
