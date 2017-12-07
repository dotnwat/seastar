cmake_minimum_required(VERSION 3.5.1)
project(seastar CXX C)

# Wall

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

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

add_subdirectory(json)
add_subdirectory(net)
add_subdirectory(core)
add_subdirectory(http)
add_subdirectory(proto)

add_library(seastar STATIC
  $<TARGET_OBJECTS:core>
  $<TARGET_OBJECTS:json>
  $<TARGET_OBJECTS:net>
  $<TARGET_OBJECTS:http>
  $<TARGET_OBJECTS:metrics2_proto>)

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

add_subdirectory(apps)
add_subdirectory(tests)
