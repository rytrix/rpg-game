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

spell system 
  - needs to be ironed out
  - wait until there is gameplay

lighting
  - spot lights
  - find a good way to make shadows less pixelated
  - change depth map size on a per light basis

rendering engine
  - swap between deferred and forward rendering 
  - potential for forward passes after deferrered pipeline for transparency
  - particles

models
  - I need some sort of a cool example scene
  - animation graphs? (research needed)
  - learn more blender

ECS
  - (maybe) automatic instanced rendering for the same models
  - or indirect rendering pipeline

physics
  - player character
  - ray selection


# Done
resistances/damage reduction
 - make armor scale off of max hp, 
   - meaning more armor relative to the scaled stamina grants more % damage reduction
   - abilities need a scaling factor with armor and resistance indicating how much they get reduced by those stats

enemy generation based on item levels
  - Need to do gear generation (done)
