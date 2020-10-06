# QTools

Integrates and wraps Qt5 and VTK functionality.

Download [libbuild](https://github.com/richeytastic/libbuild) for easy build and install of this library.

## Prerequisities
- [r3dvis](https://github.com/richeytastic/r3dvis)
- [Qt5](https://www.qt.io)
- [QuaZip](https://github.com/stachenov/quazip)
- [AppImage](https://github.com/AppImage/AppImageKit) - copy included.

A version of QuaZip is included here (custom_quazip.zip) which uses customised CMake scripts
that leverage the build scripts of [libbuild](https://github.com/richeytastic/libbuild) so
that QuaZip can be easily found when building QTools.

Before building QTools, ensure that the rmv tool is built and installed. This will be
performed automatically if using [libbuild](https://github.com/richeytastic/libbuild).
