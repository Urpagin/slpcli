//
// Created by urpagin on 8/5/25.2025-08-05.
//

#include "Connection.h"

#include "DataTypesUtils.h"

#include <iostream>

using namespace std::string_literals;

Connection::Connection(const asio::any_io_executor &ex, ServerQuery server) : server_query_{std::move(server)}, socket_(ex) {}


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
            err("Failed to read Status Response packet JSON size: EOF");
        } else {
            err("Failed to read Status Response packet JSON size: ", e.what());
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
        if (auto [ec, n] = co_await asio::async_read(socket_, asio::buffer(buffer.data(), json_size),
                                                     asio::as_tuple(asio::use_awaitable));
            ec) {
            throw asio::system_error(ec);
        }

    } catch (const asio::system_error &e) {
        if (e.code() == asio::error::eof) {
            err("Failed to read Status Response packet JSON: EOF");
        } else {
            err("Failed to read Status Response packet JSON size: ", e.what());
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
        err("Failed to resolve the address: ", server_query_.server.address, ':', server_query_.server.port);
        co_return;
    }

    // connect with a completion token
    co_await asio::async_connect(socket_, endpoints, asio::use_awaitable);
    socket_.set_option(tcp::no_delay(true));

    co_return;
}


/// @brief Queries the Minecraft server using the Status List Ping protocol and
/// calls the `callback_` function passing in a `std::expected<Result, ResultErr>`,
asio::awaitable<Outcome> Connection::query_slp() {
    using dtu = DataTypesUtils;

    // 'Build Handshake' and 'Status Request' packets.
    auto handshake_packet =
            dtu::make_handshake_packet(server_query_.server.address, server_query_.server.port, server_query_.protocol_version);
    constexpr auto status_request_packet = dtu::make_status_request_packet();

    try {
        if (!socket_.is_open()) {
            err("Socket is closed, cannot query");
            co_return Outcome{std::unexpect, server_query_.server, "Socket closed."};
        }
        // Send server-bound packets:
        co_await asio::async_write(socket_, asio::buffer(handshake_packet), asio::use_awaitable);
        co_await asio::async_write(socket_, asio::buffer(status_request_packet), asio::use_awaitable);

        // Receive client-bound packet SLP JSON string response, and return it.
        co_return Result{server_query_.server, co_await read_json_status_response_packet()};

    } catch (const asio::system_error &e) {
        std::string err_msg = "connection with the MC server failed: "s + e.what();
        co_return Outcome{std::unexpect, server_query_.server, err_msg};
    }
}


asio::awaitable<Outcome> Connection::query() {
    info("Connecting to the Minecraft server...");
    co_await connect();
    info("Connected successfully!");


    info("Querying SLP...");
    Outcome res = co_await query_slp();
    co_return res;
}
