ragel = find_program('ragel')

parsers = [
  'chunk_parsers',
  'request_parser',
  'response_parser']

http_parser_includes = []

foreach name : parsers
  t = custom_target(
    input: f'@name@.rl',
    output: f'@name@.hh',
    command: [ragel, '-G2', '-o', '@OUTPUT@', '@INPUT@'])
  http_parser_includes += [t]
endforeach
