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
#pragma once

#include <span>
#include <unordered_set>

#include <seastar/core/future.hh>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/net/tls.hh>
#include <seastar/net/stack.hh>

namespace seastar::tls {

/// Abstract interface for a TLS session.
///
/// This is the primary abstraction that TLS backends (GnuTLS, OpenSSL)
/// implement. Generic wrapper classes delegate all TLS operations
/// through this interface.
class session_impl {
public:
    virtual ~session_impl() = default;
    virtual future<> put(std::span<temporary_buffer<char>> bufs) = 0;
    virtual future<> flush() = 0;
    virtual future<temporary_buffer<char>> get() = 0;
    virtual void close() = 0;
    virtual seastar::net::connected_socket_impl& socket() const = 0;
    virtual future<std::optional<session_dn>> get_distinguished_name() = 0;
    virtual future<std::vector<subject_alt_name>> get_alt_name_information(
        std::unordered_set<subject_alt_name_type> types) = 0;
    virtual future<bool> is_resumed() = 0;
    virtual future<session_data> get_session_resume_data() = 0;
    virtual future<std::vector<certificate_data>> get_peer_certificate_chain() = 0;
    virtual future<std::optional<sstring>> get_selected_alpn_protocol() = 0;
    virtual future<sstring> get_cipher_suite() = 0;
    virtual future<sstring> get_protocol_version() = 0;
    virtual future<> force_rehandshake() = 0;
};

} // namespace seastar::tls
