# thuai-egg-2021-backend

[![build](https://github.com/ssast-tech/thuai-egg-2021-backend/actions/workflows/main.yml/badge.svg)](https://github.com/ssast-tech/thuai-egg-2021-backend/actions/workflows/main.yml)

# How to build

- fetch all the submodules

For windows:

- install `vcpkg`:

```bash
cd tools/vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install box2d:x64-windows
```

build with `build.ps1`

For OSX/Linux:

- install `vcpkg`:
```bash
cd tools/vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install box2d
```
build with `build.sh`

