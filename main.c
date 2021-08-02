#include <stdio.h>
#include <SDL.h>
#include <SDL_render.h>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gHelloWorld = NULL;
SDL_Surface* gStretchedSurface = NULL;

int init();
int loadMedia();
void quit();
SDL_Surface* loadSurface(char* path);

int main(int argc, char* argv[]) {
    int should_quit = 0;
    SDL_Event e;
    SDL_Rect stretchRect;

    if (!init()) {
        printf("Init failure!");
        quit();
        return 0;
    }


//    gStretchedSurface = loadSurface("hello.bmp");
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = SCREEN_WIDTH;
    stretchRect.h = SCREEN_HEIGHT;

    int x = 0;
    int y = 0;
    while(!should_quit) {
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                should_quit = 1;
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(gRenderer, x, y);
        x += 1;
        if (x >= SCREEN_WIDTH) {
            x = 0;
            y += 1;
            SDL_RenderPresent(gRenderer);
        }
        if (y >= SCREEN_HEIGHT) {
            y = 0;
        }

//            SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
//        SDL_BlitScaled(gStretchedSurface, NULL, gScreenSurface, &stretchRect);
//        SDL_UpdateWindowSurface(gWindow);
    }

    quit();
    return 0;
}

int init() {
    int succ = 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failure! %d\n", SDL_GetError());
        succ = 0;
    } else {
//        gWindow = SDL_CreateWindow("S-2500 Image Parser", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
//                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer);
        SDL_SetWindowTitle(gWindow, "S-2500 Image Parser");
        if (!gWindow) {
            printf("Window couldn't be created! %s\n", SDL_GetError());
            succ = 0;
        } else {
            gScreenSurface = SDL_GetWindowSurface(gWindow);
            SDL_RenderClear(gRenderer);
        }
    }

    return succ;
}

int loadMedia() {
    int succ = 1;

    gHelloWorld = SDL_LoadBMP("hello.bmp");
    if (!gHelloWorld) {
        printf("Failed to load media! %s", SDL_GetError());
        succ = 0;
    }

    return succ;
}

SDL_Surface* loadSurface(char* path) {
    SDL_Surface* optimizedSurface = NULL;

    SDL_Surface* loadedSurface = SDL_LoadBMP(path);
    if (!loadedSurface) {
        printf("Unable to load image %s! Erorr: %s\n", path, SDL_GetError());
    } else {
        optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
        if (!optimizedSurface) {
            printf("Unable to optimize image %s! Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }

    return optimizedSurface;
}

void quit() {
    SDL_FreeSurface(gHelloWorld);
    gHelloWorld = NULL;

    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}