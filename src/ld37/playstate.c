#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <conf/game.h>
#include <conf/type.h>
#include <GFraMe/gframe.h>
#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmText.h>
#include <GFraMe/gfmTilemap.h>
#include <ld37/hook.h>
#include <ld37/level.h>
#include <ld37/player.h>
#include <ld37/playstate.h>
#include <stdint.h>
#include <string.h>

/** Current level orientation */
static levelOrientation curOrientation = 0;

static gfmText *pText = 0;

/** Group with every 'stuff' (any collectible/interactable) */
static gfmGroup *pStuff = 0;

/** Main camera */
static gfmCamera *pCamera = 0;

/** Whether we have already flipped this frame */
static uint32_t hasFlipped = 0;
static uint32_t hasWon = 0;

/**
 * Spawn something new in the stuff group
 *
 * @param  [ in]x The sprite's position
 * @param  [ in]y The sprite's position
 * @param  [ in]t The sprite's type
 * @param  [ in]f The sprite's frame
 */
gfmSprite* _spawnStuff(int x, int y, type t, int f);

err initPlaystate() {
    gfmRV rv;

    rv = gfmText_getNew(&pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfm_getCamera(&pCamera, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmGroup_getNew(&pStuff);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmGroup_setDefType(pStuff, T_STUFF);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDefSpriteset(pStuff, gfx.pSset8x8);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDefDimensions(pStuff, 6/*w*/, 6/*h*/, -1/*offx*/, -1/*offy*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDeathOnLeave(pStuff, 0/*dontDie*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDeathOnTime(pStuff, -1/*dontDie*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv =  gfmGroup_setDrawOrder(pStuff, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setCollisionQuality(pStuff
            , gfmCollisionQuality_collideEverything);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_preCache(pStuff, MAX_STUFF, MAX_STUFF);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;

    return ERR_OK;
}

err loadPlaystate() {
    gfmParser *pParser = 0;
    gfmRV rv;
    err erv;

    curOrientation = 0;

//"YOU DID IT!\n"
//"CONGRATULATIONS!!"
    rv = gfmText_init(pText, 16/*x*/, 96/*y*/, 18/*maxWidth*/, 3/*maxLines*/
            , 100/*delay*/, 0/*bindToWorld*/, gfx.pSset8x8, 0/*tile*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    hasWon = 0;

    erv = loadLevel(LO_DEFAULT);
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    rv = gfmGroup_killAll(pStuff);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmCamera_setWorldDimensions(pCamera, getLevelWidth(),
            getLevelHeight());
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
    rv = gfmCamera_setDeadzone(pCamera, 2 * V_WIDTH / 5 /*x*/
            , 2 * V_HEIGHT / 5 /*y*/, V_WIDTH / 5, V_HEIGHT / 5);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

    rv = gfmParser_getNew(&pParser);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
    rv = gfmParser_initStatic(pParser, game.pCtx, "map/map_obj.gfm");
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

    while (1) {
        char *type;
        int x, y;

        rv = gfmParser_parseNext(pParser);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        rv = gfmParser_getIngameType(&type, pParser);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
        rv = gfmParser_getPos(&x, &y, pParser);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        /* Adjust it for the default entities */
        x += 1;
        y -= 8;

        if (strcmp(type, "player") == 0) {
            erv = loadPlayer(pParser);
            ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
        }
        else if (strcmp(type, "horizontal") == 0) {
            _spawnStuff(x, y, T_HORIZONTAL, HORIZONTAL_TILE);
        }
        else if (strcmp(type, "vertical") == 0) {
            _spawnStuff(x, y, T_VERTICAL, VERTICAL_TILE);
        }
        else if (strcmp(type, "jewel") == 0) {
            _spawnStuff(x, y, T_JEWEL, JEWEL_TILE);
        }
        else {
            ASSERT_TO(0, erv = ERR_UNKNOWNTYPE, __ret);
        }
    }

    resetHook();

    erv = ERR_OK;
__ret:
    gfmParser_free(&pParser);

    return erv;
}

void cleanPlaystate() {
    gfmGroup_free(&pStuff);
    gfmText_free(&pText);
}

err updatePlaystate() {
    gfmRV rv;
    err erv;

    if (hasFlipped) {
        hasFlipped--;
    }

    rv = gfmQuadtree_initRoot(collision.pQt, -8/*x*/, -8/*y*/
            , getLevelWidth() + 16, getLevelHeight() + 16, 8/*depth*/
            , 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmGroup_update(pStuff, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmQuadtree_collideGroup(collision.pQt, pStuff);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        doCollide(collision.pQt);
    }

    erv = preUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    erv = updateHook();
    ASSERT(erv == ERR_OK, erv);

    erv = postUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    /* Center the camera */
    do {
        int x, y;
        gfmSprite_getCenter(&x, &y, pPlayer);
        gfmCamera_centerAtPoint(pCamera, x, y);
    } while (0);

    rv = gfmText_update(pText, game.pCtx);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

err drawPlaystate() {
    gfmRV rv;
    err erv;

    rv = gfmTilemap_draw(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = drawHook();
    ASSERT(erv == ERR_OK, erv);
    erv = drawPlayer();
    ASSERT(erv == ERR_OK, erv);
    rv = gfmGroup_draw(pStuff, game.pCtx);
    ASSERT(erv == ERR_OK, erv);
    rv = gfmText_draw(pText, game.pCtx);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Spawn something new in the stuff group
 *
 * @param  [ in]x The sprite's position
 * @param  [ in]y The sprite's position
 * @param  [ in]t The sprite's type
 * @param  [ in]f The sprite's frame
 */
gfmSprite* _spawnStuff(int x, int y, type t, int f) {
    gfmSprite *pSpr;
    gfmRV rv;

    rv = gfmGroup_recycle(&pSpr, pStuff);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setPosition(pSpr, x, y);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setType(pSpr, t);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setFrame(pSpr, f);
    ASSERT(rv == GFMRV_OK, 0);

    return pSpr;
}

/** Flip a sprite */
static void _flipSprite(gfmSprite *pSpr, levelOrientation spriteOrientation) {
    int x, y;
    gfmSprite_getCenter(&x, &y, pSpr);
    if ((spriteOrientation) & LO_HORIZONTAL_MIRROR) {
        x = getLevelWidth() - x;
    }
    if ((spriteOrientation) & LO_VERTICAL_MIRROR) {
        y = getLevelHeight() - y;
    }
    gfmSprite_setCenter(pSpr, x, y);
}

/** Flip the world */
static err _flipWorld(levelOrientation spriteOrientation) {
    gfmGroupNode *pList;
    gfmRV rv;
    err erv;

    erv = loadLevel(curOrientation);
    ASSERT(erv == ERR_OK, erv);
    _flipSprite(pPlayer, spriteOrientation);
    _flipSprite(pHook, spriteOrientation);

    rv = gfmGroup_update(pStuff, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_getCollideableList(&pList, pStuff);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    while (pList != 0) {
        gfmSprite *pSpr;
        rv = gfmGroup_getNextSprite(&pSpr, &pList);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        _flipSprite(pSpr, spriteOrientation);
    }

    hasFlipped = 16;

    return ERR_OK;
}

/**
 * Flip the world (and everything within) horizontally
 */
err flipHorizontal() {
    if (hasFlipped) {
        return ERR_OK;
    }
    curOrientation ^= LO_HORIZONTAL_MIRROR;
    return _flipWorld(LO_HORIZONTAL_MIRROR);
}

/**
 * Flip the world (and everything within) vertically
 */
err flipVertical() {
    if (hasFlipped) {
        return ERR_OK;
    }
    curOrientation ^= LO_VERTICAL_MIRROR;
    return _flipWorld(LO_VERTICAL_MIRROR);
}

void win() {
    if (hasWon) {
        return;
    }
    hasWon = 1;
    gfmText_setTextStatic(pText
        , "YOU DID IT!\n"
          "CONGRATULATIONS!!"
        , 1/*doCopy*/);
}
