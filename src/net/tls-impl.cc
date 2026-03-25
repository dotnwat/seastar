/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright 2015 Cloudius Systems
 */

#include <sys/stat.h>

#include <seastar/core/file.hh>

#include "tls-impl.hh"

namespace seastar {

future<file_result> read_fully(const sstring& name, const sstring& what) {
    return open_file_dma(name, open_flags::ro).then([name = name](file f) mutable {
        return do_with(std::move(f), [name = std::move(name)](file& f) mutable {
            return f.stat().then([&f, name = std::move(name)](struct stat s) mutable {
                return f.dma_read_bulk<char>(0, s.st_size).then([s, name = std::move(name)](temporary_buffer<char> buf) mutable {
                    return file_result{ std::move(buf), file_info{
                        std::move(name), std::chrono::system_clock::from_time_t(s.st_mtim.tv_sec) +
                            std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(s.st_mtim.tv_nsec))
                    } };
                });
            }).finally([&f]() {
                return f.close();
            });
        });
    }).handle_exception([name = name, what = what](std::exception_ptr ep) -> future<file_result> {
       try {
           std::rethrow_exception(std::move(ep));
       } catch (...) {
           std::throw_with_nested(std::runtime_error(sstring("Could not read ") + what + " " + name));
       }
    });
}

data_source tls::tls_connected_socket_impl::source() {
    return data_source(std::make_unique<source_impl>(_session));
}

data_sink tls::tls_connected_socket_impl::sink() {
    return data_sink(std::make_unique<sink_impl>(_session));
}

}
