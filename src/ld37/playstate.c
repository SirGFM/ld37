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
#include <GFraMe/gfmTilemap.h>
#include <ld37/hook.h>
#include <ld37/level.h>
#include <ld37/player.h>
#include <ld37/playstate.h>
#include <string.h>

/** Group with every 'stuff' (any collectible/interactable) */
static gfmGroup *pStuff = 0;

/** Main camera */
static gfmCamera *pCamera = 0;

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
    rv = gfmGroup_setCollisionQuality(pStuff, gfmCollisionQuality_visibleOnly);
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

    erv = loadLevel(LO_DEFAULT);
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

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
}

err updatePlaystate() {
    gfmRV rv;
    err erv;

    rv = gfmQuadtree_initRoot(collision.pQt, -8/*x*/, -8/*y*/
            , getLevelWidth() + 16, getLevelHeight() + 16, 8/*depth*/
            , 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = preUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    erv = updateHook();
    ASSERT(erv == ERR_OK, erv);

    erv = postUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    rv = gfmGroup_update(pStuff, game.pCtx);
    ASSERT(erv == ERR_OK, erv);
    rv = gfmQuadtree_collideGroup(collision.pQt, pStuff);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        doCollide(collision.pQt);
    }

    /* Center the camera */
    do {
        rv = gfmCamera_isSpriteInside(pCamera, pPlayer);
        if (rv == GFMRV_TRUE) {
            int x, y;
            gfmSprite_getCenter(&x, &y, pPlayer);
            gfmCamera_centerAtPoint(pCamera, x, y);
        }
    } while (0);

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

