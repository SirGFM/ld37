#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <GFraMe/gfmTilemap.h>
#include <ld37/level.h>
#include <ld37/test.h>

err initTest() {
    err erv;

    erv = loadLevel(LO_DEFAULT);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

void cleanTest() {
}

err updateTest() {
    gfmRV rv;
    err erv;

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

    rv = gfmTilemap_update(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

err drawTest() {
    gfmRV rv;

    rv = gfmTilemap_draw(pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

