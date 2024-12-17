#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect dstRect = { x, y, textWidth, textHeight };
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void drawSidebar(SDL_Renderer* renderer, TTF_Font* font) {
    // Sidebar background
    SDL_SetRenderDrawColor(renderer, 40, 50, 90, 255);
    SDL_Rect sidebarRect = { 0, 0, 100, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &sidebarRect);

    // Sidebar items
    SDL_Color textColor = { 255, 255, 255 };
    renderText(renderer, font, "HDSD", 20, 50, textColor);
    renderText(renderer, font, "REMOTE", 20, 100, textColor);
    renderText(renderer, font, "Contrib.", 20, 150, textColor);
    renderText(renderer, font, "About", 20, 200, textColor);
}

void drawMainContent(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* contentFont) {
    // Main content background
    SDL_SetRenderDrawColor(renderer, 20, 70, 120, 255);
    SDL_Rect contentRect = { 100, 0, SCREEN_WIDTH - 100, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &contentRect);

    // Title with a shadow effect
    SDL_Color shadowColor = { 0, 0, 0, 100 };
    SDL_Color titleColor = { 255, 255, 255 };
    renderText(renderer, titleFont, "Documentation", 220, 30, shadowColor);
    renderText(renderer, titleFont, "Documentation", 215, 25, titleColor);

    // Content text
    int textX = 150;
    int textY = 100;
    renderText(renderer, contentFont, "Request header: req /{request_id}/ {type_of_request} /{optional}", textX, textY, titleColor);
    renderText(renderer, contentFont, "+ request_id: When using GUI, auto-generated.", textX, textY + 40, titleColor);
    renderText(renderer, contentFont, "+ type_of_request: Takeshot, keylog, shutdown,", textX, textY + 80, titleColor);
    renderText(renderer, contentFont, "  listprocess, listdir, get, RunExe/App", textX, textY + 120, titleColor);
    renderText(renderer, contentFont, "+ Optional: Path to file (when using runexe), time in", textX, textY + 160, titleColor);
    renderText(renderer, contentFont, "  milliseconds (when key log), others can be blank", textX, textY + 200, titleColor);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Remote PC App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 36);  // Adjust font path as needed
    TTF_Font* contentFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 18);
    if (!titleFont || !contentFont) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw UI elements
        drawSidebar(renderer, contentFont);
        drawMainContent(renderer, titleFont, contentFont);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(titleFont);
    TTF_CloseFont(contentFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
