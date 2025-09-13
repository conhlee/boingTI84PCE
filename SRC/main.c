/*
    *   Original 'boing' demo by Dale Luck and R.J. Mical in 1984-1985
    *   C 'boing' reconstruction by Jimmy Maher in 2009-2010
    *   TI-84 Plus CE-T port by conhlee in 2025
*/

#include <graphx.h>
#include <ti/getcsc.h>

#include <stdint.h>

#include "BALL.IND.inc"
#include "BALL.PAL.inc"

/* Fixed-point macros. */
#define FIX_SHIFT   (5)
#define FIX_SCALE   (1 << FIX_SHIFT)

#define TO_FIXED(x)     ((x) * FIX_SCALE)
#define FROM_FIXED(x)   ((x) / FIX_SCALE)

#define FIXED_MULT(x, y) ((x) * (y) / FIX_SCALE)

#define IS_FIXED_WHOLE(x) (((x) & (FIX_SCALE - 1)) == 0)

/* Ball image dimensions. */
#define BALL_WIDTH (144)
#define BALL_HEIGHT (100)

// The frame rate is a little slow, so we ramp up the base speed to 2x to account for it.
#define BALL_BASE_SPEED (2)

typedef struct {
    // All of these values are fixed-point.
    int posX, posY;
    int scrollX, scrollY;
    int colorCycle;
    int animSpeed;

    union {
        gfx_sprite_t spr;
        uint8_t sprData[sizeof(gfx_sprite_t) + (BALL_WIDTH * BALL_HEIGHT)];
    };
} BallState;
static BallState sBall;

static inline void gfxInit(void) {
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(BALL_PAL, BALL_PAL_len, 0);
    gfx_SetTransparentColor(0);
}

static inline void gfxShutdown(void) {
    gfx_End();
}

static inline void ballInit(void) {
    sBall.spr.width = BALL_WIDTH;
    sBall.spr.height = BALL_HEIGHT;

    int pixelCount = BALL_WIDTH * BALL_HEIGHT;
    for (int i = 0; i < pixelCount; i++) {
        sBall.spr.data[i] = BALL_IND[i];
    }

    sBall.posX = TO_FIXED(0);
    sBall.posY = TO_FIXED(0);
    sBall.scrollX = TO_FIXED(1);
    sBall.scrollY = TO_FIXED(-1);
    sBall.colorCycle = TO_FIXED(2);

    sBall.animSpeed = TO_FIXED(1);
}

static inline void ballUpdate(void) {
    // Writing directly to gfx_palette .. kind of a dirty hack..
    uint16_t *palette = (uint16_t *)(gfx_palette) + 1;

    sBall.colorCycle += FIXED_MULT(
        TO_FIXED((sBall.scrollX > 0) ? -1 : 1),
        sBall.animSpeed * BALL_BASE_SPEED
    );

    if (sBall.colorCycle < 0)
        sBall.colorCycle = TO_FIXED(13);
    else if (sBall.colorCycle > TO_FIXED(13))
        sBall.colorCycle = 0;

    short colorCycleInt = FROM_FIXED(sBall.colorCycle);

    for (int i = 0; i < 7; i++) {
        if ((colorCycleInt + i) < 14) {
            palette[colorCycleInt + i + 2] = gfx_RGBTo1555(255, 255, 255);
            palette[colorCycleInt + i + 18] = gfx_RGBTo1555(255, 255, 255);
        }
        else {
            palette[colorCycleInt + i - 12] = gfx_RGBTo1555(255, 255, 255);
            palette[colorCycleInt + i + 4] = gfx_RGBTo1555(255, 255, 255);
        }
    }
    for (int i = 7; i < 14; i++) {
        if ((colorCycleInt + i) < 14) {
            palette[colorCycleInt + i + 2] = gfx_RGBTo1555(255, 0, 0);
            palette[colorCycleInt + i + 18] = gfx_RGBTo1555(255, 0, 0);
        }
        else {
            palette[colorCycleInt + i - 12] = gfx_RGBTo1555(255, 0, 0);
            palette[colorCycleInt + i + 4] = gfx_RGBTo1555(255, 0, 0);
        }
    }

    if (sBall.scrollX > 0) {
        palette[colorCycleInt + 2] = gfx_RGBTo1555(255, 221, 221);
        palette[colorCycleInt + 18] = gfx_RGBTo1555(255, 221, 221);
    }
    else {
        if ((colorCycleInt + 6) < 14) {
            palette[colorCycleInt + 8] = gfx_RGBTo1555(255, 221, 221);
            palette[colorCycleInt + 24] = gfx_RGBTo1555(255, 221, 221);
        }
        else {
            palette[colorCycleInt - 6] = gfx_RGBTo1555(255, 221, 221);
            palette[colorCycleInt + 10] = gfx_RGBTo1555(255, 221, 221);
        }
    }

    sBall.posX += FIXED_MULT(sBall.scrollX, sBall.animSpeed * BALL_BASE_SPEED);
    if (sBall.posX <= TO_FIXED(-55)) {
        sBall.scrollX = -sBall.scrollX;
        sBall.posX = TO_FIXED(-55 + 1);
    }
    if (sBall.posX >= TO_FIXED(100)) {
        sBall.scrollX = -sBall.scrollX;
        sBall.posX = TO_FIXED(100 - 1);
    }

    int scrollYFact;
    if (sBall.posY > TO_FIXED(-10))
        scrollYFact = 1;
    else if (sBall.posY > TO_FIXED(-30))
        scrollYFact = 2;
    else if (sBall.posY > TO_FIXED(-60))
        scrollYFact = 3;
    else
        scrollYFact = 4;

    sBall.posY += FIXED_MULT(sBall.scrollY * scrollYFact, sBall.animSpeed * BALL_BASE_SPEED);

    if (sBall.posY <= TO_FIXED(-100)) {
        sBall.scrollY = -sBall.scrollY;
        sBall.posY = TO_FIXED(-100 + 1);
    }
    if (sBall.posY >= TO_FIXED(0)) {
        sBall.scrollY = -sBall.scrollY;
        sBall.posY = TO_FIXED(0 - 1);
    }
}

