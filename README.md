# McpingerCpp

A simple C++ tool to query the SLP (Status List Ping) of a Minecraft Notchian (Java Edition) server.

---

<small>A naive implementation of the SLP (Server List Ping) protocol in C++.</small>

# Usage

`./McpingerCpp <server> <?port>`  
or  
`./McpingerCpp <server>:<?port>`

E.g., `./McpingerCpp mc.hypixel.net` or `./McPingerCpp mc.server.net:25570`

## Options

* `-q`, `--quiet`, Suppresses every diagnostic message on stdout and stderr, so the program writes only the raw JSON payload (or an empty string on error). Ideal when the command is part of a shell pipeline.

# Building

Running the `run.sh` script and pass your arguments to it, or building it manually:

```bash
mkdir build && cd build
```

```bash
cmake ..
```

```bash
make -j$(nproc)
```

# Compatibility

## Platforms

| OS      | Compatibility |
|---------|---------------|
| Linux   | YES ✅         |
| MacOS   | MAYBE 🤔       |
| Windows | NO ❌          |

## C++ Version

* C++20 and above.

## Issues & Bugs

* The program does not seem to work for all Minecraft servers. It does not work for `mc.hypixel.net` but does for `bmc.mineflake.net`.

* It sometimes pends for a long time on certain servers.
