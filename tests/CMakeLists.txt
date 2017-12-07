add_library(boost_test_utils OBJECT
  test-utils.cc
  test_runner.cc)

macro(make_test name)
  add_executable(seastar_${name} ${name}.cc)
  target_link_libraries(seastar_${name} seastar)
  install(TARGETS seastar_${name} DESTINATION bin)
endmacro(make_test)

macro(make_test_boost name)
  make_test(${name})
  target_sources(seastar_${name} PUBLIC $<TARGET_OBJECTS:boost_test_utils>)
  target_link_libraries(seastar_${name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endmacro(make_test_boost)

set(TESTS
  directory_test
  linecount
  echotest
  l3_test
  ip_test
  timertest
  tcp_test
  smp_test
  thread_context_switch
  udp_server
  udp_client
  blkdiscard_test
  sstring_test
  unwind_test
  defer_test
  tcp_sctp_server
  tcp_sctp_client
  allocator_test
  udp_zero_copy
  shared_ptr_test
  weak_ptr_test
  checked_ptr_test
  slab_test
  distributed_test
  rpc
  packet_test
  chunked_fifo_test
  circular_buffer_test
  program_options_test
  tuple_utils_test
  tls_echo_server
  tls_simple_client
  circular_buffer_fixed_capacity_test
  noncopyable_function_test
  scheduling_group_demo)

foreach(testname ${TESTS})
  make_test(${testname})
endforeach()

if(WITH_DPDK)
  target_compile_definitions(seastar_echotest PUBLIC "-DHAVE_DPDK")
endif(WITH_DPDK)

add_subdirectory(perf)

set(BOOST_TESTS
  fileiotest
  futures_test
  alloc_test
  foreign_ptr_test
  semaphore_test
  expiring_fifo_test
  thread_test
  fair_queue_test
  output_stream_test
  fstream_test
  rpc_test
  connect_test
  dns_test
  tls_test
  httpd
  json_formatter_test
  execution_stage_test
  lowres_clock_test)

foreach(testname ${BOOST_TESTS})
  make_test_boost(${testname})
endforeach()

add_subdirectory(memcached)
