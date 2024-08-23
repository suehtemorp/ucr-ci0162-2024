# Instructions
Follow these instructions via a terminal on your Debian-based system, using the folder on which you found these instructions as the working directory.

## Dependencies
You'll likely need:
- `cmake`: The CMake buildsystem for C++. Tested on v3.25.1
- `x86_64-w64-mingw32-g++-posix` & `x86_64-w64-mingw32-gcc-posix`: The MinGW-bundled cross compiler/linker for C/C++ (available as Debian package). Tested on v12-posix

## Build config
Run this CMake tuned commands to configure your app for build. 
Make sure you have the `x86_64-w64-mingw32-g++-posix` cross-compiler installed on your Debian-based system. 
```
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix --no-warn-unused-cli -B./build
```

## Build app
Your app might already be built, but just in case, run the following command to built an executable on the `game/` directory. It will be titled `sdl_game`.
```
cmake --build ./build --config Debug --target sdl_game
```

## Clean app
Run the following command to remove such executable.
```
cmake --build ./build --config Debug --target clean
```

## Run app
Make sure you include the `SDL2.dll` linked library implementation for SDL2 included on the game directory alongside the executable.
Run the executable as any other GUI application.
