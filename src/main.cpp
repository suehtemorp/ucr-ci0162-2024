// SDL2 utilities
#include <SDL.h>

// Easy I/O
#include <iostream>
#include <fstream>
#include <sstream>

// Filesystem navigation
#include <filesystem>

// Components
#include "Components/Drawing.hpp"
#include "Components/Physics.hpp"

// Systems
#include "Systems/Systems.hpp"

// Services
#include "Services/WindowService.hpp"
#include "Services/AssetStore.hpp"
#include "Services/StopwatchService.hpp"

// Entity-component systems' manager
#include "ECS/ECS.hpp"

// Alias for ECS to use
using GameECS = ECS<
    Physics,
    Drawing
>::WithServices<
    StopwatchService,
    AssetStore,
    WindowService
>;

// Actions to take on said ECS's services

/// @brief Handle the input on a given frame
/// @param ecsManager ECS currently taking place
/// @param stopwatch Physics timekeeping service
void HandleInput(GameECS::ManagerService& ecsManager, 
    StopwatchService& stopwatch) {
    bool exitGame = false;

    // Check for input events
    for (SDL_Event currentEvent; 
        SDL_PollEvent(&currentEvent) == 1;) {
        switch (currentEvent.type)
        {
            // If exiting, stop the ECS
            case SDL_QUIT:
                exitGame = true;
                break;

            // Check if a key was pressed (indirectly by its release)
            case SDL_KEYUP:
                switch (currentEvent.key.keysym.sym)
                {
                    // If P is released, toggle the simulation delta
                    case SDLK_p:
                        stopwatch.Toggle();
                        break;

                    // If ESC is released, stop the ECS
                    case SDLK_ESCAPE:
                        exitGame = true;
                        break;
                
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    // Request the ECS manager to stop running 
    // the systems and services
    if (exitGame) {
        ecsManager.RequestStop();
    }
}

/// @brief Render the entities drawn on the current frame 
/// @param window Window service to draw entities from
void DrawEntities(WindowService& window) {
    // If the last frame was a draw frame, commit the drawings
    if (window.OnDrawFrame()) {
        window.Commit();
    }

    // Attempt to acquire the next drawing frame
    window.AcquireDrawFrame();
}

/// @brief Reset the physics timer after a fixed
/// timestep
/// @param stopwatch 
void ResetDeltaTimer(StopwatchService& stopwatch) {
    // Reset the physics timer after 1000 ticks
    if (stopwatch.Milliseconds() >= 1000) {
        stopwatch.Reset();
    }
}

// Parse services

/// @brief Construct a window service given some input config
/// @param input Input stream containing space-delimited parameters
/// @return Constructed window
WindowService ParseWindow(std::istream& input) {
    // Collect parameters
    int w, h, r, g, b;

    input >> w;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window width");
    }

    input >> h;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window height");
    }

    input >> r;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window red hue");
    }

    input >> g;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window green hue");
    }

    input >> b;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window green hue");
    }

    // Validate them
    if (w < 0 || h < 0) {
        throw std::runtime_error
        ("Invalid window dimensions");
    }

    if (
        r < 0 || r > 255 ||
        g < 0 || g > 255 ||
        b < 0 || b > 255
    ) {
        throw std::runtime_error
        ("Invalid RGB color (hues must be within 0-255)");
    }

    // Build the window
    return std::move(
        WindowService(
            w, h, 60,
            SDL_Color{
                (unsigned char) r, 
                (unsigned char) g, 
                (unsigned char) b
            },
            "Game Window!"
    ));
}

/// @brief Load a font asset given some parameters
/// @param assetStore Asset store to load font into
/// @param input Input stream containing space-delimited parameters
void ParseFont(AssetStore& assetStore, std::istream& input) {
    // Collect parameters
    int r, g, b, s;
    std::string p;

    input >> p;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse font path");
    }

    input >> r;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window red hue");
    }

    input >> g;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window green hue");
    }

    input >> b;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse window green hue");
    }

    input >> s;

    if (input.fail()) {
        throw std::runtime_error
        ("Unable to parse font size");
    }

    // Validate them
    if (s < 0) {
        throw std::runtime_error
        ("Invalid font size");
    }

    if (
        r < 0 || r > 255 ||
        g < 0 || g > 255 ||
        b < 0 || b > 255
    ) {
        throw std::runtime_error
        ("Invalid RGB color (hues must be within 0-255)");
    }

    if (
        !std::filesystem::exists(p) ||
        !std::filesystem::path(p).has_filename() ||
        std::filesystem::path(p).extension() != ".ttf"
    ) {
        throw std::runtime_error
        ("Invalid font file path");
    }

    // Load the font
    assetStore.LoadFont(
        p.c_str(), s, SDL_Color{
            (unsigned char) r, 
            (unsigned char) g, 
            (unsigned char) b
        }
    );
}

