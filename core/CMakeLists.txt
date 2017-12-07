set(CORE_FILES
  reactor.cc
  systemwide_memory_barrier.cc
  fstream.cc
  posix.cc
  memory.cc
  resource.cc
  scollectd.cc
  metrics.cc
  app-template.cc
  thread.cc
  dpdk_rte.cc
  fsqual.cc
  exception_hacks.cc
  prometheus.cc
  ../net/packet.cc
  ../net/posix-stack.cc
  ../net/net.cc
  ../net/stack.cc
  ../net/inet_address.cc
  ../rpc/rpc.cc
  ../rpc/lz4_compressor.cc
  ../util/conversions.cc
  ../util/program-options.cc
  ../util/log.cc
  ../util/backtrace.cc
  ../util/alloc_failure_injector.cc)

add_library(core OBJECT ${CORE_FILES})

if(WITH_DPDK)
  target_compile_definitions(core PUBLIC "-DHAVE_DPDK")
  set_target_properties(core PROPERTIES COMPILE_FLAGS "-I${DPDK_INCLUDE_DIR}")
endif(WITH_DPDK)
