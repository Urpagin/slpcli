//
// Created by urpagin on 2025-08-05.
//

#include "Dispatcher.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <iostream>
#include <thread>
#include <utility>


Dispatcher::Dispatcher(Callback callback, SlpOptions opts) :
    ioc_(asio::io_context{}), ex_guard_(ioc_.get_executor()), cb_(std::move(callback)),
    semaphore_(ioc_.get_executor(), DEFAULT_SEM_LIMIT), worker_count_(get_worker_count(worker_thread_count)) {

    for (size_t _{0}; _ < worker_count_; ++_) {
        workers.emplace_back([this]() { ioc_.run(); });
    }

    // TODO: callback thread pool, executor and shiii
}


size_t Dispatcher::get_worker_count(const std::optional<size_t> def) {
    if (def)
        return *def;

    if (const int sys_c = std::thread::hardware_concurrency() > 0)
        return static_cast<size_t>(sys_c);

    return DEFAULT_WORKER_COUNT;
}
asio::awaitable<void> Dispatcher::query_one(ServerQuery q) {
    // TODO: right place to add a timer, and race it with I/O.

    // Initialize a Connection object to later query the MC server.
    Connection conn{ioc_.get_executor(), std::move(q)};

    // Actually query the Minecraft server, await the response.
    Outcome res = co_await conn.query();

    // Post a task to be executed eagerly by another executor (not the main one)
    // Using the cb_ex to execute the callback function calls, so that we can continue
    // to query servers as fast as possible.
    asio::post(cb_pool.get_executor(), [r = std::move(res), cb = cb_, count = &in_flight_]() {
        try {
            cb(std::move(r));
        } catch (const std::exception &e) {
            std::cerr << "Error: exception while querying: " << e.what() << std::endl;
        }
        --(*count);
    });

    co_return;
}

bool Dispatcher::submit(ServerQuery query) {
    // Acquire a semaphore permission.
    semaphore_.acquire();

    // Increment in_flight_, we're in the process of querying a server.
    ++in_flight_;

    // Spawn the pretty, magnificent, incredible and oiled up coroutine;
    // The executor from ioc_ will handle the rest.
    asio::co_spawn(ioc_.get_executor(), query_one(std::move(query)), asio::detached);

    // TODO: constant return val.
    return true;
}

void Dispatcher::seal() const { std::cout << "seal()\n"; }

void Dispatcher::finish() const { std::cout << "finish()\n"; }
