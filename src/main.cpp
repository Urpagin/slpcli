#include <CLI/CLI.hpp>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <iostream>
#include <netdb.h>
#include <string>
#include <utility>

#include <string_view>
#include "slp.h"
#include "slpcliConfig.h"

#ifdef _WIN32
#include <io.h> // _dup, _dup2, _close
#define dup _dup
#define dup2 _dup2
#define close _close
constexpr const char *DEV_NULL = "NUL";
#else
#include <unistd.h> // dup, dup2, close
constexpr auto DEV_NULL = "/dev/null";
#endif
// #include "logger"


#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
using namespace std::chrono_literals;


// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp
// TODO: USE slp/src/logger.hpp

/// 100% ChatGPT.
void quick_and_dirty_run() {
    using namespace std::chrono_literals;

    auto f = std::make_shared<std::ofstream>("ok.txt", std::ios::app);
    slp serv{SlpOptions{.semaphore_count = 100, .worker_thread_count = 100}, [f](const Outcome &out) {
                 if (out.has_value()) {
                     std::cout << out->json << '\n';
                     *f << out->json << "\n";
                 }
             }};


    std::ifstream in("/home/urpagin/Documents/PROGRAMMING/other/McpingerCpp/src/servers2.txt");
    if (!in) {
        std::cerr << "failed to open service.txt\n";
        return;
    }

    // Read everything into RAM (quick & dirty).
    std::vector<std::pair<std::string, std::uint16_t>> ips;
    ips.reserve(8192);

    auto trim = [](std::string &s) {
        auto not_space = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    };

    std::string line;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::string addr;
        std::uint16_t port = 25565; // default MC port

        if (!line.empty() && line.front() == '[') {
            // Bracketed IPv6: [addr]:port (port optional)
            auto rb = line.find(']');
            if (rb != std::string::npos) {
                addr = line.substr(1, rb - 1);
                if (rb + 1 < line.size() && line[rb + 1] == ':') {
                    try {
                        int p = std::stoi(line.substr(rb + 2));
                        if (p < 0)
                            p = 0;
                        if (p > 65535)
                            p = 65535;
                        port = static_cast<std::uint16_t>(p);
                    } catch (...) {
                    }
                }
            } else {
                // Malformed; take raw, keep default port
                addr = line;
            }
        } else {
            // host[:port]
            if (auto pos = line.rfind(':'); pos != std::string::npos) {
                addr = line.substr(0, pos);
                try {
                    int p = std::stoi(line.substr(pos + 1));
                    if (p < 0)
                        p = 0;
                    if (p > 65535)
                        p = 65535;
                    port = static_cast<std::uint16_t>(p);
                } catch (...) {
                    // keep default port
                }
            } else {
                addr = std::move(line);
            }
        }

        if (!addr.empty())
            ips.emplace_back(std::move(addr), port);
    }

    const auto timeout = 3s; // tweak if you like

    for (auto &[addr, port]: ips) {
        serv.submit({
                .server = {addr, port}, .timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout),
                // .protocol_version = -1 (default)
        });
    }

    printf("ABCD\n");
    serv.seal_and_wait();
}

void ping_one() {
    // Construct the client with defaults; print JSON for successes only.
    const slp serv{SlpOptions{}, [](const Outcome &out) {
                       if (out.has_value()) {
                           std::cout << out->json << std::endl;
                       }
                   }};

    serv.submit(ServerQuery{.server = McServer{"blog.urpagin.net", 25565}, .timeout = 3s});
    serv.seal_and_wait();
}


namespace detail {
    inline int saved_out = -1;
    inline int saved_err = -1;
} // namespace detail

void disable_output();
void enable_output();
std::pair<std::string, uint16_t> read_server_address(int, char *[]);
std::string get_version();

struct AppOptions {
    bool is_quiet{false};
    std::string addr;
    uint16_t port{25565};
    int timeout{5};
    int protocol_version_handshake{-1};
};
AppOptions parse_args(int, char **);

