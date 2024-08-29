// SDL2 Hello World demo
// Taken from user cicanci on GitHub
// https://gist.github.com/cicanci/b54715298ab55dff2fbcd0ca3829d13b
#include <SDL.h>

#include <iostream>
#include <random>
#include <ECS/ECS.hpp>

// Window size
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Image size
#define IMAGE_WIDTH 256
#define IMAGE_HEIGHT 256

class RankComponent : public Component { public: int rank; };

class CounterService : public Service { public: int counter; };

class RandIntService : public Service {
    private:
        std::mt19937* _generator = nullptr;
        std::uniform_int_distribution<int> _distribution;

        int _min = 0, _max = 0;

    public:
        RandIntService(int min, int max):
            _min(min), _max(max), 
            _distribution(std::uniform_int_distribution<int>(_min, _max)),
            _generator(new std::mt19937()) 
        {}

        RandIntService(RandIntService&& other) {
            _min = other._min;
            _max = other._max;
            _generator = other._generator;
            _distribution = std::uniform_int_distribution<int>(_min, _max);

            other._generator = nullptr;
        }

        RandIntService& operator=(RandIntService&& other) {
            if (
                _generator != other._generator 
                && _generator != nullptr
            ) {
                delete _generator;
            }

            _min = other._min;
            _max = other._max;
            _generator = other._generator;
            _distribution = std::uniform_int_distribution<int>(_min, _max);

            other._generator = nullptr;
            return *this;
        }

        int GetRandInt() {
            return _distribution(*_generator);
        }

        ~RandIntService() {
            if (_generator != nullptr) {
                delete _generator;
            }
        }
};

using CountingECS = ECS<RankComponent>::WithServices<CounterService, RandIntService>;

void CounterSystem(
    std::tuple<const RankComponent&> components, 
    std::tuple<CounterService&, CountingECS::ManagerService&> services
) {
    std::cout << "> Visiting component with rank " << std::get<0>(components).rank;
    std::cout << " at count " << std::get<0>(services).counter++ << std::endl;;

    if (std::get<0>(services).counter == 2048) {
        std::get<1>(services).RequestStop();
    }

    return;
}

void RankShuffleSystem(
    std::tuple<RankComponent&> components, 
    std::tuple<RandIntService&> services
) {
    int oldRank = std::get<0>(components).rank; 
    std::get<0>(components).rank = std::get<0>(services).GetRandInt();

    std::cout << "> Promoted! From " << oldRank;
    std::cout << " to " << std::get<0>(components).rank << std::endl;

    return;
}

void TestECS() {
    std::cout << "==== Testing ECS! ====" << std::endl;

    std::cout << "Initializing ECS..." << std::endl ;
    CountingECS ecs;

    std::cout << "Adding entities..." << std::endl ;
    ecs.AddEntity(RankComponent{.rank = 1});
    ecs.AddEntity(RankComponent{.rank = 2});
    ecs.AddEntity(RankComponent{.rank = 3});
    ecs.AddEntity();

    std::cout << "Adding systems..." << std::endl ;
    ecs.AddSystem(CounterSystem);
    ecs.AddSystem(RankShuffleSystem);

    std::cout << "Adding service..." << std::endl ;
    ecs.InstallService(CounterService{.counter = 0});
    ecs.InstallService(RandIntService(10, 30));

    std::cout << "Starting ECS..." << std::endl ;
    ecs.Start();

    std::cout << "Waiting for ECS stop..." << std::endl ;
    ecs.AwaitStop();

    std::cout << "==== All done! ====" << std::endl;
}

int main(int argc, char* args[])
{
    TestECS();

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