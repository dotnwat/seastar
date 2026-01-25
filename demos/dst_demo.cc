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
#include <zlib.h>
#include <random>
#include <seastar/core/gate.hh>
#include <seastar/core/sharded.hh>
#include <seastar/core/app-template.hh>
#include <seastar/core/thread.hh>
#include <seastar/util/assert.hh>
#include <seastar/util/closeable.hh>
#include <seastar/core/coroutine.hh>
#include <seastar/util/later.hh>

// Roughly this program does the following:
//
// Each core generates a bunch of background fibers that send a random value to
// a random core. Some added sleep and yields help introduce non-determisim in
// this process.
//
// Each core records the order in which the messages arrive.
//
// At the end of the experiment we print out the order of messages received by
// each core.
//
// For convenience a checksum is computed across the entire sequence for easily
// detecting if two runs produced the same sequence.
//
// When run with --dst false the output is usually unlike any previous run, and
// with --dst true the output expected to always be the same.
class service : public seastar::peering_sharded_service<service> {
public:
    seastar::future<> scatter() {
        for (int i = 0; i < 30; ++i) {
            auto randval = gen();
            (void)seastar::with_gate(gate, [this, randval] {
                auto choice = randval % seastar::smp::count;
                return container().invoke_on(choice, [randval] (auto& s) {
                    s.msgs.push_back(randval);
                });
            });
            usleep(randval % 1000);
            if (randval % 4) {
                co_await seastar::yield();
            }
        }
    }

    seastar::future<> wait() {
        return gate.close();
    }

    unsigned long print() {
        auto crc = crc32(0, NULL, 0);
        for (auto m : msgs) {
            crc = crc32(crc, reinterpret_cast<unsigned char*>(&m), sizeof(m));
        }
        std::cerr << fmt::format("  {:02}: hash={:05} seq -> {:05}", seastar::this_shard_id(),
                    (uint16_t)crc, fmt::join(msgs, ",")) << std::endl;
        return crc;
    }

    std::mt19937 gen{seastar::this_shard_id()};
    std::vector<uint16_t> msgs;
    seastar::gate gate;
};

int main(int ac, char** av) {
    seastar::app_template app;
    return app.run(ac, av, [&] {
        return seastar::async([&] {
            seastar::sharded<service> s;
            s.start().get();
            s.invoke_on_all([] (auto& s) { return s.scatter(); }).get();
            s.invoke_on_all([] (auto& s) { return s.wait(); }).get();

            auto crc = crc32(0, NULL, 0);
            std::cerr << "CORE" << std::endl;
            for (unsigned i = 0; i < seastar::smp::count; ++i) {
                unsigned long shard_crc;
                s.invoke_on(i, [&shard_crc] (auto& s) { shard_crc = s.print(); }).get();
                crc = crc32(crc, reinterpret_cast<unsigned char*>(&shard_crc), sizeof(shard_crc));
            }
            std::cerr << fmt::format("FINAL HASH={:05}", (uint16_t)crc) << std::endl;
            s.stop().get();
        });
    });
}
