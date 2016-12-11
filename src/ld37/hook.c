/**
 * @file include/ld37/hook.c
 *
 * Handle the hook and flinging the player
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/type.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmQuadtree.h>
#include <ld37/hook.h>
#include <ld37/player.h>
#include <math.h>

/* == Global stuff ========================================================== */

/** The hook sprite */
gfmSprite *pHook = 0;

/** Number of parts in the hook's chain */
static const int chainLen = 32;

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
}

/** Update the hook, if active */
err updateHook() {
    gfmInput *pInput;
    gfmRV rv;
    int angle, x, y, px, py, dy, dx;

    rv = gfm_getInput(&pInput, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmInput_getPointerPosition(&x, &y, pInput);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setCenter(pHook, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_getCenter(&px, &py, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Calculate the angle between the player and the hook (and set the sprite
     * accordingly) */
    dy = py - y;
    dx = px - x;
    if (dx == 0 && dy < 0) {
        /* Corner case, straight bellow */
        angle = 180;
    }
    else if (dx == 0 && dy > 0) {
        /* Corner case, straight above */
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

/* == Draw the chain ======================================================== */
    rv = gfmSprite_getCenter(&tgtX, &tgtY, pHook);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getCenter(&plX, &plY, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    dx = (tgtX - plX) / (double)chainLen;
    dy = (tgtY - plY) / (double)chainLen;

    i = 1;
    while (i < chainLen) {
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

