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

gfx_UninitedSprite(sBallSpr, BALL_WIDTH, BALL_HEIGHT);

static inline void ballInit(void) {
    sBallSpr->width = BALL_WIDTH;
    sBallSpr->height = BALL_HEIGHT;

    int pixelCount = BALL_WIDTH * BALL_HEIGHT;
    for (int i = 0; i < pixelCount; i++) {
        sBallSpr->data[i] = BALL_IND[i];
    }
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
        gfx_Line_NoClip(
            SCALE_X(j), SCALE_Y(0),
            SCALE_X(j), SCALE_Y(192)
        );
    }

    for (int j = 0; j < 200; j += 16) {
        gfx_Line_NoClip(
            SCALE_X(48), SCALE_Y(j),
            SCALE_X(288), SCALE_Y(j)
        );
    }

    for (int j = 48, k = 20; j < 300; j += 16, k += 20) {
        gfx_Line_NoClip(
            SCALE_X(j), SCALE_Y(192),
            SCALE_X(k), SCALE_Y(215)
        );
    }

    gfx_Line_NoClip(
        SCALE_X(45), SCALE_Y(194),
        SCALE_X(291), SCALE_Y(194)
    );

    gfx_Line_NoClip(
        SCALE_X(41), SCALE_Y(197),
        SCALE_X(295), SCALE_Y(197)
    );

    gfx_Line_NoClip(
        SCALE_X(37), SCALE_Y(201),
        SCALE_X(300), SCALE_Y(201)
    );

    gfx_Line_NoClip(
        SCALE_X(30), SCALE_Y(207),
        SCALE_X(308), SCALE_Y(207)
    );

    gfx_Line_NoClip(
        SCALE_X(20), SCALE_Y(215),
        SCALE_X(319), SCALE_Y(215)
    );

// Don't need these anymore!
#undef SCALE_X
#undef SCALE_Y

}

static inline void ballDraw(int x, int y) {
    int trueX = (BALL_WIDTH / 2) + x;
    int trueY = (BALL_HEIGHT / 2) - y - 20;

    gfx_TransparentSprite(sBallSpr, trueX, trueY);
}

static inline void paletteUpdate(void) {
    gfx_SetPalette(BALL_PAL, BALL_PAL_len, 0);
    gfx_SetTransparentColor(0);
}

int main(void) {
    ballInit();

    gfx_Begin();
    gfx_SetDrawBuffer();

    int colorCycle = 2;

    int posX = 0;
    int posY = 0;
    int scrollX = 1;
    int scrollY = -1;

    // Offset by one to skip transparent color at index 0.
    uint16_t *ballColoring = ((uint16_t *)BALL_PAL) + 1;

    do {
        if (scrollX > 0)
            colorCycle--;
        else
            colorCycle++;

        if (colorCycle < 0)
            colorCycle = 13;
        else if (colorCycle > 13)
            colorCycle = 0;

        for (int i = 0; i < 7; i++) {
            if ((colorCycle + i) < 14) {
                ballColoring[colorCycle + i + 2] = gfx_RGBTo1555(255, 255, 255);
                ballColoring[colorCycle + i + 18] = gfx_RGBTo1555(255, 255, 255);
            }
            else {
                ballColoring[colorCycle + i - 12] = gfx_RGBTo1555(255, 255, 255);
                ballColoring[colorCycle + i + 4] = gfx_RGBTo1555(255, 255, 255);
            }
        }
        for (int i = 7; i < 14; i++) {
            if ((colorCycle + i) < 14) {
                ballColoring[colorCycle + i + 2] = gfx_RGBTo1555(255, 0, 0);
                ballColoring[colorCycle + i + 18] = gfx_RGBTo1555(255, 0, 0);
            }
            else {
                ballColoring[colorCycle + i - 12] = gfx_RGBTo1555(255, 0, 0);
                ballColoring[colorCycle + i + 4] = gfx_RGBTo1555(255, 0, 0);
            }
        }
    
        if (scrollX > 0) {
            ballColoring[colorCycle + 2] = gfx_RGBTo1555(255, 221, 221);
            ballColoring[colorCycle + 18] = gfx_RGBTo1555(255, 221, 221);
        }
        else {
            if ((colorCycle + 6) < 14) {
                ballColoring[colorCycle + 8] = gfx_RGBTo1555(255, 221, 221);
                ballColoring[colorCycle + 24] = gfx_RGBTo1555(255, 221, 221);
            }
            else {
                ballColoring[colorCycle - 6] = gfx_RGBTo1555(255, 221, 221);
                ballColoring[colorCycle + 10] = gfx_RGBTo1555(255, 221, 221);
            }
        }

        paletteUpdate();

        posX += scrollX * ANIME_SPEED;
        if (posX <= -55 || posX >= 95)
            scrollX = -scrollX;

        int scrollYFact;
        if (posY > -10)
            scrollYFact = 1;
        else if (posY > -30)
            scrollYFact = 2;
        else if (posY > -60)
            scrollYFact = 3;
        else
            scrollYFact = 4;

        posY += scrollY * scrollYFact * ANIME_SPEED;

        if (posY <= -100 || posY >= 0)
            scrollY = -scrollY;

        bgDraw();

        ballDraw(posX, posY);

        gfx_BlitBuffer();
    } while (!os_GetCSC());

    gfx_End();

    return 0;
}
