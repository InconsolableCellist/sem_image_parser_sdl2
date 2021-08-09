#include <stdio.h>
#include <SDL.h>
#include <SDL_render.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//#define DATA_FILE "data.dat"
#define DATA_FILE "/dev/ttyACM3"

const int SCREEN_WIDTH = 3100;
const int SCREEN_HEIGHT = 2000;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gHelloWorld = NULL;
SDL_Surface* gStretchedSurface = NULL;

int gDataFile = 0;


int init();
int loadMedia();
void quit();
SDL_Surface* loadSurface(char* path);

int main(int argc, char* argv[]) {
    int should_quit = 0;
    int x = 0;
    int y = 0;
    uint16_t data = 0;
    double syncDuration = 0;
    double pixelIntensity = 0;
    uint8_t scanMode = 0;
    int status = 0;
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

    while(!should_quit) {
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                should_quit = 1;
            }
        }

        status = read(gDataFile, &data, 2);
        if (status <= 0) {
            printf("Fatal error when trying to read data! Status: %d. Errno: %d\n", status, errno);
            continue;
        }
        while (data == 0xFEFE) {
            read(gDataFile, &data, 2);
            syncDuration = data/16;
            read(gDataFile, &data, 2);
            syncDuration += ((double)data) / 1000000;
            read(gDataFile, &data, 2);
            scanMode = data;
            if (syncDuration >= 0.5) {
                x = 0;
                y = 0;
            } else {
                if (!(y%4)) {
                    SDL_RenderPresent(gRenderer);
                    printf("scanMode: %d", scanMode);
                }
                x = 0;
                y += 5;
            }

            read(gDataFile, &data, 2);
        }

        pixelIntensity = floor( ((double)data) /1024*255);
        if (pixelIntensity > 255) {
            pixelIntensity = 255;
        }

        SDL_SetRenderDrawColor(gRenderer, pixelIntensity, pixelIntensity, pixelIntensity, 255);
        SDL_RenderDrawPoint(gRenderer, x, y);
        SDL_RenderDrawPoint(gRenderer, x, y+1);
        SDL_RenderDrawPoint(gRenderer, x, y+2);
        SDL_RenderDrawPoint(gRenderer, x, y+3);
        SDL_RenderDrawPoint(gRenderer, x, y+4);
        x += 1;
        if (x >= SCREEN_WIDTH) {
            x = 0;
            y += 1;
//            SDL_RenderPresent(gRenderer);
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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failure! %d\n", SDL_GetError());
        return 0;
    }
//        gWindow = SDL_CreateWindow("S-2500 Image Parser", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
//                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer);
    SDL_SetWindowTitle(gWindow, "S-2500 Image Parser");
    if (!gWindow) {
        printf("Window couldn't be created! %s\n", SDL_GetError());
    }
    gScreenSurface = SDL_GetWindowSurface(gWindow);
    SDL_RenderClear(gRenderer);
    gDataFile = open(DATA_FILE, O_RDONLY);
    if (gDataFile == -1) {
        printf("Unable to open data file %s!\n", DATA_FILE);
        return 0;
    }

    return 1;
}

int loadMedia() {
    int succ = 1;

    gHelloWorld = SDL_LoadBMP("hello.bmp");
    if (!gHelloWorld) {
        printf("Failed to load media! %s\n", SDL_GetError());
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