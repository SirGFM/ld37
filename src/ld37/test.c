#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>
#include <ld37/level.h>
#include <ld37/player.h>
#include <ld37/test.h>
#include <string.h>

err initTest() {
    gfmParser *pParser = 0;
    gfmRV rv;
    err erv;

    erv = loadLevel(LO_DEFAULT);
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    rv = gfmParser_getNew(&pParser);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
    rv = gfmParser_initStatic(pParser, game.pCtx, "map/test_obj.gfm");
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
        else {
            ASSERT_TO(0, erv = ERR_UNKNOWNTYPE, __ret);
        }
    }

    erv = ERR_OK;
__ret:
    gfmParser_free(&pParser);

    return erv;
}

void cleanTest() {
}

err updateTest() {
    gfmRV rv;
    err erv;

#if 0
    if (DID_JUST_PRESS(left)) {
        erv = loadLevel(LO_DEFAULT);
        ASSERT(erv == ERR_OK, erv);
    }
    else if (DID_JUST_PRESS(right)) {
        erv = loadLevel(LO_HORIZONTAL_MIRROR);
        ASSERT(erv == ERR_OK, erv);
    }
    else if (DID_JUST_PRESS(up)) {
        erv = loadLevel(LO_VERTICAL_MIRROR);
        ASSERT(erv == ERR_OK, erv);
    }
    else if (DID_JUST_PRESS(down)) {
        erv = loadLevel(LO_MIRROR_BOTH);
        ASSERT(erv == ERR_OK, erv);
    }
#endif

    rv = gfmTilemap_update(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = preUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    erv = postUpdatePlayer();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

err drawTest() {
    gfmRV rv;
    err erv;

    rv = gfmTilemap_draw(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = drawPlayer();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

