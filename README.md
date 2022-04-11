# libsm64-retro64 - Super Mario 64 as a library, for Retro64 Minecraft Mod

This is a fork of [libsm64](https://github.com/libsm64/libsm64) intended for use with the Retro64 Minecraft mod.
Main changes include Audio support (from [sm64-port](https://github.com/sm64-port/sm64-port)), functions for affecting Mario (changing powerup, damage, teleport, water level, etc), extending the code to work with coordinates > 32768, and other misc. changes for the mod.

If you want to use this for a project, I suggest you look at the original (linked above) as this is intended for a very specific use-case, and is very hacked together & un-documented (especially audio support!)

## Building on Linux

- Ensure python3 is installed.
- Ensure the SDL2 and GLEW libraries are installed if you're building the test program (on Ubuntu: libsdl2-dev, libglew-dev)
- Run `make` to build
- To run the test program you'll need a SM64 US ROM in the root of the repository with the name `baserom.us.z64`.

## Building on Windows (test program not supported)
- [Follow steps 1-4 for setting up MSYS2 MinGW 64 here](https://github.com/sm64-port/sm64-port#windows), but replace the repository URL with `https://github.com/Retro64Mod/libsm64-retro64.git`
- Run `make` to build

## Building on macOS
- Open up terminal and clone the macOS repository with `git clone https://github.com/EmeraldLoc/libsm64-retro64/ -b macOS`
- Cd to libsm64-retro64 with `cd libsm64-retro64`
- Install homebrew at https://brew.sh
- Run `brew install make mingw-w64 gcc sdl2 pkg-config glew glfw libusb audiofile coreutils`
- Run `gmake -j`

## Make targets (all platforms)

- `make lib`: (Default) Build the `dist` directory, containing the shared object or DLL and public-facing header.
- `make test`: Builds the library `dist` directory as well as the test program. (does not run on macOS)
- `make run`: Build and run the SDL+OpenGL test program.

