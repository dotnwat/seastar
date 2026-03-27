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
#include <openssl/evp.h>
#include <seastar/core/sstring.hh>
#include <seastar/util/assert.hh>
#include <seastar/util/defer.hh>
#include <fmt/format.h>

namespace seastar::crypto {

class openssl_crypto_provider final : public crypto_provider {
public:
    sstring sha1_hash(std::string_view input) override;
    sstring base64_encode(std::string_view input) override;
};

sstring openssl_crypto_provider::sha1_hash(std::string_view input) {
    auto md_ptr = EVP_MD_fetch(nullptr, "SHA1", nullptr);
    if (!md_ptr) {
        throw std::runtime_error("Failed to fetch SHA-1 algorithm from OpenSSL");
    }
    auto free_evp_md_ptr = defer([&]() noexcept { EVP_MD_free(md_ptr); });

    unsigned char hash[20];
    unsigned int hash_size = sizeof(hash);
    SEASTAR_ASSERT(hash_size == static_cast<unsigned int>(EVP_MD_get_size(md_ptr)));

    if (1 != EVP_Digest(input.data(), input.size(), hash, &hash_size, md_ptr, nullptr)) {
        throw std::runtime_error("Failed to perform SHA-1 digest in OpenSSL");
    }

    return sstring(reinterpret_cast<const char*>(hash), sizeof(hash));
}

sstring openssl_crypto_provider::base64_encode(std::string_view input) {
    const auto encode_capacity = [](size_t input_size) {
        return (((4 * input_size) / 3) + 3) & ~0x3U;
    };
    auto base64_encoded = uninitialized_string<sstring>(encode_capacity(input.size()));
    EVP_EncodeBlock(reinterpret_cast<unsigned char *>(base64_encoded.data()), reinterpret_cast<const unsigned char *>(input.data()), input.size());
    return base64_encoded;
}

std::unique_ptr<crypto_provider> create_openssl_provider() {
    return std::make_unique<openssl_crypto_provider>();
}

} // namespace seastar::crypto
