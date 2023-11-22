# SmokeyBedrockParser

- Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- Install [Vcpkg](https://github.com/microsoft/vcpkg) and set up command line integration as described in the Vcpkg docs. Remember the location of Vcpkg's `CMAKE_TOOLCHAIN_FILE`
- Run `vcpkg install zlib:x64-windows`
- Navigate to `compile.bat` and edit the path to Vcpkg's `CMAKE_TOOLCHAIN_FILE` to match your installation.
- Run `compile.bat` to build the project.
- If this is successful, SmokeyBedrockParser can be found in `.\build\Release\SmokeyBedrockParser.exe`.