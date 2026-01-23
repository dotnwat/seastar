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
 * Copyright 2025-present ScyllaDB
 */

#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <seastar/util/assert.hh>
#include <utility>

/// \file
/// \brief Infrastructure for deterministic simulation testing (DST) context-local storage.
///
/// In DST mode, thread-local variables are replaced with arrays indexed by a
/// context ID. This allows a single physical thread to multiplex multiple
/// logical shards/reactors, with instant "context switching" by changing the
/// current context index.
///
/// In non-DST mode, these utilities compile down to regular thread-local storage.

namespace seastar {
namespace dst {

/// Maximum number of simulated contexts (shards) supported in DST mode.
/// This is a compile-time constant to avoid dynamic allocation in hot paths.
inline constexpr std::size_t max_contexts = 64;

#ifdef SEASTAR_DST

/// The currently active context ID. Changing this value effectively
/// "switches" which logical shard/reactor is active.
inline thread_local std::size_t current_context = 0;

/// Switch to a different context. This is the "context switch" operation
/// in DST mode - it's just an integer assignment.
inline void switch_context(std::size_t ctx) noexcept {
    current_context = ctx;
}

/// Get the current context ID.
inline std::size_t get_current_context() noexcept {
    return current_context;
}

/// Counter for allocating unique context IDs to threads.
/// Starts at 1 because context 0 is reserved for the main thread.
inline std::atomic<std::size_t> next_context_id{1};

/// Allocate a unique context ID for the current thread.
/// Call this once at thread entry point.
inline void allocate_context() noexcept {
    current_context = next_context_id.fetch_add(1);
    SEASTAR_ASSERT(current_context < max_contexts);
}

/// A context-local variable wrapper for value types.
/// In DST mode, stores an array of values indexed by context ID.
/// Access via get() or the conversion operators.
template<typename T>
class context_local {
    std::array<T, max_contexts> _storage{};
public:
    context_local() = default;
    explicit context_local(const T& init) {
        _storage.fill(init);
    }

    T& get() noexcept { return _storage[current_context]; }
    const T& get() const noexcept { return _storage[current_context]; }

    // Allow direct access to specific context (for initialization, debugging)
    T& get(std::size_t ctx) noexcept { return _storage[ctx]; }
    const T& get(std::size_t ctx) const noexcept { return _storage[ctx]; }

    // Implicit conversion to T& for seamless usage
    operator T&() noexcept { return get(); }
    operator const T&() const noexcept { return get(); }

    // Assignment
    context_local& operator=(const T& val) {
        get() = val;
        return *this;
    }

    // Pointer-like member access (for method calls: container->method())
    T* operator->() noexcept { return &get(); }
    const T* operator->() const noexcept { return &get(); }

    // Forwarding operator[] for indexable types (containers, arrays)
    template<typename K>
    auto operator[](K&& key) -> decltype(get()[std::forward<K>(key)]) {
        return get()[std::forward<K>(key)];
    }
    template<typename K>
    auto operator[](K&& key) const -> decltype(get()[std::forward<K>(key)]) {
        return get()[std::forward<K>(key)];
    }

    // Forwarding operator() for callable types (distributions, functors)
    template<typename... Args>
    auto operator()(Args&&... args) -> decltype(get()(std::forward<Args>(args)...)) {
        return get()(std::forward<Args>(args)...);
    }
    template<typename... Args>
    auto operator()(Args&&... args) const -> decltype(get()(std::forward<Args>(args)...)) {
        return get()(std::forward<Args>(args)...);
    }
};

/// A context-local variable wrapper for pointer types.
/// Provides operator-> and operator* for pointer-like semantics.
template<typename T>
class context_local_ptr {
    std::array<T*, max_contexts> _storage{};
public:
    context_local_ptr() = default;
    explicit context_local_ptr(T* init) {
        _storage.fill(init);
    }

    T*& get() noexcept { return _storage[current_context]; }
    T* const& get() const noexcept { return _storage[current_context]; }

    // Allow direct access to specific context (for initialization, debugging)
    T*& get(std::size_t ctx) noexcept { return _storage[ctx]; }
    T* const& get(std::size_t ctx) const noexcept { return _storage[ctx]; }

    // Pointer-like access
    T* operator->() const noexcept { return _storage[current_context]; }
    T& operator*() const noexcept { return *_storage[current_context]; }

    // Implicit conversion to T* for seamless usage
    operator T*() const noexcept { return _storage[current_context]; }

    // Assignment
    context_local_ptr& operator=(T* val) noexcept {
        get() = val;
        return *this;
    }

    // Comparison with nullptr
    bool operator==(std::nullptr_t) const noexcept { return get() == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return get() != nullptr; }

    // Boolean conversion
    explicit operator bool() const noexcept { return get() != nullptr; }
};

#else // !SEASTAR_DST

// In non-DST mode, these are no-ops or simple wrappers

inline void switch_context(std::size_t) noexcept {}
inline std::size_t get_current_context() noexcept { return 0; }
inline void allocate_context() noexcept {}

/// In non-DST mode, context_local is just a thin wrapper around a value.
template<typename T>
class context_local {
    T _storage{};
public:
    context_local() = default;
    explicit context_local(const T& init) : _storage(init) {}

    T& get() noexcept { return _storage; }
    const T& get() const noexcept { return _storage; }
    T& get(std::size_t) noexcept { return _storage; }
    const T& get(std::size_t) const noexcept { return _storage; }

    operator T&() noexcept { return _storage; }
    operator const T&() const noexcept { return _storage; }

    context_local& operator=(const T& val) {
        _storage = val;
        return *this;
    }

    // Pointer-like member access (for method calls: container->method())
    T* operator->() noexcept { return &_storage; }
    const T* operator->() const noexcept { return &_storage; }

    // Forwarding operator[] for indexable types (containers, arrays)
    template<typename K>
    auto operator[](K&& key) -> decltype(_storage[std::forward<K>(key)]) {
        return _storage[std::forward<K>(key)];
    }
    template<typename K>
    auto operator[](K&& key) const -> decltype(_storage[std::forward<K>(key)]) {
        return _storage[std::forward<K>(key)];
    }

    // Forwarding operator() for callable types (distributions, functors)
    template<typename... Args>
    auto operator()(Args&&... args) -> decltype(_storage(std::forward<Args>(args)...)) {
        return _storage(std::forward<Args>(args)...);
    }
    template<typename... Args>
    auto operator()(Args&&... args) const -> decltype(_storage(std::forward<Args>(args)...)) {
        return _storage(std::forward<Args>(args)...);
    }
};

/// In non-DST mode, context_local_ptr is just a thin wrapper around a pointer.
template<typename T>
class context_local_ptr {
    T* _storage{};
public:
    context_local_ptr() = default;
    explicit context_local_ptr(T* init) : _storage(init) {}

    T*& get() noexcept { return _storage; }
    T* const& get() const noexcept { return _storage; }
    T*& get(std::size_t) noexcept { return _storage; }
    T* const& get(std::size_t) const noexcept { return _storage; }

    T* operator->() const noexcept { return _storage; }
    T& operator*() const noexcept { return *_storage; }
    operator T*() const noexcept { return _storage; }

    context_local_ptr& operator=(T* val) noexcept {
        _storage = val;
        return *this;
    }

    bool operator==(std::nullptr_t) const noexcept { return _storage == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return _storage != nullptr; }
    explicit operator bool() const noexcept { return _storage != nullptr; }
};

#endif // SEASTAR_DST

} // namespace dst
} // namespace seastar
