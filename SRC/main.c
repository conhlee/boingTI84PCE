/*
    *   Original C 'boing' reconstruction by Jimmy Maher in 2009-2010
    *   TI-84 Plus CE-T port by conhlee in 2025
*/

#include <graphx.h>
#include <ti/getcsc.h>

#include <stdint.h>

#include "BALL.IND.inc"
#include "BALL.PAL.inc"

#define BALL_WIDTH (144)
#define BALL_HEIGHT (100)

// The frame rate is a little slow, so we ramp up the speed to 2x to account for it.
#define ANIME_SPEED (2)

typedef struct {
    short posX, posY;
    short scrollX, scrollY;
    short colorCycle;

    union {
        gfx_sprite_t spr;
        uint8_t sprData[sizeof(gfx_sprite_t) + (BALL_WIDTH * BALL_HEIGHT)];
    };
} BallState;
static BallState sBall;

static inline void ballInit(void) {
    sBall.spr.width = BALL_WIDTH;
    sBall.spr.height = BALL_HEIGHT;

    int pixelCount = BALL_WIDTH * BALL_HEIGHT;
    for (int i = 0; i < pixelCount; i++) {
        sBall.spr.data[i] = BALL_IND[i];
    }

    sBall.posX = 0;
    sBall.posY = 0;
    sBall.scrollX = 1;
    sBall.scrollY = -1;
    sBall.colorCycle = 2;
}

static inline void ballUpdate(void) {
    // Offset by one to skip transparent color at index 0.
    uint16_t *palette = ((uint16_t *)BALL_PAL) + 1;

    sBall.colorCycle += (sBall.scrollX > 0) ? -1 : 1;

    if (sBall.colorCycle < 0)
        sBall.colorCycle = 13;
    else if (sBall.colorCycle > 13)
        sBall.colorCycle = 0;

    for (int i = 0; i < 7; i++) {
        if ((sBall.colorCycle + i) < 14) {
            palette[sBall.colorCycle + i + 2] = gfx_RGBTo1555(255, 255, 255);
            palette[sBall.colorCycle + i + 18] = gfx_RGBTo1555(255, 255, 255);
        }
        else {
            palette[sBall.colorCycle + i - 12] = gfx_RGBTo1555(255, 255, 255);
            palette[sBall.colorCycle + i + 4] = gfx_RGBTo1555(255, 255, 255);
        }
    }
    for (int i = 7; i < 14; i++) {
        if ((sBall.colorCycle + i) < 14) {
            palette[sBall.colorCycle + i + 2] = gfx_RGBTo1555(255, 0, 0);
            palette[sBall.colorCycle + i + 18] = gfx_RGBTo1555(255, 0, 0);
        }
        else {
            palette[sBall.colorCycle + i - 12] = gfx_RGBTo1555(255, 0, 0);
            palette[sBall.colorCycle + i + 4] = gfx_RGBTo1555(255, 0, 0);
        }
    }

    if (sBall.scrollX > 0) {
        palette[sBall.colorCycle + 2] = gfx_RGBTo1555(255, 221, 221);
        palette[sBall.colorCycle + 18] = gfx_RGBTo1555(255, 221, 221);
    }
    else {
        if ((sBall.colorCycle + 6) < 14) {
            palette[sBall.colorCycle + 8] = gfx_RGBTo1555(255, 221, 221);
            palette[sBall.colorCycle + 24] = gfx_RGBTo1555(255, 221, 221);
        }
        else {
            palette[sBall.colorCycle - 6] = gfx_RGBTo1555(255, 221, 221);
            palette[sBall.colorCycle + 10] = gfx_RGBTo1555(255, 221, 221);
        }
    }

    sBall.posX += sBall.scrollX * ANIME_SPEED;
    if (sBall.posX <= -55 || sBall.posX >= 100)
        sBall.scrollX = -sBall.scrollX;

    short scrollYFact;
    if (sBall.posY > -10)
        scrollYFact = 1;
    else if (sBall.posY > -30)
        scrollYFact = 2;
    else if (sBall.posY > -60)
        scrollYFact = 3;
    else
        scrollYFact = 4;

    sBall.posY += sBall.scrollY * scrollYFact * ANIME_SPEED;

    if (sBall.posY <= -100 || sBall.posY >= 0)
        sBall.scrollY = -sBall.scrollY;
}

static inline void ballDraw(void) {
    int x = (BALL_WIDTH / 2) + sBall.posX;
    int y = (BALL_HEIGHT / 2) - sBall.posY - 20;

    gfx_TransparentSprite(&sBall.spr, x, y);
}

static inline void bgDraw(void) {
    // Draw the gray BG
    gfx_FillScreen(1);

    // Assign the purple color ..
    gfx_SetColor(17);

    /*
        TODO: is it possible to buffer these lines out to a texture for a speed boost?
        I'm fairly sure drawing all these lines is a little slow ..
    */

// Scale AMIGA coords to our screen coords.
#define SCALE_X(x) ((int)((int)(x) * (GFX_LCD_WIDTH) / 336))
#define SCALE_Y(y) ((int)((int)(y) * (GFX_LCD_HEIGHT) / 216))

    for (int j = 48; j < 300; j += 16) {
        gfx_VertLine_NoClip(SCALE_X(j), SCALE_Y(0), SCALE_Y(192));
    }

    for (int j = 0; j < 200; j += 16) {
        gfx_HorizLine_NoClip(SCALE_X(48), SCALE_Y(j), SCALE_X(288) - SCALE_X(48));
    }

    for (int j = 48, k = 20; j < 300; j += 16, k += 20) {
        gfx_Line_NoClip(
            SCALE_X(j), SCALE_Y(192),
            SCALE_X(k), SCALE_Y(215)
        );
    }

    gfx_HorizLine_NoClip(SCALE_X(45), SCALE_Y(194), SCALE_X(291) - SCALE_X(45) + 1);

    gfx_HorizLine_NoClip(SCALE_X(41), SCALE_Y(197), SCALE_X(295) - SCALE_X(41) + 1);

    gfx_HorizLine_NoClip(SCALE_X(37), SCALE_Y(201), SCALE_X(300) - SCALE_X(37) + 1);

    gfx_HorizLine_NoClip(SCALE_X(30), SCALE_Y(207), SCALE_X(308) - SCALE_X(30) + 1);

    gfx_HorizLine_NoClip(SCALE_X(20), SCALE_Y(215), SCALE_X(319) - SCALE_X(20) + 1);

// Don't need these anymore!
#undef SCALE_X
#undef SCALE_Y

}

static inline void syncPalette(void) {
    gfx_SetPalette(BALL_PAL, BALL_PAL_len, 0);
    gfx_SetTransparentColor(0);
}

int main(void) {
    ballInit();

    gfx_Begin();
    gfx_SetDrawBuffer();

    do {
        ballUpdate();

        syncPalette();

        bgDraw();
        ballDraw();

        gfx_SwapDraw();
    } while (os_GetCSC() == 0);

    gfx_End();

    return 0;
}
