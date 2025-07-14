> [!IMPORTANT]
> I've made a major breakthrough just now! To fix the infamous `mc.hypixel.net` not working bug! Dear reader, let me a few hours and I shall come back with a program that's actually useful!!!!

# slpcli

A simple C++ tool to query the SLP (Status List Ping) of a Minecraft Notchian (Java Edition) server.

---

<small>A naive implementation of the SLP (Server List Ping) protocol in C++.</small>

# Usage

`./McpingerCpp <server> <?port>`  
or  
`./McpingerCpp <server>:<?port>`

## Examples

No port:
```bash
./slpcli play.craftedsurvival.net
```
With port:
```bash
./slpcli 23.230.3.162:25572
```

Output of CLI usage with <a href="https://jqlang.org/" target="_blank" rel="noopener noreferrer">jq</a> to print out the online players:
```bash
$ ./slpcli -q purpleprison.net | jq '.players.online'
438
```

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
* It pends on certain servers.

# References

* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping" target="_blank" rel="noopener noreferrer">SLP Docs</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets" target="_blank" rel="noopener noreferrer">Packet Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String" target="_blank" rel="noopener noreferrer">String Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets#VarInt_and_VarLong" target="_blank" rel="noopener noreferrer">VarInt Logic</a>
