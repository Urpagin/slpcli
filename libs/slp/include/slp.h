//
// Created by urpagin on 8/10/25.
//

#ifndef SLPCLI_SLP_H
#define SLPCLI_SLP_H
#include <chrono>
#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <thread>


#include "logger.hpp"

using logger::err;
using logger::info;
using logger::log_end;
using logger::log_start;

/// @brief Represents a Minecraft server.
struct McServer {
    /// Address of the Minecraft server.
    std::string address;
    /// Port of the Minecraft server.
    uint16_t port;
};

/// @brief Result of a Minecraft server's SLP protocol query and some metadata.
struct Result {
    /// The MC server.
    McServer server;
    /// SLP response of the Minecraft server.
    std::string json;
};

/// @brief Result of a failed query to a Minecraft server.
struct ResultErr {
    /// The MC server;
    McServer server;
    /// Error message.
    std::string message;
};

/// @brief The date that is required to query a Minecraft server.
struct ServerQuery {
    /// The server to query.
    McServer server;

    /// Controls the total time the whole query takes at max. DNS query, connection, reads & writes.
    /// If the query exceeds the timeout, it is dropped.
    std::chrono::milliseconds timeout{10000LL}; // 10s default

    /// The protocol version included in the Handshake packet.
    /// Default to `-1`. Should work, if not, set something else.
    int protocol_version{-1};
};

/// Response type when querying a server.
using Outcome = std::expected<Result, ResultErr>;
/// Callback function type that's called at each server query.
using Callback = std::function<void(Outcome)>;

/// Options for the dispatcher.
struct SlpOptions {
    /// Controls the number of asynchronous tasks at one time.
    size_t semaphore_count{1024};
    /// Controls the number of threads running, each querying servers.
    size_t worker_thread_count{std::thread::hardware_concurrency()};
    /// Controls the number of threads running, each calling the callback function with the MC servers responses.
    /// In other words, the number of threads that call the call-site-provided callback function (I think).
    size_t callback_worker_threads{1};
};

/// @brief Query the SLP protocol on Minecraft: Java Edition servers.
///
/// Following the PIMPL pattern.
class slp {
public:
    explicit slp(SlpOptions, Callback);
    ~slp();

    // Copy & Move constructors. (Mostly black magic for now)
    slp(const slp &) = delete;
    slp &operator=(const slp &) = delete;
    slp(slp &&) noexcept = default;
    slp &operator=(slp &&) noexcept = default;

    void submit(ServerQuery q) const;
    void seal_and_wait() const;

    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // TODO: Add sugar:
    // Reads servers from an istream, streams results to a Callback, then returns.
    // void run_streaming(std::istream& in, const Client::Options&, Callback cb);
    // + add range && std::span support.

private:
    // We follow the PIMPL pattern.
    // So-called "forward-declaration"
    // The Impl is defined only in the source file.
    struct Impl;
    std::unique_ptr<Impl> p_; // pointer to the real data
};


#endif // SLPCLI_SLP_H
