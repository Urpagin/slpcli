//
// Created by urpagin on 2025-08-05.

#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <asio/io_context.hpp>
#include <asio/thread_pool.hpp>
#include <boost/sam/semaphore.hpp>
#include <thread>

#include "Connection.h"
#include "slp.h"


// TODO: Race I/O instead of `steady_timer`. The winner cancels the loser. Simpler.

// TODO: enforce an inflight cap. (isn't this the semaphore already?)

class Dispatcher {
    /// I/O Interface with the kernel.
    asio::io_context ioc_;

    /// Keeps the io_context_ alive even when no tasks are done (I think)
    asio::executor_work_guard<asio::io_context::executor_type> ex_guard_;

    /// Vector of threads, normally about the same number of CPU threads.
    /// Each one will call io_context_.run().
    std::vector<std::jthread> workers;

    /// The number of threads the callback function will be called on.
    constexpr static size_t DEFAULT_CALLBACK_EX_THREADS{1};

    /// Callback that runs when a server responds.
    Callback cb_;

    /// Thread pool that will call the callback on other threads so that the workers
    /// aren't waiting for the callback to finish.
    asio::thread_pool cb_pool{DEFAULT_CALLBACK_EX_THREADS};

    /// Limits the number of tasks at one time.
    sam::semaphore semaphore_;

    // TODO: DO I NEED THIS?
    /// Number of tasks? currently running.
    std::atomic<size_t> in_flight_{0};

    /// Whether the seal() method has been called. If so, the submit() method becomes a no-op.
    std::atomic<bool> is_sealed{false};

    /// Default number of threads to run if no specified AND error getting the CPU thread-count.
    constexpr static size_t DEFAULT_WORKER_COUNT{2};

    /// The number of worker threads to pull up.
    size_t worker_count_;


    /// @brief Returns the number of worker threads to pull up.
    /// Optionally pass in a thread count.
    ///
    /// Behavior:
    ///     - If a default is passed, it will be returned;
    ///     - If no default passed, it will try to get the CPU thread-count;
    ///     - If error getting the CPU thread-count, use DEFAULT_WORKER_COUNT.
    static size_t get_worker_count(size_t def);


    /// Coroutine; queries a Minecraft server.
    asio::awaitable<void> query_one(ServerQuery);


public:
    /// @brief Public constructor
    ///
    /// The callback function.
    /// The I/O executor.
    Dispatcher(Callback callback, const SlpOptions &opts);


    /// @brief Destructor - Calls seal() and finish().
    ~Dispatcher() {
        seal();
        finish();
    }


    /// @brief Submits a single server into the sink.
    bool submit(ServerQuery);

    /// @brief Seals the sink, not allowing any more servers to be queued up.
    void seal() const;

    /// @brief BLOCKING - waits for all threads & coroutines to rejoin & return.
    void finish() const;


    // TODO: submit_range(R&& range) that just loops and calls submit per element.
    // TODO: submit_batch(std::span<const ServerQuery>)

    // TODO: submit() method that takes one server and throws it into the endless pit of the sink
    // TODO: overloads with standard streams and all, with support for files!!!!!!!!!
    // NOTE: I love this
    // TODO: support NDJSON (newline delimited JSON)
    // TODO: Usages like so:
    /*
     *  cat servers.txt | slp --ok ip > online.txt
     *
     *  cat servers.txt | slp --format ndjson > results.jsonl
        jq -r 'select(.ok) | .server' results.jsonl > online.txt
     */

    // TODO
    /*
    *Input hygiene (so “cat … | slp” just works)

    Accept host, host:port, and [ipv6]:port.

    Trim whitespace; ignore empty lines and lines starting with #.

    Default port if missing (e.g., 25565).
     */
};
#endif // DISPATCHER_H
