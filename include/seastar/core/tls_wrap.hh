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
 * Copyright 2026 ScyllaDB
 */

#pragma once

#include <concepts>
#include <mutex>
#include <type_traits>
#include <vector>
#include <array>

namespace seastar {

// Probably a better way to handle this than the tls_reg_ok flag below
//
//==383417==ERROR: AddressSanitizer: heap-use-after-free on address 0x511000022ad0 at pc 0x584001603b42 bp 0x7ffefe5fc800 sp 0x7ffefe5fc7f0
//WRITE of size 8 at 0x511000022ad0 thread T0
//    #0 0x584001603b41 in decltype (::new ((void*)(0)) void*((declval<void* const&>)())) std::construct_at<void*, void* const&>(void**, void* const&) /usr/include/c++/13/bits/stl_construct.h:97
//    #1 0x5840015db11e in void std::allocator_traits<std::allocator<void*> >::construct<void*, void* const&>(std::allocator<void*>&, void**, void* const&) /usr/include/c++/13/bits/alloc_traits.h:540
//    #2 0x5840015db11e in std::vector<void*, std::allocator<void*> >::push_back(void* const&) /usr/include/c++/13/bits/stl_vector.h:1286
//    #3 0x5840015caafb in seastar::tls_registry::add(void*) /home/nwatkins/src/seastar/include/seastar/core/tls_wrap.hh:36
//    #4 0x5840015de501 in seastar::tls_wrap<std::chrono::time_point<seastar::lowres_clock, std::chrono::duration<long, std::ratio<1l, 1000000000l> > > >::registrar::registrar(seastar::tls_wrap<std::chrono::time_point<seastar::lowres_clock, std::chrono::duration<long, std::ratio<1l, 1000000000l> > > >*) /home/nwatkins/src/seasta
//r/include/seastar/core/tls_wrap.hh:52
//    #5 0x5840015d3038 in seastar::tls_wrap<std::chrono::time_point<seastar::lowres_clock, std::chrono::duration<long, std::ratio<1l, 1000000000l> > > >::tls_wrap() /home/nwatkins/src/seastar/include/seastar/core/tls_wrap.hh:59
//    #6 0x584001bf60a8 in __tls_init /home/nwatkins/src/seastar/include/seastar/core/lowres_clock.hh:65
//    #7 0x58400174bc6f in TLS wrapper function for seastar::local_engine (/home/nwatkins/src/seastar/build.deb/tests/unit/abort_source_test+0x57d5c6f) (BuildId: 3e2256255ad43dbb1481bec43b451afce6bd2930)
//    #8 0x58400182f113 in dl_iterate_phdr /home/nwatkins/src/seastar/src/core/exception_hacks.cc:147
//    #9 0x7ed578f3ca02 in __lsan::LockStuffAndStopTheWorld(void (*)(__sanitizer::SuspendedThreadsList const&, void*), __lsan::CheckForLeaksParam*) ../../../../src/libsanitizer/lsan/lsan_common_linux.cpp:142
//    #10 0x7ed578f398e6 in CheckForLeaks ../../../../src/libsanitizer/lsan/lsan_common.cpp:778
//    #11 0x7ed578f3a06f in CheckForLeaks ../../../../src/libsanitizer/lsan/lsan_common.cpp:765
//    #12 0x7ed578f3a06f in __lsan::DoLeakCheck() ../../../../src/libsanitizer/lsan/lsan_common.cpp:821
//    #13 0x7ed577047381 in __cxa_finalize stdlib/cxa_finalize.c:82
//    #14 0x7ed578e3b7b6  (/lib/x86_64-linux-gnu/libasan.so.8+0x3b7b6) (BuildId: 0241d5a774aeb1d6babd9f68d743bdcf31b4a97d)
//
//0x511000022ad0 is located 208 bytes inside of 256-byte region [0x511000022a00,0x511000022b00)
//freed by thread T0 here:
//    #0 0x7ed578eff5e8 in operator delete(void*, unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:164
//    #1 0x584001622dc4 in std::__new_allocator<void*>::deallocate(void**, unsigned long) /usr/include/c++/13/bits/new_allocator.h:172
//    #2 0x5840015e6418 in std::allocator<void*>::deallocate(void**, unsigned long) /usr/include/c++/13/bits/allocator.h:210
//    #3 0x5840015e6418 in std::allocator_traits<std::allocator<void*> >::deallocate(std::allocator<void*>&, void**, unsigned long) /usr/include/c++/13/bits/alloc_traits.h:517
//    #4 0x5840015e6418 in std::_Vector_base<void*, std::allocator<void*> >::_M_deallocate(void**, unsigned long) /usr/include/c++/13/bits/stl_vector.h:390
//    #5 0x5840015db40e in std::_Vector_base<void*, std::allocator<void*> >::~_Vector_base() /usr/include/c++/13/bits/stl_vector.h:369
//    #6 0x5840015db639 in std::vector<void*, std::allocator<void*> >::~vector() /usr/include/c++/13/bits/stl_vector.h:738
//    #7 0x58400163d61d in seastar::tls_registry::~tls_registry() /home/nwatkins/src/seastar/include/seastar/core/tls_wrap.hh:31
//    #8 0x7ed577047a75 in __run_exit_handlers stdlib/exit.c:108
//    #9 0x7ed577047bbd in __GI_exit stdlib/exit.c:138
//    #10 0x7ed57702a1d0 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:74
//    #11 0x7ed57702a28a in __libc_start_main_impl ../csu/libc-start.c:360
//    #12 0x584001593854 in _start (/home/nwatkins/src/seastar/build.deb/tests/unit/abort_source_test+0x561d854) (BuildId: 3e2256255ad43dbb1481bec43b451afce6bd2930)
inline bool tls_reg_ok{true};

struct tls_registry {
    size_t idx = 0;
    std::array<void*, 100> wrappers{};
    void add(void* p) {
        wrappers[idx++] = p;
    }
    ~tls_registry() {
        tls_reg_ok = false;
    }
};

inline constinit tls_registry tls_reg;

template<typename T>
class tls_wrap;

template<typename T>
requires std::is_class_v<T>
class tls_wrap<T> : public T {
private:
    struct registrar {
        registrar(tls_wrap* self) {
            if (tls_reg_ok) {
                tls_reg.add(static_cast<T*>(self));
            }
        }
    };

