/**
 * @file include/ld37/player.c
 *
 * Handle the player
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
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
/** Whether the player jumped this frame */
static int didJustJump;

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

    rv = gfmSprite_setVerticalAcceleration(pPlayer, PLAYER_GRAV);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

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

/** Collide the player against the world */
void collidePlayer() {
    gfmRV rv;
    rv = gfmQuadtree_collideSprite(collision.pStaticQt, pPlayer);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        doCollide(collision.pStaticQt);
    }
    rv = gfmQuadtree_collideSprite(collision.pQt, pPlayer);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        doCollide(collision.pQt);
    }
}

/** Handle input, update the player's physics and collide it */
err preUpdatePlayer() {
    gfmRV rv;
    gfmCollision dir;

    rv = gfmSprite_getCollision(&dir, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

/* == Move ================================================================== */
    if (IS_PRESSED(left)) {
        gfmSprite_setDirection(pPlayer, 1);
        rv = gfmSprite_setHorizontalVelocity(pPlayer, -PLAYER_SPEED);
    }
    else if (IS_PRESSED(right)) {
        gfmSprite_setDirection(pPlayer, 0);
        rv = gfmSprite_setHorizontalVelocity(pPlayer, PLAYER_SPEED);
    }
    else {
        rv = gfmSprite_setHorizontalVelocity(pPlayer, 0);
    }
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

/* == Jump ================================================================== */
    didJustJump = 0;
    if ((dir & gfmCollision_down) && DID_JUST_PRESS(jump)) {
        rv = gfmSprite_setVerticalVelocity(pPlayer, PLAYER_JUMP);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        didJustJump = 1;
    }

/* == Physics update + collision ============================================ */

    rv = gfmSprite_update(pPlayer, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    collidePlayer();

    return ERR_OK;
}

/** Adjust everything after all updates. Mostly used to set the animation */
err postUpdatePlayer() {
    double vx;
    gfmRV rv;
    gfmCollision dir;
    err erv;

    rv = gfmSprite_getCollision(&dir, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getHorizontalVelocity(&vx, pPlayer);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (didJustJump) {
        erv =  _playAnimation(PL_JUMP, 1/*force*/);
        ASSERT(erv == ERR_OK, erv);
    }
    else if (!(dir & gfmCollision_down)) {
        /* Filter playing animations while airborne */
    }
    else if (vx != 0) {
        erv =  _playAnimation(PL_WALK, 0/*force*/);
        ASSERT(erv == ERR_OK, erv);
    }
    else {
        erv =  _playAnimation(PL_STAND, 0/*force*/);
        ASSERT(erv == ERR_OK, erv);
    }

    return ERR_OK;
}

/** Draw the player */
err drawPlayer() {
    gfmRV rv;
    rv = gfmSprite_draw(pPlayer, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    return ERR_OK;
}

