cmake_minimum_required(VERSION 3.5.1)
project(seastar CXX C)

# Wall

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/")

find_package(GnuTLS REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system program_options unit_test_framework
  thread filesystem REQUIRED)
find_package(aio)
find_package(CryptoPP REQUIRED)
find_package(Ragel REQUIRED)
find_package(LZ4 1.7 REQUIRED)

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES pthread)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)

include(Swagger)

pkg_check_modules(yaml_cpp REQUIRED yaml-cpp)

find_package(Protobuf REQUIRED)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
  message(FATAL_ERROR "cannot find protobuf compiler")
endif()

add_subdirectory(fmt)
add_subdirectory(c-ares)

# TODO: add the external version linking
option(WITH_DPDK "Enable DPDK" OFF)
if(WITH_DPDK)
  include(BuildDPDK)
  build_dpdk()
endif()

include_directories(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/fmt)

set(EXTRALIBS ${CMAKE_DL_LIBS})
if(NOT APPLE)
  list(APPEND EXTRALIBS rt)
endif()

# FIXME: detect unwind
list(APPEND EXTRALIBS unwind)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
# we use `asm()` to inline assembly, so enable the GNU extension
#set(CMAKE_C_EXTENSIONS ON)
set(C_STANDARD_REQUIRED ON)

add_subdirectory(proto)

set(NET_FILES
  net/proxy.cc
  net/virtio.cc
  net/dpdk.cc
  net/ip.cc
  net/ethernet.cc
  net/arp.cc
  net/native-stack.cc
  net/ip_checksum.cc
  net/udp.cc
  net/tcp.cc
  net/dhcp.cc
  net/tls.cc
  net/dns.cc
  net/packet.cc
  net/posix-stack.cc
  net/net.cc
  net/stack.cc
  net/inet_address.cc)

set(CORE_FILES
  core/reactor.cc
  core/systemwide_memory_barrier.cc
  core/fstream.cc
  core/posix.cc
  core/memory.cc
  core/resource.cc
  core/scollectd.cc
  core/metrics.cc
  core/app-template.cc
  core/thread.cc
  core/dpdk_rte.cc
  core/fsqual.cc
  core/exception_hacks.cc
  core/prometheus.cc)

set(RPC_FILES
  rpc/rpc.cc
  rpc/lz4_compressor.cc)

set(UTIL_FILES
  util/conversions.cc
  util/program-options.cc
  util/log.cc
  util/backtrace.cc
  util/alloc_failure_injector.cc)

ragel_target(request_parser
  http/request_parser.rl
  http/request_parser.hh
  COMPILE_FLAGS "-G2")

ragel_target(http_response_parser
  http/http_response_parser.rl
  http/http_response_parser.hh
  COMPILE_FLAGS "-G2")

set(HTTP_FILES
  http/transformers.cc
  http/json_path.cc
  http/file_handler.cc
  http/common.cc
  http/routes.cc
  http/matcher.cc
  http/mime_types.cc
  http/httpd.cc
  http/reply.cc
  http/api_docs.cc
  http/request_parser.hh
  http/http_response_parser.hh)

set(JSON_FILES
  json/formatter.cc
  json/json_elements.cc)

add_library(seastar STATIC
  ${CORE_FILES}
  ${NET_FILES}
  ${RPC_FILES}
  ${UTIL_FILES}
  ${HTTP_FILES}
  ${JSON_FILES}
  $<TARGET_OBJECTS:metrics2_proto>)

target_include_directories(seastar PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>)

target_link_libraries(seastar
  fmt
  c-ares
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${EXTRALIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${AIO_LIBRARIES}
  ${CRYPTOPP_LIBRARIES}
  ${GNUTLS_LIBRARIES}
  ${LZ4_LIBRARY}
  ${PROTOBUF_LIBRARIES}
)

if(WITH_DPDK)
  target_link_libraries(seastar ${DPDK_LIBRARIES})
endif(WITH_DPDK)

option(SEASTAR_WITH_TESTS "Enable building apps and tests" ON)
if(SEASTAR_WITH_TESTS)
  add_subdirectory(apps)
  add_subdirectory(tests)
endif(SEASTAR_WITH_TESTS)