int main(const int argc, char *argv[]) {
    // This syntax is so cool
    const auto [is_quiet, addr, port, timeout, protocol_version] = parse_args(argc, argv);

    if (is_quiet)
        disable_output();

    std::cout << "Querying '" << addr << ":" << port << "'...\n\n" << std::endl;

    try {

        quick_and_dirty_run();
        // ping_one();
        std::cout << "hi" << std::endl;

        // Print JSON
    } catch (const std::exception &e) {
        std::cerr << "Error: failed to query SLP: " << e.what() << std::endl;
        std::exit(-1);
    }

    return 0;
}

/// @brief Split "<addr>:<port>" into {addr, optional<port>}.
/// If the colon is missing, second element is std::nullopt.
std::pair<std::string_view, std::optional<uint16_t>> split_addr(std::string_view input) {
    const std::size_t pos = input.find(':');
    if (pos == std::string_view::npos)
        return {input, std::nullopt};

    std::string_view host = input.substr(0, pos);
    const std::string_view port_sv = input.substr(pos + 1);

    if (port_sv.empty()) // “host:”
        return {host, std::nullopt};

    if (const int p_int = std::stoi(std::string(port_sv)); p_int < 0 || p_int > 1 << 16) {
        throw std::runtime_error("Port invalid");
    } else {
        return {host, static_cast<uint16_t>(p_int)};
    }
}

/// @brief Parses the CLI arguments.
AppOptions parse_args(const int argc, char **argv) {
    CLI::App app{std::format("Minecraft: Java Edition SLP CLI (version: {})", get_version())};

    AppOptions opts;

    app.add_flag("-s,--silent", opts.is_quiet, "Only prints the JSON or an empty string if error.");
    app.add_option("-a,--address,addr", opts.addr, "Server address with optional \":port\".")->required();
    const auto port_opt = app.add_option("-p,--port,port", opts.port, "Port of the Minecraft server (default 25565).");

    app.add_option("-t,--timeout", opts.timeout, "The timeout in seconds at which the query is dropped");

    app.add_option("--protocol-version", opts.protocol_version_handshake,
                   "The protocol version that the client plans on using to connect "
                   "to the server. Don't change if you don't know what it means.");

    try {
        app.parse(argc, argv); // may std::exit on help/usage
    } catch (const CLI::ParseError &e) {
        std::exit(app.exit(e));
    }

    /* ---- post‑parse reconciliation ---- */
    try {

        auto [host, embedded_port] = split_addr(opts.addr);
        opts.addr = host;

        if (embedded_port && port_opt->count()) {
            // throw CLI::ParseError("Port specified twice: in --address and
            // --port.");
            std::exit(app.exit(CLI::ValidationError("Port specified twice: in --address and --port.")));
        }

        if (embedded_port)
            opts.port = *embedded_port;
    } catch (const std::exception &e) {
        const auto e_msg = std::format("Failed to split address and port: {}", e.what());
        std::exit(app.exit(CLI::ValidationError(e_msg)));
    }

    return opts;
}

/// @brief Returns the current version of the program in format:
/// "<MAJOR>.<MINOR>"
std::string get_version() { return std::to_string(SLPCLI_VERSION_MAJOR) + "." + std::to_string(SLPCLI_VERSION_MINOR); }

/// @brief Disables std{out,err}
void disable_output() {
    // 1. Save originals
    detail::saved_out = dup(fileno(stdout));
    detail::saved_err = dup(fileno(stderr));

    // 2. Redirect to null device
    (void) freopen(DEV_NULL, "w", stdout);
    (void) freopen(DEV_NULL, "w", stderr);
}

/// @brief Enables std{out,err}
void enable_output() {
    if (detail::saved_out == -1 || detail::saved_err == -1)
        return; // nothing to restore

    fflush(stdout);
    fflush(stderr);

    dup2(detail::saved_out, fileno(stdout));
    dup2(detail::saved_err, fileno(stderr));

    close(detail::saved_out);
    close(detail::saved_err);

    detail::saved_out = detail::saved_err = -1;
}
