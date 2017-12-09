set(NET_FILES

add_library(net OBJECT ${NET_FILES})

if(WITH_DPDK)
  target_compile_definitions(net PUBLIC "-DHAVE_DPDK")
  set_target_properties(net PROPERTIES
    COMPILE_FLAGS "-march=native -I${DPDK_INCLUDE_DIR} -Wno-invalid-offsetof")
endif(WITH_DPDK)
