ragel_target(request_parser
  request_parser.rl
  ${CMAKE_CURRENT_BINARY_DIR}/request_parser_raw.hh
  COMPILE_FLAGS "-G2")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/request_parser.hh
  COMMAND sed
  ARGS -e '1h\;2,$$H\;$$!d\;g' -re 's/static const char _nfa[^;]*\;//g' ${CMAKE_CURRENT_BINARY_DIR}/request_parser_raw.hh > ${CMAKE_CURRENT_BINARY_DIR}/request_parser.hh
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/request_parser_raw.hh
  COMMENT "Applying fix to Ragel output")

ragel_target(http_response_parser
  http_response_parser.rl
  ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser_raw.hh
  COMPILE_FLAGS "-G2")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser.hh
  COMMAND sed
  ARGS -e '1h\;2,$$H\;$$!d\;g' -re 's/static const char _nfa[^;]*\;//g' ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser_raw.hh > ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser.hh
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser_raw.hh
  COMMENT "Applying fix to Ragel output")

set(HTTP_FILES
  transformers.cc
  json_path.cc
  file_handler.cc
  common.cc
  routes.cc
  matcher.cc
  mime_types.cc
  httpd.cc
  reply.cc
  api_docs.cc
  ${CMAKE_CURRENT_BINARY_DIR}/request_parser.hh
  ${CMAKE_CURRENT_BINARY_DIR}/http_response_parser.hh)

add_library(http OBJECT ${HTTP_FILES})
