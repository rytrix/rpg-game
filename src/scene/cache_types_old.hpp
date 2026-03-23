#pragma once

#define ModelCache ResourceManager<std::string, Renderer::Model, 100>
#define TextureCache ResourceManager<std::string, Renderer::Texture, 500>