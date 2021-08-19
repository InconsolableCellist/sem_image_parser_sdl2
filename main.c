#include <stdio.h>
#include <SDL.h>
#include <SDL_render.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//#define DATA_FILE "data.dat"
#define DATA_FILE "/dev/ttyACM1"

#define BUF_SIZE 1024

const int SCREEN_WIDTH = 768;
const int SCREEN_HEIGHT = 1024;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gHelloWorld = NULL;
SDL_Surface* gStretchedSurface = NULL;
SDL_Texture* gTexture = NULL;
uint32_t* gPixels = NULL;

int gDataFile = 0;


int init();
int loadMedia();
void quit();
SDL_Surface* loadSurface(char* path);

int main(int argc, char* argv[]) {
    int should_quit = 0;
    int x = 0;
    int y = 0;
    uint16_t* dataBuffer;
    double syncDuration = 0;
    double frameDuration = 0;
    double pixelIntensity = 0;
    uint8_t scanMode = 0;
    int status = 0;
    uint16_t min = 65535;
    uint16_t max = 0;
    double minSync = 65535;
    double maxSync = 0;
    double syncAverage = 0;
    uint32_t syncNum = 0;
    SDL_Event e;
    SDL_Rect stretchRect;
    uint32_t bytesRead = 0;

    if (!init()) {
        printf("Init failure!");
        quit();
        return 0;
    }

    dataBuffer = malloc((sizeof(uint16_t) * BUF_SIZE));
    for (uint16_t i=0; i<BUF_SIZE; ++i) {
        dataBuffer[i] = 0x0;
    }

    gPixels = malloc((sizeof(uint32_t) * SCREEN_WIDTH * SCREEN_HEIGHT));
    memset(gPixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
//    for (uint16_t i=0; i<SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
//        gPixels[i] =
//    }

//    gStretchedSurface = loadSurface("hello.bmp");
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = SCREEN_WIDTH;
    stretchRect.h = SCREEN_HEIGHT;

    gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);

    while(!should_quit) {
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                should_quit = 1;
            }
        }

        status = read(gDataFile, dataBuffer, BUF_SIZE*sizeof(uint16_t));
        bytesRead += status;
        if (status <= 0) {
            printf("End of data or fatal error when trying to read dataBuffer! Status: %d. Total read: %d. Errno: %d\n", status, bytesRead, errno);
            getchar();
            should_quit = 1;
        }

//        SDL_RenderPresent(gRenderer);
        SDL_UpdateTexture(gTexture, NULL, gPixels, SCREEN_WIDTH * sizeof(uint32_t));
        for (uint16_t i=0; i<(status/sizeof(uint16_t)); i++) {
            while (dataBuffer[i] == 0xFEFA) {
                syncDuration = dataBuffer[++i] / 16;
                syncDuration += ((double)dataBuffer[++i]) / 1000000;
                scanMode = dataBuffer[++i];
                frameDuration = dataBuffer[++i] / 16;
                frameDuration += ((double)dataBuffer[++i]) / 1000000;

                if (syncDuration > maxSync) {
                    printf("minSync/maxSync: %f/%f\n", minSync, maxSync);
                    maxSync = syncDuration;
                }
                if (syncDuration < minSync) {
                    printf("minSync/maxSync: %f/%f\n", minSync, maxSync);
                    minSync = syncDuration;
                }
                if (syncDuration >= 0.04) {
                    printf("new frame\n\tx = %d\n\tpulse duration: %f\n\tframe duration %g\n\tscanmode: %d\n", x, syncDuration, frameDuration, scanMode);
                    x = 0;
                    y = 0;
                    SDL_RenderClear(gRenderer);
                    SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
                    SDL_RenderPresent(gRenderer);
                } else {
                    if (!(y%1)) {
                        printf("scanMode: %d\n\tpulse duration: %f\n\tframe duration: %f\n", scanMode, syncDuration, frameDuration);
                        printf("\tsyncAverage: %f\n\tmaxSync: %f\n\tminSync: %f\n", syncAverage, maxSync, minSync);
                        SDL_RenderClear(gRenderer);
                        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
                        SDL_RenderPresent(gRenderer);
                    }
//                    SDL_RenderPresent(gRenderer);
//                    printf("x pulse at x = %d\n\tpulse duration: %f\n\tframe duration %g\n\tscanmode: %d\n", x, syncDuration, frameDuration, scanMode);
                    x = 0;
                    y += 2;
                }
                syncNum += 1;
                syncAverage += syncDuration/syncNum;

                i++;
            }

            if (dataBuffer[i] > max && dataBuffer[i] < 8192) {
                max = dataBuffer[i];
                printf("min/max: %d/%d\n", min, max);
            }
            if (dataBuffer[i] < min) {
                min = dataBuffer[i];
                printf("min/max: %d/%d\n", min, max);
            }

            if (max == 0) {
               max = 1;
            }
//            pixelIntensity = floor(((double)dataBuffer[i]) / max) * 255;
            pixelIntensity = (((double)dataBuffer[i]) / max) * 255;
            if (pixelIntensity > 255) {
                pixelIntensity = 255;
            }
//            if (pixelIntensity < 100) {
//                pixelIntensity = 100;
//            }

            if (x < SCREEN_WIDTH) {
//                SDL_SetRenderDrawColor(gRenderer, pixelIntensity, pixelIntensity, pixelIntensity, 255);
//                SDL_RenderDrawPoint(gRenderer, x, y);
//                SDL_RenderDrawPoint(gRenderer, x, y+1);
//                SDL_RenderDrawPoint(gRenderer, x, y+2);
//                SDL_RenderDrawPoint(gRenderer, x, y+3);
//                SDL_RenderDrawPoint(gRenderer, x, y+4);
                uint32_t val = ((uint8_t)pixelIntensity) << 24;
                val |= ((uint8_t)pixelIntensity) << 16;
                val |= ((uint8_t)pixelIntensity) << 8;
                val |= ((uint8_t)pixelIntensity);
                gPixels[y * SCREEN_WIDTH + x] = val;
                if (y < SCREEN_HEIGHT - 4) {
                    gPixels[(y+1) * SCREEN_WIDTH + x] = val;
//                    gPixels[(y+2) * SCREEN_WIDTH + x] = val;
//                    gPixels[(y+3) * SCREEN_WIDTH + x] = val;
//                    gPixels[(y+4) * SCREEN_WIDTH + x] = val;
                }
            }
            x += 1;
            if (y >= SCREEN_HEIGHT) {
                y = 0;
                printf("frame overflow\n");
            }
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

    SDL_DestroyTexture(gTexture);

    close(gDataFile);

    free(gPixels);

    SDL_Quit();
}