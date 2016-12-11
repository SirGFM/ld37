/**
 * @file include/ld37/hook.c
 *
 * Handle the hook and flinging the player
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <conf/type.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmQuadtree.h>
#include <ld37/hook.h>
#include <ld37/player.h>
#include <math.h>
#include <stdint.h>

/* == Global stuff ========================================================== */

enum enHookFlags {
    HF_ACTIVE   = 0x01
  , HF_THROW    = 0x02
  , HF_GRAPPLED = 0x04
  , HF_RECOVER  = 0x08
};
typedef enum enHookFlags hookFlags;

/** The hook sprite */
gfmSprite *pHook = 0;

/** Hook's flags. Bits 8-15 are the current step stage (i.e., frame count) */
static uint32_t flags = 0;

/** Target position for the hook */
static int targetX;
static int targetY;
/** Position from which the hook was thrown */
static int sourceX;
static int sourceY;

/* == Functions ============================================================= */

/** Initialize the hook */
err initHook() {
    gfmRV rv;

    rv = gfmSprite_getNew(&pHook);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(pHook, 0/*x*/, 0/*y*/, HOOK_DEF_WIDTH, HOOK_DEF_HEIGHT
            , gfx.pSset4x4, HOOK_DEF_OFFX, HOOK_DEF_OFFY, 0/*child*/, T_HOOK);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setFrame(pHook, HOOK_INIT_FRAME);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Release all memory used by the hook */
void cleanHook() {
    gfmSprite_free(&pHook);
}

/** Reset the hook back to its initial state */
void resetHook() {
    flags = 0;
}

/** Update the hook, if active */
err updateHook() {
    gfmRV rv;
    int angle, hx, hy, px, py, dy, dx;

    rv = gfmSprite_getCenter(&hx, &hy, pHook);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getCenter(&px, &py, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

/* == Throw the hook (or skip update) ======================================= */

    if (!(flags & HF_ACTIVE)) {
        /* Calculate the target position for the hook */
        if (DID_JUST_PRESS(grapple)) {
            targetX = px;
            targetY = py;
            sourceX = px;
            sourceY = py;

            if (IS_PRESSED(right) && IS_PRESSED(up)) {
                targetY -= MAX_HOOK_DIST * M_SQRT1_2;
                targetX += MAX_HOOK_DIST * M_SQRT1_2;
            }
            else if (IS_PRESSED(right) && IS_PRESSED(down)) {
                targetY += MAX_HOOK_DIST * M_SQRT1_2;
                targetX += MAX_HOOK_DIST * M_SQRT1_2;
            }
            else if (IS_PRESSED(left) && IS_PRESSED(down)) {
                targetY += MAX_HOOK_DIST * M_SQRT1_2;
                targetX -= MAX_HOOK_DIST * M_SQRT1_2;
            }
            else if (IS_PRESSED(left) && IS_PRESSED(up)) {
                targetY -= MAX_HOOK_DIST * M_SQRT1_2;
                targetX -= MAX_HOOK_DIST * M_SQRT1_2;
            }
            else if (IS_PRESSED(up)) {
                targetY -= MAX_HOOK_DIST;
            }
            else if (IS_PRESSED(right)) {
                targetX += MAX_HOOK_DIST;
            }
            else if (IS_PRESSED(down)) {
                targetY += MAX_HOOK_DIST;
            }
            else if (IS_PRESSED(left)) {
                targetX -= MAX_HOOK_DIST;
            }
            else {
                return ERR_OK;
            }

            /* Clean every other flag */
            flags = HF_ACTIVE | HF_THROW;
        } /* DID_JUST_PRESS(grapple) */
        else {
            return ERR_OK;
        }
    } /* !(flags & HF_ACTIVE) */
    else {
        /* On release, retrieve the hook */
        if (IS_RELEASED(grapple)) {
            flags &= ~(HF_THROW | HF_GRAPPLED);
            flags |= HF_RECOVER;
        }
    }

/* == Update hook's position ================================================ */

    if (flags & HF_THROW) {
        double dx, dy;
        uint32_t time;
        int x, y;

        /* Retrieve the time from the flags */
        time = (flags >> 8) & 0xff;

        /* Step the hook in a frame-by-frame way... >__< */
        dx = (double)(targetX - sourceX) / (double)HOOK_TIME;
        dy = (double)(targetY - sourceY) / (double)HOOK_TIME;

        /* Calculate & set the new position */
        x = sourceX + dx * time;
        y = sourceY + dy * time;
        rv = gfmSprite_setCenter(pHook, x, y);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Increase the time and store it back */
        time++;
        if (time > 0xff) {
            time = 0xff;
        }
        flags = flags & ~0xff00;
        flags |= time << 8;

        /* Check if either the time overflown or the maximum distance was
         * reached */
        x = x - px;
        x = x * x;
        y = y - py;
        y = y * y;

        if (time >= HOOK_TIME || x + y >= MAX_HOOK_DIST * MAX_HOOK_DIST) {
            flags &= ~HF_THROW;
            flags |= HF_RECOVER;

            /* Update the targeted position */
            rv = gfmSprite_getCenter(&targetX, &targetY, pHook);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        }
    }

    if (flags & HF_GRAPPLED) {
        /* TODO */
    }
    else {
        /* TODO Collide with the world to check if did grapple */
    }

    if (flags & HF_RECOVER) {
        double dx, dy;
        uint32_t time;
        int x, y;

        /* Retrieve the time from the flags */
        time = (flags >> 8) & 0xff;

        /* Step the hook in a frame-by-frame way... >__< */
        dx = (double)(targetX - px) / (double)HOOK_TIME;
        dy = (double)(targetY - py) / (double)HOOK_TIME;

        /* Calculate & set the new position */
        x = px + dx * time;
        y = py + dy * time;
        rv = gfmSprite_setCenter(pHook, x, y);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Decrease the time and store it back */
        time--;
        flags = flags & ~0xff00;
        flags |= time << 8;

        if (time <= 0) {
            flags &= ~(HF_ACTIVE | HF_RECOVER);
            return ERR_OK;
        }
    }

/* == Update hook's frame/angle ============================================= */

    /* Calculate the angle between the player and the hook (and set the sprite
     * accordingly) */
    dy = py - hy;
    dx = px - hx;
    if (dx == 0 && dy < 0) {
        /* Corner case: straight bellow */
        angle = 180;
    }
    else if (dx == 0 && dy > 0) {
        /* Corner case: straight above */
        angle = 0;
    }
    else if (dx == 0) {
        /* Corner case: centered on player */
        angle = 0;
    }
    else {
        double tg;

        tg = (double)(dy) / (double)(dx);
        angle = (int)(atan(tg) * 180.0 / M_PI);
        angle += 90;
        if (dx > 0) {
            /* Hook to the player's left */
            angle += 180;
        }
    }

    rv = gfmSprite_setFrame(pHook, HOOK_INIT_FRAME + angle / 45);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Draw the hook, if active */
err drawHook() {
    gfmRV rv;
    float dx, dy;
    int i, tgtX, tgtY, plX, plY;

    gfmDebug_printf(game.pCtx, 0, 100
            , " ACTV:   %01i\n"
              " THRW:   %01i\n"
              " GRAP:   %01i\n"
              " RECV:   %01i\n"
              " TIME: %04i\n"
              "SRC_X:   %02i\n"
              "SRC_Y:   %02i\n"
              "TGT_X:   %02i\n"
              "TGT_Y:   %02i\n"
            , (flags & HF_ACTIVE)
            , (flags & HF_THROW)
            , (flags & HF_GRAPPLED)
            , (flags & HF_RECOVER)
            , (flags >> 8) & 0xff
            , sourceX
            , sourceY
            , targetX
            , targetY);

    if (!(flags & HF_ACTIVE)) {
        return ERR_OK;
    }

/* == Draw the chain ======================================================== */

    rv = gfmSprite_getCenter(&tgtX, &tgtY, pHook);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getCenter(&plX, &plY, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    dx = (tgtX - plX) / (double)CHAIN_LEN;
    dy = (tgtY - plY) / (double)CHAIN_LEN;

    i = 1;
    while (i < CHAIN_LEN) {
        int x, y;

        x = plX + dx * i - 1;
        y = plY + dy * i - 1;
        rv = gfm_drawTile(game.pCtx, gfx.pSset4x4, x, y, CHAIN_FRAME, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        i++;
    }

/* == Draw the hook ========================================================= */

    rv = gfmSprite_draw(pHook, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}
