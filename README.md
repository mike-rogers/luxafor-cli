## Luxafor CLI utility

The company [Luxafor](https://luxafor.com) sells a little RBG LED "[flag](https://luxafor.com/flag-usb-busylight-availability-indicator/)" that connects to a PC via USB-A, and I love it.

But I wanted a CLI client. So I wrote one. And you can use it, if you'd like.

## Building

These instructions work on both MacOS and Raspian.

### Dependencies

You will need to install the HID API for USB devices and the CMake build system in order to compile this software.

* For MacOS using Homebrew: `brew install hidapi cmake`

* For Raspbian: `sudo apt install libhidapi-dev cmake`

### Get the source

Download the source code (with submodules) with the following:

`gitclone --recurse-submodules http://github.com/mike-rogers/luxafor-cli.git`

### Build using CMake

From within the `luxafor-cli` directory:

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## Run the program

To run, from the build directory, `./luxafor blue`. For some systems you might need to use `sudo ./luxafor blue`.

### Arguments

The program takes one argument, which can either be a simple color (`blue`, `green`, etc.) or a hex value, e.g. `0x043f2c`.

## License

This project has been released with the MIT license.
