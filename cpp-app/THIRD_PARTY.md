# Third party code

SmartFlow's application source is in `src/` and `include/`. The traffic model and educational drawing algorithms are project code ported from the preserved JavaScript implementation.

The native drawing and window library is **raylib 5.5**, from the official [raylib repository](https://github.com/raysan5/raylib/releases/tag/5.5). The library is written in C and is called by the C++ application. Its use does not make the application's traffic logic JavaScript or a browser wrapper.

The library's original `src` directory is retained under `vendor/raylib-5.5/src`, with `vendor/raylib-5.5/LICENSE`. Embedded dependencies retain their notices in that source tree. The build compiles the library locally and links it into the executable.

Source archive: `https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip`

Downloaded archive SHA256: `00e8a7c85f7a5221e56ee8e4e1c6642b773eae73d29379a91c8ac5149e803f42`

The application reads Segoe UI from the Windows font installation when present. This font is not copied or redistributed. If it is unavailable, the application uses raylib's bundled default font.
