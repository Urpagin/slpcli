//
// Created by urpagin on 2025-08-05.
//

#include "Dispatcher.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <thread>
#include <utility>

using namespace asio::experimental::awaitable_operators;

Dispatcher::Dispatcher(Callback callback, const SlpOptions &opts) :
    ioc_(asio::io_context{}), ex_guard_(ioc_.get_executor()), cb_(std::move(callback)),
    semaphore_(ioc_.get_executor(), static_cast<int>(opts.semaphore_count)),
    worker_count_(get_worker_count(opts.worker_thread_count)) {

    for (size_t _{0}; _ < worker_count_; ++_) {
        workers.emplace_back([this]() { ioc_.run(); });
    }

    // TODO: callback thread pool, executor and shiii
}


size_t Dispatcher::get_worker_count(const size_t def) {
    if (def > 0)
        return def;
    return DEFAULT_WORKER_COUNT;
}
asio::awaitable<void> Dispatcher::query_one(ServerQuery q) {

    // Set up the timer. Cancels the WHOLE SLP operation if it wins against it.
    asio::steady_timer timer(ioc_.get_executor());
    timer.expires_after(q.timeout);

    // Initialize a Connection object to later query the MC server.
    Connection conn{ioc_.get_executor(), std::move(q)};

    // Fire the timer and if it finishes, fire the cancellation signal.

    // Very sexy. The first coroutine that finishes cancels the other one.
    // Variant of either our Outcome or whatever it is async_wait returns.
    // auto v = co_await (conn.query() || timer.async_wait(asio::use_awaitable));
    auto v = co_await (conn.query() || timer.async_wait(asio::use_awaitable));


    // Post a task to be executed eagerly by another executor (not the main one)
    // Using the cb_ex to execute the callback function calls, so that we can continue
    // to query servers as fast as possible.
    asio::post(cb_pool.get_executor(), [v = std::move(v), cb = cb_]() {
        try {
            if (v.index() == 0) {
                cb(std::move(std::get<0>(v)));
            } else {
                err("Query took longer than timeout; cancelled.");
                cb(Outcome{std::unexpect});
            }
        } catch (const std::exception &e) {
            err("Exception while querying: ", e.what());
        }
    });


    co_return;
}

static size_t submitted_count = 0;
/// @brief Submits a task for execution only if the Dispatch is NOT sealed.
/// @returns true for OK, false for sealed (or otherwise?)
bool Dispatcher::submit(ServerQuery query) {
    ++submitted_count;
    if (is_sealed.load(std::memory_order_acquire))
        return false;


    // Spawn the pretty, magnificent, incredible and oiled up coroutine;
    // The executor from ioc_ will handle the rest.


    // Spawn the query coroutine while making sure to use RAII on the semaphore, calling it async.
    asio::co_spawn(
            ioc_.get_executor(),
            [this, q = std::move(query)]() -> asio::awaitable<void> {
                co_await semaphore_.async_acquire(asio::use_awaitable); // non-blocking
                struct Permit {
                    sam::semaphore &s;
                    // "... RELEEEEASE!", a chubby blue-shirted individual once said.
                    ~Permit() { s.release(); }
                } p{semaphore_};

                co_await query_one(q);
                co_return;
            },
            asio::detached);

    // TODO: constant return val.
    // edit 1: What did I mean by that?
    // Edit 2: Exactly, what did I mean by that?
    return true;
}


/// @brief Effectively makes submit() a no-op; closing the sink and waits for all workers to finish.
void Dispatcher::seal_and_wait() {
    info("Sealing and waiting...");
    // We cannot safely call this function more than once, since the workers are already being joined.
    if (is_sealed.load())
        return;

    // Do not keep the io_context alive if all coroutines are done.
    ex_guard_.reset();

    this->is_sealed.store(true);
    for (auto &w: this->workers) {
        w.join();
    }

    // Explicitly wait for all the callbacks to be called.
    cb_pool.join();
    info("Wait done; all queries done; all callbacks done.");
}
