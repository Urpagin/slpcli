# slpcli

A simple C++ tool to query the SLP (Server List Ping) of a Minecraft: Java Edition (Notchian) server.

---

<small>A naive implementation of the SLP (Server List Ping) protocol in C++ using [non-boost Asio](https://think-async.com/Asio/).</small>

# Usage

```bash
./slpcli [OPTIONS] addr [port]


POSITIONALS:
  addr TEXT REQUIRED          Server address with optional ":port". 
  port UINT                   Port of the Minecraft server (default 25565). 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -q,     --quiet             Only prints the JSON or an empty string if error. 
  -a,     --address TEXT REQUIRED 
                              Server address with optional ":port". 
  -p,     --port UINT         Port of the Minecraft server (default 25565). 
 

```

## Examples

No port:

```bash
./slpcli mc.hypixel.net
```

With port:

```bash
./slpcli 23.230.3.162:25572
```

Output of CLI usage with <a href="https://jqlang.org/" target="_blank" rel="noopener noreferrer">jq</a> to print out the
online players:

```bash
$ ./slpcli -q purpleprison.net | jq '.players.online'
438
```

Output of CLI usage with chained bash commands to show the favicon of Hypixel with feh:
You can also play with Unix commands. Here is the command to show the icon of Hypixel with [feh](https://github.com/derf/feh):
```bash
./slpcli mc.hypixel.net -q | jq .favicon -r | cut -d, -f2 | base64 -d | feh -
```
Or to save it as an image file:
```bash
./slpcli mc.hypixel.net -q | jq .favicon -r | cut -d, -f2 | base64 -d > favicon.png
```

## Options

* `-q`, `--quiet`, Suppresses every diagnostic message on stdout and stderr, so the program writes only the raw JSON
  payload (or an empty string on error). Ideal when the command is part of a shell pipeline.

## Using the Server List Ping code in your C++ project

From this base project:

```cpp
$ cat main.cpp
int main() {
    return 0;
}
```
```cmake
$ cat CMakeLists.txt
cmake_minimum_required(VERSION 3.24)
project(myapp)

add_executable(myapp main.cpp)
```

1. Clone the repositoty
```bash
git clone https://github.com/Urpagin/slpcli.git
```
2. Update link the lib to your project in `CMakeLists.txt`
```bash
cmake_minimum_required(VERSION 3.24)
project(myapp)

# Add this
add_subdirectory(slpcli/libs/slp)

add_executable(myapp main.cpp)

# And this
target_link_libraries(myapp PRIVATE slp)
```
3. Use the lib in your project
```cpp
#include <iostream>
#include "slp.h"

int main() {
    auto ping = slp("mc.hypixel.net");
    auto resp = ping.query_slp();
    std::cout << resp << std::endl;
    return 0;
}
```
4. Build
```bash
mkdir -p build && cd build && cmake .. && make -j$(nproc) && ./myapp
```

# Building

Running the `run_debug.sh` script and pass your arguments to it, or building it manually:

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

* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping" target="_blank" rel="noopener noreferrer">
  SLP Docs</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets" target="_blank" rel="noopener noreferrer">Packet
  Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String" target="_blank" rel="noopener noreferrer">
  String Format</a>
* <a href="https://minecraft.wiki/w/Java_Edition_protocol/Packets#VarInt_and_VarLong" target="_blank" rel="noopener noreferrer">
  VarInt Logic</a>
