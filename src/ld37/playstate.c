#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>
#include <ld37/hook.h>
#include <ld37/level.h>
#include <ld37/player.h>
#include <ld37/test.h>
#include <string.h>

err initPlaystate() {
    return ERR_OK;
}

err loadPlaystate() {
    gfmParser *pParser = 0;
    gfmRV rv;
    err erv;

    erv = loadLevel(LO_DEFAULT);
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    rv = gfmParser_getNew(&pParser);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
    rv = gfmParser_initStatic(pParser, game.pCtx, "map/map_obj.gfm");
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

    while (1) {
        char *type;

        rv = gfmParser_parseNext(pParser);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        rv = gfmParser_getIngameType(&type, pParser);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
        if (strcmp(type, "player") == 0) {
            erv = loadPlayer(pParser);
            ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
        }
        else if (strcmp(type, "horizontal")) {
        }
        else if (strcmp(type, "vertical")) {
        }
        else if (strcmp(type, "jewel")) {
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

    return ERR_OK;
}

err drawPlaystate() {
    gfmRV rv;
    err erv;

    rv = gfmTilemap_draw(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = drawPlayer();
    ASSERT(erv == ERR_OK, erv);
    erv = drawHook();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

