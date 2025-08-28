//
// Created by urpagin on 2025-08-05.
//

#ifndef CONNECTION_H
#define CONNECTION_H

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <expected>

#include "slp.h"


/// @brief Object that manager a singular asynchronous connection to
/// a Minecraft server.
class Connection : std::enable_shared_from_this<Connection> {
public:
    /// Constructor
    explicit Connection(const asio::any_io_executor &ex, ServerQuery server);

    /// Query the Minecraft server.
    asio::awaitable<Outcome> query();

private:
    /// Information about the Minecraft server.
    ServerQuery server_query_;
    /// Socket to the Minecraft server.
    asio::ip::tcp::socket socket_;
    /// Callback that runs when a server responds.
    std::function<void(Outcome)> callback_;

    /// Opens the socket, connects it to the Minecraft server.
    asio::awaitable<void> connect();

    /// Reads the JSON status response packet from `socket_`.
    asio::awaitable<std::string> read_json_status_response_packet();

    /// Queries the MC server's SLP.
    asio::awaitable<Outcome> query_slp();
};

#endif // CONNECTION_H
