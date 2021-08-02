#include <stdio.h>
#include <SDL.h>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;

SDL_Window* gWindow = NULL;
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
    if (!init()) {
        printf("Init failure!");
        quit();
        return 0;
    }

    gStretchedSurface = loadSurface("hello.bmp");
    SDL_Rect stretchRect;
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = SCREEN_WIDTH;
    stretchRect.h = SCREEN_HEIGHT;

    if (gStretchedSurface) {
        while(!should_quit) {
            while(SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    should_quit = 1;
                }
            }
//            SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
            SDL_BlitScaled(gStretchedSurface, NULL, gScreenSurface, &stretchRect);
            SDL_UpdateWindowSurface(gWindow);
        }
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
        gWindow = SDL_CreateWindow("S-2500 Image Parser", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!gWindow) {
            printf("Window couldn't be created! %s\n", SDL_GetError());
            succ = 0;
        } else {
            gScreenSurface = SDL_GetWindowSurface(gWindow);
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

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}