/**
 * @file include/ld37/hook.c
 *
 * Handle the hook and flinging the player
 */
#include <base/collision.h>
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
#include <stdlib.h>

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

/**
 * Bits  0-7:  Hook's flags.
 * Bits  8-15: Current step stage (i.e., frame count)
 * Bits 16-23: Current maximum distance
 */
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
            , HOOK_SPRITESET, HOOK_DEF_OFFX, HOOK_DEF_OFFY, 0/*child*/, T_HOOK);
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

/**
 * Retrieve the current angle between the player and the hook
 *
 * NOTE: 0 points upward, M_PI_2 points to the right
 *
 * @param  [ in]px The player's position
 * @param  [ in]py The player's position
 * @param  [ in]hx The hook's position
 * @param  [ in]hy The hook's position
 */
static double _getCurrentAngle(int px, int py, int hx, int hy) {
    int dx, dy;

    /* Calculate the angle between the player and the hook (and set the
     * sprite accordingly) */
    dy = py - hy;
    dx = px - hx;
    if (dx == 0 && dy < 0) {
        /* Corner case: straight bellow */
        return M_PI;
    }
    else if (dx == 0 && dy > 0) {
        /* Corner case: straight above */
        return 0;
    }
    else if (dx == 0) {
        /* Corner case: centered on player */
        return 0;
    }
    else {
        double angle, tg;

        tg = (double)(dy) / (double)(dx);
        angle = atan(tg);
        angle += M_PI_2;
        if (dx > 0) {
            /* Hook to the player's left */
            angle += M_PI;
        }
        return angle;
    }
}

/** Retrieve the current distance between the hook and the player */
static uint32_t _getCurrentDistance() {
    int x, y, px, py;

    gfmSprite_getCenter(&x, &y, pHook);
    gfmSprite_getCenter(&px, &py, pPlayer);

    x = x - px;
    x = x * x;
    y = y - py;
    y = y * y;

    return (uint32_t)sqrt((float)(x + y));
}

/** Called whenever the hook grapple onto something */
void onGrapple() {
    flags &= ~(HF_THROW | HF_RECOVER);
    flags |= HF_GRAPPLED;

    /* Mostly for debug asserting... */
    ASSERT_TO(MAX_HOOK_DIST <= 0xff, NOOP(), __cont);
__cont:
    flags &= ~0xff0000;
    flags |= MAX_HOOK_DIST << 16;
}

/** Update the hook, if active */
err updateHook() {
    gfmRV rv;
    int hx, hy, px, py;

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
        uint32_t dist, maxDist;
        gfmCollision dir;

        /* Update the maximum distance */
        maxDist = (flags >> 16) & 0xff;
        if (IS_PRESSED(up) && maxDist > 0) {
            maxDist--;
        }
        else if (IS_PRESSED(down) && maxDist < MAX_HOOK_DIST) {
            maxDist++;
        }
        flags &= ~0xff0000;
        flags |= maxDist << 16;

        rv = gfmSprite_getCollision(&dir, pPlayer);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        dist = _getCurrentDistance();

        if (dir & gfmCollision_down) {
            if (dist >= maxDist) {
                int nx, ny, x, y;

                x = px - hx;
                y = py - hy;
                nx = x * (double)maxDist / (double)dist;
                ny = y * (double)maxDist / (double)dist;

                /* This looks fun and avoid collision bugs... but isn't correct
                 * (i.e., the player may pull itself farther than allowed) */
                if (nx - x > HOOK_GROUNDED_FORCE) {
                    nx = x + HOOK_GROUNDED_FORCE;
                }
                else if (nx - x < -HOOK_GROUNDED_FORCE) {
                    nx = x - HOOK_GROUNDED_FORCE;
                }

                if (ny - y > HOOK_GROUNDED_FORCE) {
                    ny = y + HOOK_GROUNDED_FORCE;
                }
                else if (ny - y < -HOOK_GROUNDED_FORCE) {
                    ny = y - HOOK_GROUNDED_FORCE;
                }

                x = nx + hx;
                y = ny + hy;

                rv = gfmSprite_setCenter(pPlayer, x, y);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);

                collidePlayer();
            } /* dist >= maxDist */
        } /* dir & gfmCollision_down */
        else {
            double angle, ax, ay;

            angle = 2 * M_PI - _getCurrentAngle(px, py, hx, hy);
            angle += M_PI * 6 / 4;

            ax = -cos(angle) * PLAYER_GRAV;
            ay = sin(angle) * PLAYER_GRAV;

            rv = gfmSprite_setAcceleration(pPlayer, ax, ay);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        }
    } /* flags & HF_GRAPPLED */
    else if (!(flags & HF_RECOVER)) {
        /* Collide with the world to check if did grapple */
        rv = gfmQuadtree_collideSprite(collision.pStaticQt, pHook);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            doCollide(collision.pStaticQt);
        }
        rv = gfmQuadtree_collideSprite(collision.pQt, pHook);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            doCollide(collision.pQt);
        }
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

    if (!(flags & HF_GRAPPLED)) {
        int angle = (int)(_getCurrentAngle(px, py, hx, hy) * 180.0 / M_PI);
        rv = gfmSprite_setFrame(pHook, HOOK_INIT_FRAME + angle / 45);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    } /* !(flags & HF_GRAPPLED) */

    return ERR_OK;
}

/** Draw the hook, if active */
err drawHook() {
    gfmRV rv;
    float dx, dy;
    int i, tgtX, tgtY, plX, plY;

    gfmDebug_printf(game.pCtx, 0, 60
            , " ACTV:   %01i\n"
              " THRW:   %01i\n"
              " GRAP:   %01i\n"
              " RECV:   %01i\n"
              " TIME:%04i\n"
              " DIST: %03i\n"
            , (flags & HF_ACTIVE)
            , (flags & HF_THROW)
            , (flags & HF_GRAPPLED)
            , (flags & HF_RECOVER)
            , (flags >> 8) & 0xff
            , (flags >> 16) & 0xff);

    gfmDebug_printf(game.pCtx, 0, 116
            , "SRC_X:  %02i\n"
              "SRC_Y:  %02i\n"
              "TGT_X:  %02i\n"
              "TGT_Y:  %02i\n"
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

        x = plX + dx * i - 2;
        y = plY + dy * i - 2;
        rv = gfm_drawTile(game.pCtx, HOOK_SPRITESET, x, y, CHAIN_FRAME, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        i++;
    }

/* == Draw the hook ========================================================= */

    rv = gfmSprite_draw(pHook, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Whether the hook did grapple onto something */
int isGrappled() {
    return (flags & HF_GRAPPLED);
}

