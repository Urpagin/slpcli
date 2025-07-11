# McpingerCpp

A simple C++ tool to query the SLP (Status List Ping) of a Minecraft Notchian (Java Edition) server.

# Usage

`./McpingerCpp <server> <?port>`

E.g., `./McpingerCpp mc.hypixel.net` or `./McPingerCpp a.minecraft.server 25570`

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

Not yet determined.
