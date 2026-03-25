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
 * Copyright 2024 ScyllaDB
 */
#pragma once

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>

#include <seastar/core/shared_ptr.hh>

namespace seastar::net { class connected_socket_impl; }
namespace seastar::tls {
    class session_impl;
    class certificate_credentials;
    enum class session_type;
    struct tls_options;
}

namespace seastar::tls::gnutls {

/// Create a GnuTLS TLS session.
shared_ptr<session_impl> make_session(
    session_type type,
    shared_ptr<certificate_credentials> creds,
    std::unique_ptr<net::connected_socket_impl> sock,
    const tls_options& options);

/// Return the GnuTLS error category.
const std::error_category& error_category();

/// Generate a session ticket key using GnuTLS.
std::vector<uint8_t> generate_session_ticket_key();

} // namespace seastar::tls::gnutls