/// @brief Load an entity given some parameters
/// @param window Window to draw entities with
/// @param assets Assets to draw entites with
/// @param assets ECS to place entities into
/// @param input Input stream containing space-delimited parameters
/// @return True if loaded succesfully, false otherwise
bool ParseEntity(WindowService& window, AssetStore& assets,
    GameECS& ecs, std::istream& input) {
    // Collect parameters
    std::string l, p;
    int w, h, px, py, vx, vy;
    double a;

    input >> l;

    if (input.fail()) {
        std::cerr << "Unable to parse entity text tag\n";
        return false;
    }

    input >> p;

    if (input.fail()) {
        std::cerr << "Unable to parse entity image filepath\n";
        return false;
    }

    input >> w;

    if (input.fail()) {
        std::cerr << "Unable to parse entity width\n";
        return false;
    }

    input >> h;

    if (input.fail()) {
        std::cerr << "Unable to parse entity height\n";
        return false;
    }

    input >> px;

    if (input.fail()) {
        std::cerr << "Unable to parse entity x coord\n";
        return false;
    }

    input >> py;

    if (input.fail()) {
        std::cerr << "Unable to parse entity y coord\n";
        return false;
    }

    input >> vx;

    if (input.fail()) {
        std::cerr << "Unable to parse entity x velocity\n";
        return false;
    }

    input >> vy;

    if (input.fail()) {
        std::cerr << "Unable to parse entity y velocity\n";
        return false;
    }

    input >> a;

    if (input.fail()) {
        std::cerr << "Unable to parse entity angle of rotation\n";
        return false;
    }

    // Validate them
    if (
        !std::filesystem::exists(p) ||
        !std::filesystem::path(p).has_filename() ||
        !std::filesystem::path(p).has_extension()
    ) {
        std::cerr << "Invalid image path for entity\n";
        return false;
    }

    if (!(w > 0 && h > 0)) {
        std::cerr << "Invalid image size for entity (must be positive)\n";
        return false;
    }

    // Load / find entity image texture
    SDL_Texture* imageTexture = (SDL_Texture*) assets.GetTexture(p.c_str());
    if (imageTexture == nullptr) {
        imageTexture = (SDL_Texture*) assets.LoadImage(
            window, p.c_str(), p.c_str()
        );
    }

    if (imageTexture == nullptr) {
        std::cerr << "Unable to load entity image\n";
        return false;
    }

    // Load / find entity text texture
    glm::uvec2 textSize;
    SDL_Texture* textTexture = (SDL_Texture*) assets.GetTexture(l.c_str());
    if (textTexture == nullptr) {
        textTexture = (SDL_Texture*) assets.LoadText(
            window, l.c_str(), l.c_str(), textSize
        );
    }

    if (textTexture == nullptr) {
        std::cerr << "Unable to load entity text\n";
        return false;
    }

    // Load entity
    ecs.AddEntity(
        Physics{
            .velocity{vx, vy},
            .position{px, py},
            .size{w, h},
            .angle{a},
        },
        Drawing{
            .imageSize{w, h},
            .textSize{textSize},
            .imageRef{imageTexture},
            .textRef{textTexture},
        }
    );

    return true;
}

// File parsing

