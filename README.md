# cpp_template

template for cpp projects

has various presets that can be used with cmake --preset=config
example:
 
cmake --preset=linux-clang-debug &&
ninja -C build/debug

# clang format/tidy
copied the clang-tidy file from lefticus's cmake preset github page

clang format is based off of webkit, with a few personal preference modifications

# Todo

gameplay loop
resistances/damage reduction
 -make armor scale off of max hp, 
  -meaning more armor relative to the scaled stamina grants more % damage reduction
  -abilities need a scaling factor with armor and resistance indicating how much they get reduced by those stats



spell system needs to be ironed out
