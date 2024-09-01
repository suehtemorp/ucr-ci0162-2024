#pragma once

// Core definition for service
#include "ECS/ECS_Core.hpp"

// SDL color
#include <SDL_pixels.h>

// SDL texture, window and renderer
#include <SDL_render.h>

// SDL font
#include <SDL_ttf.h>

// Character strings
#include <string>

// Memory safe container for textures
#include "Utils/MemoryAliases.hpp"

// Associative container for texture
#include <map>

// Window drawing service for texture formatting
#include "Services/WindowService.hpp"

// Pair data packing
#include <glm/vec2.hpp>

// Forward declaration required (circular dependency)
class WindowService;

/// @brief Lifetime and named-access provider of assets
class AssetStore : public Service {
    private:
        /// @brief Font shared across all texts
        Memory::unique_ptr_with_deleter<TTF_Font, TTF_CloseFont> 
        _font = nullptr;
        /// @brief Font color
        SDL_Color _fontColor{0, 0, 0, 255};

        /// @brief Stored, name-accesible textures
        std::map<
            std::string, 
            Memory::unique_ptr_with_deleter<
                SDL_Texture,
                SDL_DestroyTexture
            >
        > _textures;

    public:
        /// @brief Construct an empty asset store
        AssetStore();

        /// @brief Construct an asset store by stealing the resources of 
        /// another
        /// @param other Asset store to take underlying resources from
        AssetStore(AssetStore&& other);

        /// @brief Destroy the current window and its associated resources
        ~AssetStore();

        /// @brief Steal then own the resources of another asset store
        /// @param other Asset store to take underlying resources from
        /// @return Reference to this asset store
        AssetStore& operator=(AssetStore&& other);

        /// @brief Load a font to use across all texts
        /// @param path (Relative) filepath to load font from
        /// @param fontSize Size of font
        /// @param color Color of font
        /// @return Pointer to valid font if loaded sucessfully, nullptr otherwise
        const TTF_Font* LoadFont(const char* filepath, unsigned fontSize = 10, 
            SDL_Color color = SDL_Color{0, 0, 0, 255});

        /// @brief Load an image given that its a supported format
        /// @param window Window service utilized to draw 
        /// @param filepath (Relative) filepath of image file on disk
        /// @param nickname Unused nickname to assign to resulting texture 
        /// @remark If the nickname isn't provided, the filepath will be used on its place 
        /// @return Pointer to valid texture if loaded sucessfully, nullptr otherwise
        const SDL_Texture* LoadImage(const WindowService& window, const char* filepath, 
            const char* nickname);

        /// @brief Load a texture based on some text message
        /// @param window Window service utilized to draw 
        /// @param text Text to be rendered
        /// @param nickname Unused nickname to assign to resulting texture 
        /// @return Pointer to valid texture if loaded sucessfully, nullptr otherwise
        const SDL_Texture* LoadText(const WindowService& window, const char* text, 
            const char* nickname);

        /// @brief Load a texture based on some text message
        /// @param window Window service utilized to draw 
        /// @param text Text to be rendered
        /// @param nickname Unused nickname to assign to resulting texture 
        /// @param size Returned-by-parameter size of resulting texture 
        /// @return Pointer to valid texture if loaded sucessfully, nullptr otherwise
        const SDL_Texture* LoadText(const WindowService& window, const char* text, 
            const char* nickname, glm::uvec2& size);

        /// @brief Retrieve the font stored in the asset store
        /// @return Pointer to valid font if found, nullptr otherwise
        const TTF_Font* GetFont();

        /// @brief Retrieve the font color stored in the asset store 
        /// @return Const-reference to font color stored 
        const SDL_Color& GetFontColor();

        /// @brief Retrieve a texture stored in the asset store
        /// @param nickname Nickname assigned to the texture when loaded
        /// @return Pointer to valid texture if found, nullptr otherwise
        const SDL_Texture* GetTexture(const char* nickname);
};

static_assert(
    ServiceType<AssetStore>, 
    "AssetStore service constraint violated"
);
