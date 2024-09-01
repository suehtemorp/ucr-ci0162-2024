// Easy I/O
#include <iostream>
#include <fstream>
#include <sstream>

// Filesystem navigation
#include <filesystem>

// Core game definitions
#include "Game/Core.hpp"

// Definitions
#include "Game/Parsing.hpp"

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
            "RAM Gobbler (TM)"
    ));
}

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

WindowService ParseConfig(
    const char* configPath,
    AssetStore& assets, 
    GameECS& ecs
) {
    // Sanity check the path
    if (!std::filesystem::path(configPath).has_filename()) {
        throw std::invalid_argument(
            "Invalid configuration path (not a file)"
        );
    }

    // Attempt to load config file from it
    std::ifstream config(configPath);

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
