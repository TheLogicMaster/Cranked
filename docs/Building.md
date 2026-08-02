# Building Cranked

## Downloading Sources
Required libraries are mostly embedded either as submodules or built using Conan.

Recursively clone the repo with: `git clone --recursive https://github.com/TheLogicMaster/Cranked`.

## Conan
Conan must be installed. See the [official instructions](https://docs.conan.io/2/installation.html).
A Conan profile must exist (Create a default one with: `conan profile detect --force`).
Originally libraries would be built as Git submodules, but they were a pain to maintain. Externally installed
or git submodules may still work on Unix for convenience, though. 

## Windows
To build in Release mode with Visual Studio 2022 (Must be installed), run the following in PowerShell from the project directory:
```shell
conan install . --output-folder=build --build=missing -o *:shared=True -s build_type=Release
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
cmake --build . --config Release
```

## Linux
See the [Windows](#Windows) section and replace the generator `-G <name>` argument with the desired CMake generator or
simply remove it to use the default. Building without Conan may still work, but won't necessarily be up to date or build.

## Linking
The project currently builds using shared libraries for external dependencies due to a symbol conflict between *Unicorn*
and *zlib* (*crc32*), but it should be possible to statically link all libraries eventually (Required for homebrew 
platforms and such, anyway).
