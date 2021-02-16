# thuai-egg-2021-backend

# How to build

- fetch all the submodules
- install vcpkg:
```bash
cd tools/vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install box2d
```
- build with `build.ps1` or `build.sh`

