# slpcli

🚀 **A simple C++ tool to query the Server List Ping (SLP) of a Minecraft: Java Edition (Notchian) server.**

---

🔧 *A naive implementation of the Server List Ping (SLP) protocol in C++ using [non-boost Asio](https://think-async.com/Asio/).*

📦 **Available on the AUR:** [`slpcli-git`](https://aur.archlinux.org/packages/slpcli-git)

---

## 📌 Usage

```bash
./slpcli [OPTIONS] addr [port]

POSITIONAL ARGUMENTS:
  addr TEXT REQUIRED          Server address (optional format "address:port").
  port UINT                   Server port (default is 25565).

OPTIONS:
  -h, --help                  Display help message and exit.
  -q, --quiet                 Print only JSON response or an empty string if an error occurs.
  -a, --address TEXT REQUIRED Server address (optional format "address:port").
  -p, --port UINT             Server port (default is 25565).
```

## 🛠️ Examples

### 🎯 Basic Usage

Without specifying a port (default port 25565):

```bash
./slpcli mc.hypixel.net
```

Specifying a port:

```bash
./slpcli 23.230.3.162:25572
```

Specifying a port (option 2):

```bash
./slpcli 23.230.3.162 25572
```

Real example for Hypixel, prettified with [`jq`](https://jqlang.org/):

```bash
$ ./slpcli --quiet mc.hypixel.net | jq .
{
  "version": {
    "name": "Requires MC 1.8 / 1.21",
    "protocol": 47
  },
  "players": {
    "max": 200000,
    "online": 31198,
    "sample": []
  },
  "description": "                §aHypixel Network §c[1.8-1.21]\n     §6§lSB 0.23.1 §2§lFORAGING §8§l- §e§lSUMMER EVENT",
  "favicon": "<trimmed for GitHub>"
}
```

### 🔍 Extracting Data with jq

Display the number of online players using [`jq`](https://jqlang.org/):

```bash
./slpcli -q purpleprison.net | jq '.players.online'
# Output: 438
```

### 🖼️ Displaying Server Favicon

Use chained bash commands with [`feh`](https://github.com/derf/feh) to display the server favicon:

```bash
./slpcli mc.hypixel.net -q | jq .favicon -r | cut -d, -f2 | base64 -d | feh -
```

Save favicon as an image file:

```bash
./slpcli mc.hypixel.net -q | jq .favicon -r | cut -d, -f2 | base64 -d > favicon.png
```

### 🤫 Quiet Mode

The `-q` or `--quiet` option suppresses diagnostic messages, outputting only the raw JSON payload or an empty string upon error. Useful for shell pipelines.

---

## 📦 Installation

> [!WARNING]
> The project is currently **only** available on Arch Linux's User Repository (AUR). On other distrubutions and OSs you'll have to manually build it or download a binary in the [Releases](https://github.com/Urpagin/slpcli/releases).  

You have two main ways to install `slpcli` on Arch Linux

1. Use your favorite AUR helper like [`yay`](https://github.com/Jguer/yay) or [`paru`](https://github.com/Morganamilo/paru):
```bash
yay -S slpcli-git
```

2. Install directly from the `PKGBUILD` file
```bash
sudo pacman -S --needed git base-devel
git clone --recursive https://github.com/Urpagin/slpcli.git
cd slpcli/arch-pkg
makepkg -si
```

## 🏗️ Building

Use the provided `run_debug.sh` script or build manually:

### Manual Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

*(To be improved later.)*

---

## 💻 Compatibility

### Platforms

🌐 *Cross-platform enabled thanks to Asio*

| OS      | Compatibility |
| ------- | ------------- |
| Linux   | ✅ YES         |
| macOS   | ✅ YES         |
| Windows | ✅ YES         |

> [!NOTE]
> ⚠️ Manual build required for non-Linux/macOS platforms.

### 📐 C++ Version

* Requires C++23 or newer.

---

## 📖 Integrating SLP Code in Your C++ Project

Starting from a basic project structure:

**main.cpp**

```cpp
int main() {
    return 0;
}
```

**CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.24)
project(myapp)

add_executable(myapp main.cpp)
```

### 🚩 Steps

1. 📥 Clone the repository (**do not omit `--recursive`**):

```bash
git clone --recursive https://github.com/Urpagin/slpcli.git
```

2. 🔗 Link the library in your project's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.24)
project(myapp)

# Include SLP library
add_subdirectory(slpcli/libs/slp)

add_executable(myapp main.cpp)

# Link SLP library
target_link_libraries(myapp PRIVATE slp)
```

3. 📝 Use the library in your project:

```cpp
#include <iostream>
#include "slp.h"

int main() {
    auto ping = slp("mc.hypixel.net");
    auto response = ping.query_slp();
    std::cout << response << std::endl;
    return 0;
}
```

4. 🏭 Build and run:

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./myapp
```

---

## ⚠️ Known Issues

### 🔢 VarInt Handling

VarInt values are stored as `int` (32 bits), limiting full protocol compliance for VarInts larger than 35 bits. This limitation is intentional and acceptable for typical usage scenarios.

## Timeout
There isn't a timeout system in place as of now. This is very problematic as bogus/down hostnames make the program pend.

---

## 📚 References & Acknowledgments

* [Minecraft Protocol](https://minecraft.wiki/w/Java_Edition_protocol/)

* [Server List Ping Protocol](https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping)

* [Packet Format](https://minecraft.wiki/w/Java_Edition_protocol/Packets)

* [String Format](https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String)

* [VarInt Logic](https://minecraft.wiki/w/Java_Edition_protocol/Packets#VarInt_and_VarLong)

* [ASIO library for socket programming](https://think-async.com/Asio/)

* [CLI11 for CLI parsing](https://github.com/CLIUtils/CLI11)
