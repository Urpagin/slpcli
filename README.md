> [!IMPORTANT]
> I've made a major breakthrough just now! To fix the infamous `mc.hypixel.net` not working bug! Dear reader, let me a few hours and I shall come back with a program that's actually useful!!!!
> Edit: I'll have to rewrite this crap

# slpcli

A simple C++ tool to query the SLP (Server List Ping) of a Minecraft Notchian (Java Edition) server.

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

## TODO: HOW TO BUILD AND USE THE SLP, TODO 2 MAKE SLP A MODULE

# Building

Running the `run.sh` script and pass your arguments to it, or building it manually:

```bash
mkdir build && cd build
```

```bash
cmake ..
```

TODO: re-write all this
```bash
make -j$(nproc) or cmake --build .
```

# Compatibility

## Platforms

| OS      | Compatibility |
|---------|---------------|
| Linux   | YES ✅         |
| MacOS   | YES ✅         |
| Windows | YES ✅         |

*(Thanks to Asio)*

## C++ Version

* C++20 and above.

## Issues & Bugs

### Note on VarInts

In the program, I represented VarInt values using the `int` data type.
However, a Minecraft VarInt can be 5 bytes long, thus containing 35(5 * 7) bits of actual data.
Since `int`s are encoded using 32 bits, my VarInts are not protocol-perfect.

Even so, this is a risk I am willing to take, having to decode only a small number.

# References

* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping" target="_blank" rel="noopener noreferrer">SLP Docs</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets" target="_blank" rel="noopener noreferrer">Packet Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String" target="_blank" rel="noopener noreferrer">String Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets#VarInt_and_VarLong" target="_blank" rel="noopener noreferrer">VarInt Logic</a>
