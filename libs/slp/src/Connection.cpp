//
// Created by urpagin on 8/5/25.2025-08-05.
//

#include "Connection.h"

#include "DataTypesUtils.h"

#include <iostream>


Connection::Connection(asio::io_context &io_context, ServerQuery server, const std::function<void(Outcome)> &callback) :
    server_query_{std::move(server)}, socket_(io_context), deadline_(io_context), callback_(callback) {}


/// @brief Read the socket for the Status Response packet and returns the bytes
/// of its JSON string.
/// @returns The bytes of the JSON.
asio::awaitable<std::string> Connection::read_json_status_response_packet() {
    using dtu = DataTypesUtils;

    int json_size{0};

    try {
        // Read the packet size (unnecessary, we don't keep it)
        co_await dtu::read_varint(socket_);
        // Read the packet ID
        if (const int packet_id = co_await dtu::read_varint(socket_); packet_id != 0x00) {
            throw std::runtime_error("Error: Status Response packet ID is not 0x00");
        }
        // Read the size in bytes of the JSON from sock.
        json_size = co_await dtu::read_varint(socket_);
    } catch (const asio::system_error &e) {
        if (e.code() == asio::error::eof) {
            std::cerr << "Error: failed to read Status Response packet JSON size: EOF" << std::endl;
        } else {
            std::cerr << "Error: failed to read Status Response packet JSON size: " << e.what() << std::endl;
        }
        throw std::runtime_error("Failed to read the Status Response packet JSON size");
    }

    if (json_size <= 0) {
        throw std::runtime_error("Failed to read the Status Response packet JSON "
                                 "size: JSON size is 0 or less");
    }

    std::string buffer(json_size, '\0');

    try {
        // Read the JSON string into memory.
        asio::read(socket_, asio::buffer(buffer.data(), json_size));
    } catch (const asio::system_error &e) {
        if (e.code() == asio::error::eof) {
            std::cerr << "Error: failed to read Status Response packet JSON: EOF" << std::endl;
        } else {
            std::cerr << "Error: failed to read Status Response packet JSON size: " << e.what() << std::endl;
        }
        throw std::runtime_error("Failed to read the Status Response packet JSON size");
    }
    co_return buffer;
}

/// @brief Connects to the Minecraft server, making the socket usable.
asio::awaitable<void> Connection::connect() {
    using asio::ip::tcp;

    // Run this coroutine on the same executor as the socket
    tcp::resolver resolver(co_await asio::this_coro::executor);

    auto [ec1, endpoints] = co_await resolver.async_resolve(
            server_query_.server.address, std::to_string(server_query_.server.port), asio::as_tuple(asio::use_awaitable));
    if (ec1) {
        std::cerr << "ERROR: failed to resolve the address: " << server_query_.server.address << ":" << server_query_.server.port
                  << std::endl;
        co_return;
    }

    // connect with a completion token
    co_await asio::async_connect(socket_, endpoints, asio::use_awaitable);

    co_return;
}


/// Function inspired from the docs.
void Connection::check_deadline() {
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expiry() <= asio::steady_timer::clock_type::now()) {
        socket_.close();

        // There is no longer an active deadline. The expiry is set to the
        // maximum time point so that the actor takes no action until a new
        // deadline is set.
        deadline_.expires_at(asio::steady_timer::time_point::max());
    }

    // Put the actor back to sleep.
    deadline_.async_wait([this](const asio::error_code &) { check_deadline(); });
}


/// @brief Queries the Minecraft server using the Status List Ping protocol and
/// calls the `callback_` function passing in a `std::expected<Result, ResultErr>`,
asio::awaitable<void> Connection::query_slp() {
    using dtu = DataTypesUtils;

    // 'Build Handshake' and 'Status Request' packets.
    auto handshake_packet =
            dtu::make_handshake_packet(server_query_.server.address, server_query_.server.port, server_query_.protocol_version);
    constexpr auto status_request_packet = dtu::make_status_request_packet();

    try {
        if (!socket_.is_open()) {
            std::cerr << "WARNING: socket is closed, cannot query." << std::endl;
            callback_(Outcome{std::unexpect, server_query_.server, "Socket closed."});
            co_return;
        }
        // Send server-bound packets:
        co_await asio::async_write(socket_, asio::buffer(handshake_packet), asio::use_awaitable);
        co_await asio::async_write(socket_, asio::buffer(status_request_packet), asio::use_awaitable);

        // Receive client-bound packet JSON string.
        // Send the JSON to the callback.
        callback_(Result{server_query_.server, co_await read_json_status_response_packet()});

    } catch (const asio::system_error &e) {
        std::cerr << "Error: connection with the MC server failed: " << e.what() << std::endl;
        throw;
    }
}

asio::awaitable<void> Connection::run() {
    std::cout << "Called Connection::run()" << std::endl;

    // Start the deadline actor. We are setting a global timeout for ALL async operations;
    // not prior to each async operations.
    deadline_.expires_after(std::chrono::seconds(server_query_.timeout_s));
    deadline_.async_wait([this](const asio::error_code &) { check_deadline(); });

    std::cout << "Connecting to MC server..." << std::endl;
    co_await connect();
    std::cout << "OK" << std::endl;


    std::cout << "Querying SLP..." << std::endl;
    co_await query_slp();
    std::cout << "OK" << std::endl;
}