static inline void ballDraw(void) {
    int x = (BALL_WIDTH / 2) + FROM_FIXED(sBall.posX);
    int y = (BALL_HEIGHT / 2) - FROM_FIXED(sBall.posY) - 20;

    gfx_TransparentSprite(&sBall.spr, x, y);
}

static inline void bgDraw(void) {
    // Draw the gray BG.
    gfx_FillScreen(1);

    // Assign the purple color ..
    gfx_SetColor(17);

// Scale AMIGA coords to our screen coords.
// All of the loops below are unrolled, so this math is never done in realtime; neat!
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

static unsigned int sSpeedMesgTimer;
static unsigned int sExitMesgTimer;

// We keep these times low .. displaying text is pretty slow!
#define SHOW_SPEED_MESG() do { sSpeedMesgTimer = 20; } while (0)
#define SHOW_EXIT_MESG() do { sExitMesgTimer = 20; } while (0)

static inline void mesgInit(void) {
    gfx_SetTextFGColor(17);
    gfx_SetTextBGColor(1);
    gfx_SetTextTransparentColor(0);

    gfx_SetTextConfig(gfx_text_noclip);
    gfx_SetMonospaceFont(8);

    SHOW_SPEED_MESG();
    SHOW_EXIT_MESG();
}

static inline void mesgDraw(void) {
    int showSpeedMesg = sSpeedMesgTimer > 0;
    if (showSpeedMesg) {
        gfx_SetTextXY(10, 10);
        
        gfx_PrintString("SPEED : ");

        int animSpeedFix = sBall.animSpeed;
        int animSpeedInt = FROM_FIXED(animSpeedFix);

        if (IS_FIXED_WHOLE(animSpeedFix)) {
            // Hooray! No slow math for us!
            gfx_PrintInt(animSpeedInt, 1);
            gfx_PrintString(".000");
        }
        else {
            int fracPart = ((animSpeedFix & (FIX_SCALE - 1)) * 1000) / TO_FIXED(1);

            if (animSpeedFix < 0 && animSpeedInt == 0) {
                gfx_PrintChar('-'); // We have to manually add the - in this case.
            }
            gfx_PrintInt(animSpeedInt, 1);
            gfx_PrintChar('.');
            gfx_PrintInt(fracPart, 3);
        }

        gfx_PrintString(" ( use arrow keys )");

        sSpeedMesgTimer--;
    }

    if (sExitMesgTimer > 0) {
        gfx_PrintStringXY("press MODE to exit", 10, showSpeedMesg ? 20 : 10);

        sExitMesgTimer--;
    }
}

static inline uint8_t keyUpdate(void) {
    uint8_t keyCode = os_GetCSC();

    switch (keyCode) {
    case 0:
    case sk_Mode:
        break;

    case sk_Left:
        sBall.animSpeed -= TO_FIXED(1) / 8;
        SHOW_SPEED_MESG();
        break;
    case sk_Right:
        sBall.animSpeed += TO_FIXED(1) / 8;
        SHOW_SPEED_MESG();
        break;
    case sk_Up:
        sBall.animSpeed += TO_FIXED(1);
        SHOW_SPEED_MESG();
        break;
    case sk_Down:
        sBall.animSpeed -= TO_FIXED(1);
        SHOW_SPEED_MESG();
        break;

    default:
        SHOW_EXIT_MESG();
        break;
    }

    return keyCode;
}

int main(void) {
    gfxInit();

    ballInit();

    mesgInit();

    for (;;) {
        uint8_t keyCode = keyUpdate();

        ballUpdate();

        bgDraw();
        ballDraw();

        mesgDraw();

        gfx_SwapDraw();

        if (keyCode == sk_Mode) {
            break;
        }
    }

    gfxShutdown();

    return 0;
}
