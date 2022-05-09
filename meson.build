project('seastar', 'cpp', default_options: ['cpp_std=c++2a'])

# want src to be private
includes = include_directories(['include', 'src'])

# public def
seastar_cpp_args = [
  '-DSEASTAR_SCHEDULING_GROUPS_COUNT=@0@'.format(get_option('num_scheduling_groups')),
  '-DSEASTAR_API_LEVEL=@0@'.format(get_option('api_level'))]

# public def (compile flag)
if get_option('use_sstring')
  seastar_cpp_args += ['-DSEASTAR_SSTRING']
endif

# public def (compile def)
if get_option('task_backtrace')
  seastar_cpp_args += ['-DSEASTAR_TASK_BACKTRACE']
endif

boost = dependency('boost',
  modules: [
    'program_options',
    'thread',
    'filesystem',
    'unit_test_framework'])

cryptopp = dependency('libcrypto++', 'cryptopp')
fmt = dependency('fmt')
lz4 = dependency('liblz4')
gnutls = dependency('gnutls')
cares = dependency('libcares')
yaml = dependency('yaml-cpp')
sctp = dependency('libsctp')

cc = meson.get_compiler('cpp')
atomic_dep = cc.find_library('atomic', required: true)
cc.check_header('xfs/linux.h', required: true)
cc.check_header('valgrind/valgrind.h', required: true)
cc.check_header('sys/sdt.h', required: true)

subdir('include')

seastar_sources = files(
  'src/core/alien.cc',
  'src/core/file.cc',
  'src/core/fair_queue.cc',
  'src/core/reactor_backend.cc',
  'src/core/thread_pool.cc',
  'src/core/app-template.cc',
  'src/core/dpdk_rte.cc',
  'src/core/exception_hacks.cc',
  'src/core/execution_stage.cc',
  'src/core/fsnotify.cc',
  'src/core/fsqual.cc',
  'src/core/fstream.cc',
  'src/core/future.cc',
  'src/core/future-util.cc',
  'src/core/linux-aio.cc',
  'src/core/memory.cc',
  'src/core/metrics.cc',
  'src/core/on_internal_error.cc',
  'src/core/posix.cc',
  'src/core/prometheus.cc',
  'src/core/reactor.cc',
  'src/core/resource.cc',
  'src/core/sharded.cc',
  'src/core/scollectd.cc',
  'src/core/systemwide_memory_barrier.cc',
  'src/core/smp.cc',
  'src/core/sstring.cc',
  'src/core/thread.cc',
  'src/core/uname.cc',
  'src/core/io_queue.cc',
  'src/core/semaphore.cc',
  'src/core/condition-variable.cc',
  'src/core/program_options.cc',
  'src/http/api_docs.cc',
  'src/http/common.cc',
  'src/http/file_handler.cc',
  'src/http/httpd.cc',
  'src/http/json_path.cc',
  'src/http/matcher.cc',
  'src/http/mime_types.cc',
  'src/http/reply.cc',
  'src/http/routes.cc',
  'src/http/transformers.cc',
  'src/json/formatter.cc',
  'src/json/json_elements.cc',
  'src/net/arp.cc',
  'src/net/config.cc',
  'src/net/dhcp.cc',
  'src/net/dns.cc',
  'src/net/dpdk.cc',
  'src/net/ethernet.cc',
  'src/net/inet_address.cc',
  'src/net/ip.cc',
  'src/net/ip_checksum.cc',
  'src/net/native-stack.cc',
  'src/net/net.cc',
  'src/net/packet.cc',
  'src/net/posix-stack.cc',
  'src/net/proxy.cc',
  'src/net/socket_address.cc',
  'src/net/stack.cc',
  'src/net/tcp.cc',
  'src/net/tls.cc',
  'src/net/udp.cc',
  'src/net/unix_address.cc',
  'src/net/virtio.cc',
  'src/rpc/lz4_compressor.cc',
  'src/rpc/lz4_fragmented_compressor.cc',
  'src/rpc/rpc.cc',
  'src/util/alloc_failure_injector.cc',
  'src/util/backtrace.cc',
  'src/util/conversions.cc',
  'src/util/exceptions.cc',
  'src/util/file.cc',
  'src/util/log.cc',
  'src/util/program-options.cc',
  'src/util/read_first_line.cc',
  'src/util/tmp_file.cc',
  'src/util/short_streams.cc',
  'src/websocket/server.cc')

seastar_sources += http_parser_includes

seastar = library('seastar',
  seastar_sources,
  dependencies: [boost, fmt, cryptopp, lz4, gnutls, cares, yaml, atomic_dep,
    sctp],
  cpp_args: seastar_cpp_args + ['-Wno-non-virtual-dtor',
    '-Wno-bitwise-instead-of-logical', '-Wno-unused-but-set-variable',
    '-Wno-deprecated-declarations'],
  implicit_include_directories : false,
  include_directories: [includes])

seastar_dep = declare_dependency(
  sources: http_parser_includes,
  link_with: seastar,
  dependencies: [fmt, gnutls, lz4],
  compile_args: seastar_cpp_args,
  include_directories: [includes])

seastar_testing_sources = files(
  'src/testing/entry_point.cc',
  'src/testing/random.cc',
  'src/testing/seastar_test.cc',
  'src/testing/test_runner.cc')

seastar_testing = library('seastar_testing',
  seastar_testing_sources,
  dependencies: [boost],
  cpp_args: seastar_cpp_args + ['-Wno-non-virtual-dtor'],
  link_with: [seastar],
  implicit_include_directories : false,
  include_directories: [includes])

seastar_testing_dep = declare_dependency(
  link_with: seastar_testing,
  compile_args: seastar_cpp_args,
  dependencies: [seastar_dep, boost],
  include_directories: [includes])

seastar_perf_testing = library('seastar_perf_testing',
  'src/testing/random.cc',
  'tests/perf/linux_perf_event.cc',
  'tests/perf/perf_tests.cc',
  cpp_args: ['-Wno-non-virtual-dtor'],
  dependencies: [seastar_dep, boost, fmt])

seastar_perf_testing_dep = declare_dependency(
  link_with: seastar_perf_testing,
  compile_args: seastar_cpp_args,
  dependencies: [seastar_dep, boost, fmt],
  include_directories: [includes])

subdir('scripts')
