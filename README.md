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
real gameplay

spell system needs to be ironed out
  -lets wait till we can test gameplay

physics engine, make the interface better to use
  -split the PhysicsEngine class into multiple physics systems, PhysicsEngine should probably be a singleton (ish because jolt wanted it that way)

lighting system, spot lights

Some sort of Scene/Object system, runtime generated shaders for adding lights dynamically


# Done
resistances/damage reduction
 -make armor scale off of max hp, 
   -meaning more armor relative to the scaled stamina grants more % damage reduction
   -abilities need a scaling factor with armor and resistance indicating how much they get reduced by those stats

enemy generation based on item levels
  -Need to do gear generation (done)

character saving to sqlite

model loading
