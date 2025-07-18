#include <CLI/CLI.hpp>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <iostream>
#include <netdb.h>
#include <string>
#include <utility>

#include "slp.h"
#include "slpcliConfig.h"
#include <string_view>

void disable_output();
void enable_output();
std::pair<std::string, uint16_t> read_server_address(int, char *[]);
std::string get_version();

struct AppOptions {
  bool is_quiet{false};
  std::string addr;
  uint16_t port{25565};
};
AppOptions parse_args(int, char **);

int main(const int argc, char *argv[]) {
  // This syntax is so cool
  const auto [is_quiet, addr, port] = parse_args(argc, argv);

  if (is_quiet)
    disable_output();

  std::cout << "Querying '" << addr << ":" << port << "'...\n\n" << std::endl;

  const slp serv(addr, port);
  const std::string slp_response{serv.query_slp()};
  //enable_output();
  std::cout << "Hello";
  std::cout << "''" << slp_response << "'" << std::endl;

  return 0;
}

/// @brief Split "<addr>:<port>" into {addr, optional<port>}.
/// If the colon is missing, second element is std::nullopt.
std::pair<std::string_view, std::optional<uint16_t>>
split_addr(std::string_view input) {
  const std::size_t pos = input.find(':');
  if (pos == std::string_view::npos)
    return {input, std::nullopt};

  std::string_view host = input.substr(0, pos);
  const std::string_view port_sv = input.substr(pos + 1);

  if (port_sv.empty()) // “host:”
    return {host, std::nullopt};

  uint16_t port = static_cast<uint16_t>(std::stoi(std::string(port_sv)));
  return {host, port};
}

/// @brief Parses the CLI arguments.
AppOptions parse_args(const int argc, char **argv) {
  CLI::App app{std::format("Minecraft: Java Edition SLP CLI (version: {})",
                           get_version())};

  AppOptions opts;

  app.add_flag("-q,--quiet", opts.is_quiet,
               "Only prints the JSON or an empty string if error.");
  app.add_option("-a,--address,addr", opts.addr,
                 "Server address with optional \":port\".")
      ->required();
  const auto port_opt =
      app.add_option("-p,--port,port", opts.port,
                     "Port of the Minecraft server (default 25565).");

  try {
    app.parse(argc, argv); // may std::exit on help/usage
  } catch (const CLI::ParseError &e) {
    std::exit(app.exit(e));
  }

  /* ---- post‑parse reconciliation ---- */
  auto [host, embedded_port] = split_addr(opts.addr);
  opts.addr = host;

  if (embedded_port && port_opt->count()) {
    // throw CLI::ParseError("Port specified twice: in --address and --port.");
    std::exit(app.exit(CLI::ValidationError(
        "Port specified twice: in --address and --port.")));
  }

  if (embedded_port)
    opts.port = *embedded_port;

  return opts;
}

/// @brief Returns the current version of the program in format:
/// "<MAJOR>.<MINOR>"
std::string get_version() {
  return std::to_string(SLPCLI_VERSION_MAJOR) + "." +
         std::to_string(SLPCLI_VERSION_MINOR);
}

/// @brief Disables stdout and stderr.
void disable_output() {
#ifdef _WIN32
  freopen("NUL", "w", stdout);
  freopen("NUL", "w", stderr);
#else
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "w", stderr);
#endif
}

/// @brief Enables stdout and stderr.
void enable_output() {
#ifdef _WIN32
  freopen("CON", "w", stdout);
  freopen("CON", "w", stderr);
#else
  freopen("/dev/tty", "w", stdout);
  freopen("/dev/tty", "w", stderr);
#endif
}