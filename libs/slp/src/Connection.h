//
// Created by urpagin on 2025-08-05.
//

#ifndef CONNECTION_H
#define CONNECTION_H

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <expected>

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

/// @brief The info that are required to query a Minecraft server.
struct ServerQuery {
    /// The server to query.
    McServer server;

    /// The protocol version included in the Handshake packet.
    /// `-1` should work, if not, set something else.
    int protocol_version{-1};

    /// The timeout in seconds for the duration of the whole operation
    /// from connect to reads & writes.
    size_t timeout_s;
};


using Outcome = std::expected<Result, ResultErr>;

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
    /// Timeout the connection if too long.
    asio::steady_timer deadline_;
    /// Callback that runs when a server responds.
    std::function<void(Outcome)> callback_;

    /// Opens the socket, connects it to the Minecraft server.
    asio::awaitable<void> connect();

    /// Reads the JSON status response packet from `socket_`.
    asio::awaitable<std::string> read_json_status_response_packet();

    /// Checks if the deadline (timeout) is expired. If so, close socket.
    void check_deadline();

    /// Queries the MC server's SLP.
    asio::awaitable<Outcome> query_slp();
};

#endif // CONNECTION_H
