#include <stdio.h>
#include <SDL.h>
#include <SDL_render.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DATA_FILE "../data.dat"
//#define DATA_FILE "/dev/ttyACM1"

#define BUF_SIZE 1024

const int SCREEN_WIDTH = 768;
const int SCREEN_HEIGHT = 1024;
const int SOURCE_WIDTH = 768;
const int SOURCE_HEIGHT = 1024;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
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
    uint8_t newFrame = 0;
    uint32_t val;

    if (!init()) {
        printf("Init failure!");
        quit();
        return 0;
    }

    dataBuffer = malloc((sizeof(uint16_t) * BUF_SIZE));
    for (uint16_t i=0; i<BUF_SIZE; ++i) {
        dataBuffer[i] = 0x0;
    }

    gPixels = malloc((sizeof(uint32_t) * SOURCE_WIDTH * SOURCE_HEIGHT));
    memset(gPixels, 0, SOURCE_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = SCREEN_WIDTH;
    stretchRect.h = SCREEN_HEIGHT;

    gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, SOURCE_WIDTH, SOURCE_HEIGHT);

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

        SDL_UpdateTexture(gTexture, NULL, gPixels, SOURCE_WIDTH * sizeof(uint32_t));
        for (uint16_t i=0; i<(status/sizeof(uint16_t)); i++) {
            while (dataBuffer[i] == 0xFEFA || dataBuffer[i] == 0xFEFB) {
                if (dataBuffer[i] == 0xFEFB) {
                    newFrame = 1;
                }
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
                if (newFrame) {
                    newFrame = 0;
                    printf("new frame\n\tx = %d\n\tpulse duration: %f\n\tframe duration %g\n\tscanmode: %d\n", x, syncDuration, frameDuration, scanMode);
                    x = 0;
                    y = 0;
                    SDL_RenderClear(gRenderer);
                    SDL_RenderCopy(gRenderer, gTexture, NULL, &stretchRect);
                    SDL_RenderPresent(gRenderer);
                } else {
                    if (!(y%1)) {
                        printf("scanMode: %d\n\tpulse duration: %f\n\tframe duration: %f\n", scanMode, syncDuration, frameDuration);
                        printf("\tsyncAverage: %f\n\tmaxSync: %f\n\tminSync: %f\n", syncAverage, maxSync, minSync);
                        SDL_RenderClear(gRenderer);
                        SDL_RenderCopy(gRenderer, gTexture, NULL, &stretchRect);
                        SDL_RenderPresent(gRenderer);
                    }
//                    printf("x pulse at x = %d\n\tpulse duration: %f\n\tframe duration %g\n\tscanmode: %d\n", x, syncDuration, frameDuration, scanMode);
                    x = 0;
                    y += 1;
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

            if (y >= SOURCE_HEIGHT) {
                y = 0;
                printf("frame overflow\n");
            }
            if (x < SOURCE_WIDTH) {
                val = ((uint8_t)pixelIntensity) << 24;
                val |= ((uint8_t)pixelIntensity) << 16;
                val |= ((uint8_t)pixelIntensity) << 8;
                val |= ((uint8_t)pixelIntensity);
                gPixels[y * SOURCE_WIDTH + x] = val;
            }
            x += 1;
        }
    }

    quit();
    return 0;
}

int init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failure! %d\n", SDL_GetError());
        return 0;
    }
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer);
    SDL_SetWindowTitle(gWindow, "S-2500 Image Parser");
    if (!gWindow) {
        printf("Window couldn't be created! %s\n", SDL_GetError());
    }
//    gScreenSurface = SDL_GetWindowSurface(gWindow);
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