    [[no_unique_address]] registrar _registrar{this};

public:
    using T::T;

    // Converting constructor from T itself, since inherited constructors
    // cannot be used for initialization from the base type.
    tls_wrap(T&& val) : T(std::move(val)) {}
    tls_wrap(const T& val) : T(val) {}

    template<typename U>
    requires requires(T& base, U&& val) { base = std::forward<U>(val); }
    tls_wrap& operator=(U&& val) noexcept(noexcept(std::declval<T&>() = std::forward<U>(val))) {
        T::operator=(std::forward<U>(val));
        return *this;
    }
};

template<typename T>
requires (!std::is_class_v<T>)
class tls_wrap<T> {
    T _value;

    struct registrar {
        registrar(T* val_ptr) {
            if (tls_reg_ok) {
                tls_reg.add(val_ptr);
            }
        }
    };

    [[no_unique_address]] registrar _registrar{&_value};

public:
    tls_wrap() : _value{} {}

    template<std::convertible_to<T> U>
    tls_wrap(U&& val) : _value(std::forward<U>(val)) {}

    template<std::convertible_to<T> U>
    tls_wrap& operator=(U&& val) {
        _value = std::forward<U>(val);
        return *this;
    }

    operator T&() { return _value; }
    operator const T&() const { return _value; }

    // For pointer types, provide operator-> so that tls_wrap<T*> can be
    // used transparently where T* is expected with -> syntax.
    auto operator->() const requires std::is_pointer_v<T> { return _value; }
    auto operator->() requires std::is_pointer_v<T> { return _value; }
};

} // namespace seastar
