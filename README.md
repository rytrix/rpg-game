# cpp_template

template for cpp projects

has various presets that can be used with cmake --preset=config
example:
 
cmake --preset=linux-clang-debug &&
ninja -C build/debug

# package manager
using cpm and vcpkg as package managers

vcpkg requires that it's root directory is on your path

# clang format/tidy
copied the clang-tidy file from lefticus's cmake preset github page

clang format is based off of webkit, with a few personal preference modifications

