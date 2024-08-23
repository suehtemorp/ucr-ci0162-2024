#include <SDL.h>

// Window size
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Image size
#define IMAGE_WIDTH 256
#define IMAGE_HEIGHT 256

int main(int argc, char* args[])
{
    // SDL initialisation
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    // Window creation and position in the center of the screen
    SDL_Window* window = SDL_CreateWindow("Hello World SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        SDL_Log("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    // Render creation
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }

    // Image and texture creation
    SDL_Surface* image = SDL_LoadBMP("niko.bmp");
    if (image == nullptr)
    {
        SDL_Log("SDL_LoadBMP Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    if (texture == nullptr)
    {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        return -1;
    }

    // Free the image (only the texture is used now)
    SDL_FreeSurface(image);
    
    // The image has 256x256 and it is positioned in the middle of the screen
    SDL_Rect rect;
    rect.x = (int)(WINDOW_WIDTH * 0.5f - IMAGE_WIDTH * 0.5f);
    rect.y = (int)(WINDOW_HEIGHT * 0.5f - IMAGE_HEIGHT * 0.5f);
    rect.w = IMAGE_WIDTH;
    rect.h = IMAGE_HEIGHT;

    // Draw the texture in the screen
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_RenderPresent(renderer);

    // Keep the main loop until the window is closed (SDL_QUIT event)
    bool exit = false;
    SDL_Event eventData;
    while (!exit)
    {
        while (SDL_PollEvent(&eventData))
        {
            switch (eventData.type)
            {
            case SDL_QUIT:
                exit = true;
                break;
            }
        }
    }

    // Free the texture
    SDL_DestroyTexture(texture);

    // Destroy the render, window and finalise SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}