# SmokeyBedrockParser

- Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019)
- Install [Vcpkg](https://github.com/microsoft/vcpkg) and set up command line integration as described in the Vcpkg docs. Remember the location of Vcpkg's `CMAKE_TOOLCHAIN_FILE`
- Run `vcpkg install zlib:x64-windows`
- Open 'x64 Native Tools Command Propmpt for VS 2019' from your start menu and change your working directory to this repo source code directory.
- Run `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
- Run `cmake --build build --config Release --parallel`
- If this is successful, mcberepair can be found in `.\build\Release\SmokeyBedrockParser.exe`.