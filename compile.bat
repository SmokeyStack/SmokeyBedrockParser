cd SmokeyBedrockParser-GUI/third-party/SmokeyBedrockParser-Core
git apply -p0 patches/leveldb.patch
cd ../../..
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release --parallel