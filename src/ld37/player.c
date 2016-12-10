/**
 * @file include/ld37/player.c
 *
 * Handle the player
 */
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/type.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmQuadtree.h>
#include <ld37/player.h>

/* == Player's animation ==================================================== */

enum enPlayerAnim {
    PL_STAND = 0
  , PL_WALK
  , PL_JUMP
  , PL_THROW
  , PL_ANIM_COUNT
};
typedef enum enPlayerAnim playerAnim;

static int _playerAnimData[] = {
             /* len | fps | loop | frames */
/* PL_STAND */   1  ,  0  ,  0   , 112
/* PL_WALK  */,  4  ,  6  ,  1   , 113,112,114,112
/* PL_JUMP  */,  2  ,  60 ,  0   , 115,116
/* PL_THROW */,  1  ,  0  ,  0   , 117
};

/* == Global stuff ========================================================== */

/** The player's sprite */
gfmSprite *pPlayer = 0;

/** The player's current animation */
static playerAnim curAnim;

/* == Functions ============================================================= */

/**
 * Play a new animation
 *
 * @param  [ in]anim  The animation to be played
 * @param  [ in]force Whether the animation should be forcefully reset
 */
static err _playAnimation(int anim, int force) {
    gfmRV rv;

    if (curAnim == anim && !force) {
        return ERR_OK;
    }

    rv = gfmSprite_playAnimation(pPlayer, anim);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_resetAnimation(pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    curAnim = anim;
    return ERR_OK;
}

/** Initialize the player */
err initPlayer() {
    gfmRV rv;
    err erv;

    rv = gfmSprite_getNew(&pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(pPlayer, 0/*x*/, 0/*y*/, PLAYER_DEF_WIDTH
            , PLAYER_DEF_HEIGHT, gfx.pSset8x8, PLAYER_DEF_OFFX, PLAYER_DEF_OFFY
            , 0/*child*/, T_PLAYER);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(pPlayer, _playerAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = _playAnimation(PL_STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** Release all resources associated with the player */
void cleanPlayer() {
    gfmSprite_free(&pPlayer);
}

/**
 * Load the player as defined by the parser
 *
 * @param  [ in]pParser The parser
 */
err loadPlayer(gfmParser *pParser) {
    gfmRV rv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(pPlayer, x - PLAYER_DEF_OFFX
            , y - PLAYER_DEF_HEIGHT);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Handle input, update the player's physics and collide it */
err preUpdatePlayer() {
    gfmRV rv;

    rv = gfmSprite_update(pPlayer, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Adjust everything after all updates. Mostly used to set the animation */
err postUpdatePlayer() {
    return ERR_OK;
}

/** Draw the player */
err drawPlayer() {
    gfmRV rv;
    rv = gfmSprite_draw(pPlayer, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    return ERR_OK;
}

