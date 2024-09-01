#include "AssetStore.hpp"

// Image loading
#include <SDL_image.h>

// Error reporting
#include "stdio.h"

AssetStore::AssetStore()
{}

AssetStore::AssetStore(AssetStore &&other) {
    // Steal the other asset store service's font 
    // and textures
    _font = std::move(other._font);
    _textures = std::move(other._textures);

    // Keep track of its color
    _fontColor = other._fontColor;
}

AssetStore::~AssetStore() 
{}

AssetStore &AssetStore::operator=(AssetStore &&other) {
    // Steal the other asset store service's font 
    // and textures
    _font = std::move(other._font);
    _textures = std::move(other._textures);

    // Keep track of its color
    _fontColor = other._fontColor;

    return *this;
}

const TTF_Font* AssetStore::LoadFont(const char *filepath, 
    unsigned fontSize, SDL_Color color) {
    // Try to load font with given size
    TTF_Font* font = TTF_OpenFont(filepath, fontSize);

    // Report failure if it ocurred
    if (font == nullptr) {
        fprintf(
            stderr,
            "AssetStore: LoadFont couldn't load font at path \"%s\": %s\n",
            filepath,
            TTF_GetError()
        );

        return nullptr;
    }

    // Otherwise, replace previous font (if any) with current one
    _font.reset(font);

    // And return newly loaded font
    return font;
}

const SDL_Texture* AssetStore::LoadImage(const WindowService &window, 
    const char *filepath, const char *nickname) {
    // Attempt to reserve an empty spot on the associative container
    auto [slot, placed] = _textures.try_emplace(nickname, nullptr);

    // Report failure if it ocurred (nick already utilized)
    if (!placed) {
        fprintf(
            stderr, 
            "AssetStore: LoadImage couldn't reserve spot given " \
            "already-assigned nick \"%s\"\n",
            nickname
        );

        return nullptr;
    }

    // Attempt to load image located at path as a texture
    // We need to quickly examine the guts of the window to locate the renderer
    SDL_Texture* texture = IMG_LoadTexture(window._renderer.get(), filepath);

    // Report failure if it ocurred
    if (texture == nullptr) {
        fprintf(
            stderr, 
            "AssetStore: LoadImage couldn't load image at path \"%s\": %s\n",
            filepath,
            IMG_GetError()
        );

        // Free the reserved spot
        _textures.erase(slot);

        return nullptr;
    }

    // Assign the created texture to the reserved slot
    slot->second.reset(texture);

    // And return newly created texture
    return texture;
}

const SDL_Texture *AssetStore::LoadText(const WindowService& window, 
    const char *text, const char *nickname) {
    // Call specialization, but discard return by parameter
    glm::uvec2 ignored;
    return LoadText(window, text, nickname, ignored);
}

const SDL_Texture *AssetStore::LoadText(const WindowService &window, const char *text, 
    const char *nickname, glm::uvec2 &size) {
    // Report missing font (if at all)
    if (_font == nullptr) {
        fprintf(stderr, "AssetStore: LoadText missing font\n");
        return nullptr;
    }

    // Attempt to reserve an empty spot on the associative container
    auto [slot, placed] = _textures.try_emplace(nickname, nullptr);

    // Report failure if it ocurred (nick already utilized)
    if (!placed) {
        fprintf(
            stderr, 
            "AssetStore: LoadText spot already assigned for nick \"%s\"\n",
            nickname
        );

        return nullptr;
    }

    // Attempt to create a surface and then a texture for the given text
    SDL_Surface* textSurface =
    TTF_RenderText_Solid(_font.get(), text, _fontColor); 

    // Report surface generation errors
    if (textSurface == nullptr) {
        fprintf(
            stderr, 
            "AssetStore: LoadText couldn't generate surface for text \"%s\": %s\n",
            text,
            TTF_GetError()
        );

        // Free the reserved spot
        _textures.erase(slot);

        return nullptr;
    }

    // Record the text's width and height
    unsigned width = textSurface->w, height = textSurface->h;

    SDL_Texture* textTexture = 
    SDL_CreateTextureFromSurface(window._renderer.get(), textSurface);

    // Generated surface is no longer needed
    SDL_FreeSurface(textSurface);

    // Report texture generation errors
    if (textTexture == nullptr) {
        fprintf(
            stderr, 
            "AssetStore: LoadText couldn't generate texture for text \"%s\": %s\n",
            text,
            SDL_GetError()
        );

        // Free the reserved spot
        _textures.erase(slot);

        return nullptr;
    }

    // Assign the created texture to the reserved slot
    slot->second.reset(textTexture);

    // And the width and height to the caller
    size = {width, height};

    // And return newly created texture
    return textTexture;
}

const TTF_Font *AssetStore::GetFont()
{ return _font.get(); }

const SDL_Color &AssetStore::GetFontColor()
{ return _fontColor; }

const SDL_Texture *AssetStore::GetTexture(const char *nickname) {
    // Lookup texture by nickname
    const auto whichTexture = _textures.find(nickname);

    // Report found value (or nil if not found)
    if (whichTexture == _textures.cend()) {
        return nullptr;
    }

    const SDL_Texture* texture = whichTexture->second.get();

    // Warn if its nil
    if (texture == nullptr) {
        fprintf(
            stderr, 
            "AssetStore: GetTexture returned null pointer for nick \"%s\"\n",
            nickname
        );
    }

    return texture;
}
