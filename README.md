## Luxafor CLI utility

The company [Luxafor](https://luxafor.com) sells a little RGB LED "[flag](https://luxafor.com/flag-usb-busylight-availability-indicator/)" that connects to a PC via USB-A, and I love it.

But I wanted a CLI client. So I wrote one. And you can use it, if you'd like.

## Download

Prebuilt binaries for Linux, macOS, and Windows are available on the [releases page](https://github.com/mike-rogers/luxafor-cli/releases). Download the one for your platform and run it — no dependencies needed.

## Building from source

Works on macOS, Linux (including Raspberry Pi OS), and Windows.

### Dependencies

You need CMake (3.18+) and a C compiler. If hidapi is installed on your system, the build uses it; otherwise CMake automatically downloads and builds hidapi from source, so installing it is optional.

* macOS: `brew install cmake` (optionally `brew install hidapi`)
* Raspberry Pi OS / Debian / Ubuntu: `sudo apt install cmake build-essential libudev-dev`
* Windows: Visual Studio 2022 (with the "Desktop development with C++" workload) and CMake

### Get the source

`git clone https://github.com/mike-rogers/luxafor-cli.git`

### Build using CMake

From within the `luxafor-cli` directory:

```
cmake -B build
cmake --build build --config Release
```

The binary lands in `build/` (or `build/Release/` with Visual Studio).

To force building hidapi from source even when a system copy exists, configure with `-DLUXAFOR_VENDORED_HIDAPI=ON`.

## Run the program

`./luxafor blue`

### Arguments

The color argument can either be a simple color (`blue`, `green`, etc.) or a hex value, e.g. `0x043f2c`.

Beyond a static color, the Flag's other effects are supported:

```
luxafor --fade 60 red                          # fade to red (0-255, higher is slower)
luxafor --strobe 20 --repeat 5 green           # strobe green 5 times
luxafor --wave 3 --speed 30 blue               # wave effect (types 1-5)
luxafor --pattern 5                            # built-in pattern (1-8)
luxafor --led front blue                       # target LEDs: 1-6, front, back, all
```

Run `luxafor --help` for the full option reference, or `luxafor --version` to see which release you have.

### Linux permissions

On Linux you may need `sudo` to access the device. To allow non-root access, install the included udev rule:

```
sudo cp 99-luxafor.rules /etc/udev/rules.d/
sudo udevadm control --reload && sudo udevadm trigger
```

## License

This project has been released with the MIT license.
