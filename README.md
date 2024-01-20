# SmokeyBedrockParser

SmokeyBedrockParser is a tool for parsing Minecraft: Bedrock Edition worlds.

## Usage

SmokeyBedrockParser is an executable that takes a path to a Minecraft: Bedrock Edition world as an argument. There is also a GUI version available.

## Building

- Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- Install [Vcpkg](https://github.com/microsoft/vcpkg) and set up command line integration as described in the Vcpkg docs. Remember the location of Vcpkg's `CMAKE_TOOLCHAIN_FILE`
- Run `vcpkg install zlib:x64-windows`
- Run `vcpkg install boost-program-options:x64-windows`
- Run `vcpkg install nativefiledialog:x64-windows`
- Navigate to `compile.bat` and edit the path to Vcpkg's `CMAKE_TOOLCHAIN_FILE` to match your installation.
- Run `compile.bat` to build the project.
- If this is successful, the CLI version can be found in `build/SmokeyBedrockParser-CLI/Release/SmokeyBedrockParser.exe`.
- If this is successful, the GUI version can be found in `build/SmokeyBedrockParser-GUI/Release/SmokeyBedrockParser.exe`.

## Third Party Software

This project uses the following third party software:

- [google/leveldb](https://github.com/google/leveldb)
- [ljfa-ag/libnbtplusplus](https://github.com/ljfa-ag/libnbtplusplus)
- [gabime/spdlog](https://github.com/gabime/spdlog)
- [nlohmann/json](https://github.com/nlohmann/json)