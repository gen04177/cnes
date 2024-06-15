//  cwnd.c
//  TestNes
//
//  Created by arvin on 2017/8/16.
//  Copyright © 2017年 com.fuwo. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define WIDTH   256
#define HEIGHT  240
#define MAG     1

uint8_t wnd_dsbtn(SDL_Event *event, uint8_t* ctrl);

static uint32_t palette_sys[] =
{
    0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00,
    0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000,
    0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
    0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000,
    0xFFFEFF, 0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22,
    0xBCBE00, 0x88D800, 0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
    0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
    0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000,
};

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_Rect rect;

static uint8_t pic_mem_orgl[WIDTH * HEIGHT * 4];
static uint8_t pic_mem_frnt[WIDTH * HEIGHT * 4 * MAG * MAG];
static int full_screen = 0;
static uint32_t frame_counter;
static uint32_t time_frame0;

int wnd_init(const char *filename)
{
    int error = 0;
    if ((error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER)) != 0) {
        fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = 44100;
    wanted_spec.format = AUDIO_F32SYS;
    wanted_spec.channels = 1;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;
    wanted_spec.callback = NULL;
    if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
        printf("can't open audio.\n");
        return -1;
    }
    SDL_PauseAudio(0);

    rect.x = rect.y = 0;
    rect.h = HEIGHT * MAG;
    rect.w = WIDTH * MAG;

    window = SDL_CreateWindow(filename,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WIDTH * MAG, HEIGHT * MAG,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Couldn't create SDL window: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Couldn't create SDL renderer: %s\n", SDL_GetError());
        return -1;
    }
    if (SDL_RenderSetLogicalSize(renderer, WIDTH * MAG, HEIGHT * MAG) != 0) {
        fprintf(stderr, "Couldn't set SDL renderer logical resolution: %s\n", SDL_GetError());
        return -1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH * MAG, HEIGHT * MAG);
    if (!texture) {
        fprintf(stderr, "Couldn't create SDL texture: %s\n", SDL_GetError());
        return -1;
    }

    frame_counter = 0;
    time_frame0 = SDL_GetTicks();

    return 0;
}

static uint32_t wnd_time_left(void)
{
    uint32_t now, timer_next;
    now = SDL_GetTicks();
    timer_next = (frame_counter * 1001) / 60 + time_frame0;
    if (timer_next <= now) return 0;
    return timer_next - now;
}

void wnd_draw(uint8_t* pixels)
{
    uint8_t *fb = pic_mem_orgl;
    for (unsigned int y = 0; y < HEIGHT; y++) {
        for (unsigned int x = 0; x < WIDTH; x++) {
            uint32_t pixel = *(pixels + WIDTH * y + x);
            uint32_t rgb = palette_sys[pixel];
            *fb++ = (rgb >> 16) & 0xFF;  // R
            *fb++ = (rgb >> 8) & 0xFF;   // G
            *fb++ = rgb & 0xFF;          // B
        }
    }

    ++frame_counter;
    SDL_Delay(wnd_time_left());

    void *p;
    int pitch;
    SDL_LockTexture(texture, &rect, &p, &pitch);
    SDL_memcpy(p, pic_mem_orgl, HEIGHT * MAG * WIDTH * MAG * 3);
    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &rect, &rect);
    SDL_RenderPresent(renderer);
}

void wnd_play(float com)
{
    SDL_QueueAudio(1, &com, sizeof(float));
}

int wnd_poll(uint8_t* ctrl)
{
    SDL_Event event;

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_GameControllerOpen(i) == NULL) {
            printf("SDL_GameControllerOpen(%i): %s\n", i, SDL_GetError());
        }
    }

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                wnd_dsbtn(&event, ctrl);
                break;
        }
    }
    return 0;
}

uint8_t wnd_dsbtn(SDL_Event *event, uint8_t* ctrl)
{
    uint8_t btn = 0;
    
    switch (event->type) {
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            SDL_ControllerButtonEvent *cbutton = (SDL_ControllerButtonEvent *)event;
            switch (cbutton->button) {
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    btn = 1 << 3;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    btn = 1 << 2;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    btn = 1 << 1;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    btn = 1 << 0;
                    break;
                case SDL_CONTROLLER_BUTTON_A:
                    btn = 1 << 6;
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    btn = 1 << 7;
                    break;
                case SDL_CONTROLLER_BUTTON_X:
                    btn = 1 << 5; // 
                    break;
                case SDL_CONTROLLER_BUTTON_Y:
                    btn = 1 << 4; // 
                    break;
            }
            if (event->type == SDL_CONTROLLERBUTTONDOWN) {
                ctrl[0] |= btn;
                ctrl[1] |= btn;
            } else if (event->type == SDL_CONTROLLERBUTTONUP) {
                ctrl[0] &= ~btn;
                ctrl[1] &= ~btn;
            }
            break;
        }
    }
    
    return 0;
}
