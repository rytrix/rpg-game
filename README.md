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

spell system needs to be ironed out
  -be specific ffs
  -currently stats scale linearly, except for crit which is exponential, FIXME

enemy generation based on item levels
    - Need to do gear generation (done)

# Done
resistances/damage reduction
 -make armor scale off of max hp, 
  -meaning more armor relative to the scaled stamina grants more % damage reduction
  -abilities need a scaling factor with armor and resistance indicating how much they get reduced by those stats
