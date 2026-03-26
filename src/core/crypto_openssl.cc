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
 * Copyright 2024 Redpanda Data
 */

#include "crypto.hh"
#include "../net/tls-impl.hh"
#include "../net/tls_openssl.hh"
#include <seastar/net/stack.hh>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdexcept>
#include <fmt/format.h>

namespace seastar::crypto {

class openssl_tls_backend final : public tls_backend {
public:
    shared_ptr<tls::session_impl> make_session(
            tls::session_type type,
            shared_ptr<tls::certificate_credentials> creds,
            std::unique_ptr<net::connected_socket_impl> sock,
            const tls::tls_options& options) override;
    const std::error_category& error_category() override;
    std::vector<uint8_t> generate_session_ticket_key() override;
    shared_ptr<tls::credentials_impl> make_credentials_impl() override;
    std::unique_ptr<tls::dh_params_impl> make_dh_params(tls::dh_params::level) override;
    std::unique_ptr<tls::dh_params_impl> make_dh_params(const tls::blob&, tls::x509_crt_format) override;
};

class openssl_crypto_provider final : public crypto_provider {
public:
    std::string sha1_hash(std::string_view input) override;
    std::string base64_encode(std::string_view input) override;
    tls_backend& get_tls_backend() override;
private:
    openssl_tls_backend _tls_backend;
};

std::string openssl_crypto_provider::sha1_hash(std::string_view input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    auto* md = EVP_sha1();
    if (!EVP_Digest(input.data(), input.size(), hash, &hash_len, md, nullptr)) {
        throw std::runtime_error("EVP_Digest(SHA1) failed");
    }
    return std::string(reinterpret_cast<const char*>(hash), hash_len);
}

std::string openssl_crypto_provider::base64_encode(std::string_view input) {
    auto* bio_b64 = BIO_new(BIO_f_base64());
    auto* bio_mem = BIO_new(BIO_s_mem());
    bio_b64 = BIO_push(bio_b64, bio_mem);
    BIO_set_flags(bio_b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio_b64, input.data(), input.size());
    BIO_flush(bio_b64);
    char* buf = nullptr;
    long len = BIO_get_mem_data(bio_mem, &buf);
    std::string result(buf, len);
    BIO_free_all(bio_b64);
    return result;
}

shared_ptr<tls::session_impl> openssl_tls_backend::make_session(
        tls::session_type type,
        shared_ptr<tls::certificate_credentials> creds,
        std::unique_ptr<net::connected_socket_impl> sock,
        const tls::tls_options& options) {
    return tls::openssl::make_session(type, std::move(creds), std::move(sock), options);
}

const std::error_category& openssl_tls_backend::error_category() {
    return tls::openssl::error_category();
}

std::vector<uint8_t> openssl_tls_backend::generate_session_ticket_key() {
    return tls::openssl::generate_session_ticket_key();
}

shared_ptr<tls::credentials_impl> openssl_tls_backend::make_credentials_impl() {
    return tls::openssl::make_credentials_impl();
}

std::unique_ptr<tls::dh_params_impl> openssl_tls_backend::make_dh_params(tls::dh_params::level lvl) {
    return tls::openssl::make_dh_params(lvl);
}

std::unique_ptr<tls::dh_params_impl> openssl_tls_backend::make_dh_params(const tls::blob& b, tls::x509_crt_format fmt) {
    return tls::openssl::make_dh_params(b, fmt);
}

tls_backend& openssl_crypto_provider::get_tls_backend() {
    return _tls_backend;
}

std::unique_ptr<crypto_provider> create_openssl_provider() {
    return std::make_unique<openssl_crypto_provider>();
}

} // namespace seastar::crypto