/// @brief Attempt to parse and utilize the configuration 
/// provided to build entities and assets in the game 
/// @param configPath Path to the configuration file
/// @param assets Asset store used to load texture
/// @param ecs ECS used to add entites
/// @return Window used to render textures
WindowService ParseConfig(
    const std::filesystem::path& configPath,
    AssetStore& assets, 
    GameECS& ecs
) {
    // Sanity check the path
    if (!configPath.has_filename()) {
        throw std::invalid_argument(
            "Invalid configuration path (not a file)"
        );
    }

    // Attempt to load config file from it
    std::ifstream config(configPath.c_str());

    if (!config.is_open()) {
        throw std::runtime_error("Unable to open config file");
    }

    // Read line by line
    // And keep track of the name of the thing to parse
    std::string line, objectName;

    /// Also, keep track of parsed dependencies
    bool windowParsed = false, fontParsed = false;
    WindowService window(
        10, 10, 60, {255,255,255}, "Cargando..."
    ); 

    for (
        unsigned lineNum = 0;
        !config.eof() && std::getline(config, line);
        lineNum += 1
    ) {
        std::istringstream lineStream(line);
        lineStream >> objectName;

        // Game window
        if (objectName == "window") {
            if (windowParsed) {
                throw std::runtime_error(
                    "Window config appears more than once"
                );
            }

            window = std::move(ParseWindow(lineStream));
            windowParsed = true;

            std::cout << "Parsed window" << std::endl;
        }

        // Asset manager's font
        else if (objectName == "font") {
            if (fontParsed) {
                throw std::runtime_error(
                    "Font config appears more than once"
                );
            }

            if (!windowParsed) {
                throw std::runtime_error(
                    "Font config before window's"
                );
            }

            ParseFont(assets, lineStream);
            fontParsed = true;
        }

        // Game entities
        else if (objectName == "entity") {
            if (!windowParsed) {
                throw std::runtime_error(
                    "Entity config appears before window's"
                );
            }

            if (!fontParsed) {
                throw std::runtime_error(
                    "Entity config appears before font's"
                );
            }

            if (!ParseEntity(window, assets, ecs, lineStream)) {
                std::cerr << 
                    "Unable to load entity at line " << lineNum << 
                    std::endl;
            };
        }
    }

    // All is done
    return std::move(window);
}

void RunGame(const char* configFilepath) {
    // Create ECS
    std::cout << "Initializing ECS..." << std::endl;
    GameECS ecs;

    // Create outside services
    std::cout << "Initializing services..." << std::endl;

    // Asset store
    AssetStore assetStore;
    if (assetStore.LoadFont("./assets/fonts/highway_gothic.ttf", 
        10, SDL_Color{255, 255, 255, 255}) == nullptr) {
        return;
    };

    // Window (also adds entities)
    std::cout << "Loading config, window & entities..." << std::endl;
    WindowService windowService = std::move(
        ParseConfig(configFilepath, assetStore, ecs)
    );

    // Add systems
    std::cout << "Adding systems..." << std::endl;
    ecs.AddSystem(PhysicsSystem);
    ecs.AddSystem(DrawingSystem);

    // Install services
    std::cout << "Installing services..." << std::endl;
    ecs.InstallService(std::move(assetStore));
    ecs.InstallService(std::move(windowService));
    ecs.InstallService(StopwatchService());

    // Add actions
    std::cout << "Adding service actions..." << std::endl;
    ecs.AddServiceAction(HandleInput);
    ecs.AddServiceAction(DrawEntities);
    ecs.AddServiceAction(ResetDeltaTimer);

    // Starting running the ECS thread
    std::cout << "Starting ECS..." << std::endl;
    // ecs.Dispatch(); -> Multithreading and SDL might not work together
    ecs.Run();

    // Wait for it to stop
    // ecs.AwaitStop(); -> See comment above
    std::cout << "Quitting ECS..." << std::endl;
}

int main(int argc, char* args[]) {
    // Make note of the usage when not provided
    // the proper amount of args
    if (argc != 2) {
        std::cout 
            << "Usage: " << std::endl 
            << args[0] << " <config filename path>" << std::endl;

        return -2;
    }

    if (
        !std::filesystem::exists(args[1]) ||
        !std::filesystem::path(args[1]).has_filename()
    ) {
        std::cerr 
            << "Invalid path for configuration file: "
            << "\"" << args[1] << "\""
            << std::endl;

        return -1;
    }

    // Initialize SDL
    std::cout << "Initializing SDL..." << std::endl;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL_Init error: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize SDL's TTF
    std::cout << "Initializing SDL TTF..." << std::endl;
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init error: %s\n", SDL_GetError());
        return -1;
    }

    // Run game until exited or an error occurs
    std::cout << "Bootstrapping game..." << std::endl;
    try {
        RunGame(args[1]);
    } catch(const std::exception& e) {
        std::cerr 
            << "An error ocurred while running the game: "
            << "\"" << e.what() << "\""
            << std::endl;
    }

    // Cleanup SDL's TTF
    std::cout << "Cleaning up SDL TTF..." << std::endl;
    TTF_Quit();

    // Cleanup SDL
    std::cout << "Cleaning up SDL..." << std::endl;
    SDL_Quit();

    // All is done
    return 0;
}