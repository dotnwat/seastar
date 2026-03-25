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

#include "crypto.hh"
#include "../net/tls_gnutls.hh"
#include <seastar/net/tls.hh>
#include <seastar/net/stack.hh>
#include <seastar/util/defer.hh>
#include <gnutls/crypto.h>
#include <gnutls/gnutls.h>
#include <stdexcept>
#include <fmt/format.h>

namespace seastar::crypto {

class gnutls_tls_backend final : public tls_backend {
public:
    shared_ptr<tls::session_impl> make_session(
            tls::session_type type,
            shared_ptr<tls::certificate_credentials> creds,
            std::unique_ptr<net::connected_socket_impl> sock,
            const tls::tls_options& options) override;
    const std::error_category& error_category() override;
    std::vector<uint8_t> generate_session_ticket_key() override;
};

class gnutls_crypto_provider final : public crypto_provider {
public:
    sstring sha1_hash(std::string_view input) override;
    sstring base64_encode(std::string_view input) override;
    tls_backend& get_tls_backend() override;
private:
    gnutls_tls_backend _tls_backend;
};

sstring gnutls_crypto_provider::sha1_hash(std::string_view input) {
    unsigned char hash[20];
    if (int ret = gnutls_hash_fast(GNUTLS_DIG_SHA1, input.data(), input.size(), hash);
        ret != GNUTLS_E_SUCCESS) {
        throw std::runtime_error(fmt::format("gnutls_hash_fast: {}", gnutls_strerror(ret)));
    }
    return sstring(reinterpret_cast<const char*>(hash), sizeof(hash));
}

sstring gnutls_crypto_provider::base64_encode(std::string_view input) {
    gnutls_datum_t src_data{
        .data = reinterpret_cast<uint8_t*>(const_cast<char*>(input.data())),
        .size = static_cast<unsigned>(input.size())
    };
    gnutls_datum_t encoded_data;
    if (int ret = gnutls_base64_encode2(&src_data, &encoded_data); ret != GNUTLS_E_SUCCESS) {
        throw std::runtime_error(fmt::format("gnutls_base64_encode2: {}", gnutls_strerror(ret)));
    }
    auto free_encoded_data = defer([&] () noexcept { gnutls_free(encoded_data.data); });
    return sstring(reinterpret_cast<const char*>(encoded_data.data), encoded_data.size);
}

shared_ptr<tls::session_impl> gnutls_tls_backend::make_session(
        tls::session_type type,
        shared_ptr<tls::certificate_credentials> creds,
        std::unique_ptr<net::connected_socket_impl> sock,
        const tls::tls_options& options) {
    return tls::gnutls::make_session(type, std::move(creds), std::move(sock), options);
}

const std::error_category& gnutls_tls_backend::error_category() {
    return tls::gnutls::error_category();
}

std::vector<uint8_t> gnutls_tls_backend::generate_session_ticket_key() {
    return tls::gnutls::generate_session_ticket_key();
}

tls_backend& gnutls_crypto_provider::get_tls_backend() {
    return _tls_backend;
}

std::unique_ptr<crypto_provider> create_gnutls_provider() {
    return std::make_unique<gnutls_crypto_provider>();
}

} // namespace seastar::crypto
