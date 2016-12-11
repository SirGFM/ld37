/**
 * @file include/ld37/level.c
 *
 * Handle loading/switching the level
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/type.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmTilemap.h>
#include <ld37/level.h>
#include <stdlib.h>
#include <string.h>

/* == Global stuff ========================================================== */

/** The game's main/only tilemap */
gfmTilemap *pMap = 0;

/** Single buffer that point to every data */
static int *pDataBuffer = 0;
/** The map's width in tiles */
static int widthInTiles = 0;
/** The map's height in tiles */
static int heightInTiles = 0;
/** The original tilemap data, as loaded from the file */
static int *pBaseData = 0;
/** The tilemap data horizontally mirrored */
static int *pHorizontalMirrorData = 0;
/** The tilemap data vertically mirrored */
static int *pVerticalMirrorData = 0;
/** The tilemap data mirrored in both direction */
static int *pBothMirrorData = 0;

/* == Tilemap types dictionary ============================================== */

/* This enumeration is required to ensured no unnecessary empty space appears on
 * typeValues or typeNames */
enum {
    TM_FLOOR = 0
  , TM_DICT_LEN
};

int typeValues[] = {
    [TM_FLOOR] = T_FLOOR
};

char *typeNames[] = {
    [TM_FLOOR] = "floor"
};

/* == Functions ============================================================= */

/**
 * Modify a tile's orientation
 *
 * @param  [ in]tile        The tile, in its default orientation
 * @param  [ in]orientation The new orientation
 */
static inline int _recalculateTile(int tile, levelOrientation orientation);

/** Initialize the level's static data */
err initLevel() {
    int *pData, i;
    gfmRV rv;

    /* Load the base level */
    rv = gfmTilemap_getNew(&pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(pMap, gfx.pSset8x8, TM_DEF_WIDTH, TM_DEF_HEIGHT
            , TM_DEF_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_loadf(pMap, game.pCtx, TM_DEF_MAP, TM_DEF_MAP_LEN, typeNames
            , typeValues, TM_DICT_LEN);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Retrieve the tilemap's data so it may be mirrored */
    rv = gfmTilemap_getData(&pData, pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_getDimension(&widthInTiles, &heightInTiles, pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    widthInTiles /= 8;
    heightInTiles /= 8;

    /* Alloc enough memory for every map */
    pDataBuffer = malloc(sizeof(int) * widthInTiles * heightInTiles * 4);
    ASSERT(pDataBuffer, ERR_MALLOC);
    pBaseData = pDataBuffer;
    pHorizontalMirrorData = pDataBuffer + widthInTiles * heightInTiles;
    pVerticalMirrorData = pDataBuffer + widthInTiles * heightInTiles * 2;
    pBothMirrorData = pDataBuffer + widthInTiles * heightInTiles * 3;

    /* Initialize every map */
    memcpy(pBaseData, pData, sizeof(int) * widthInTiles * heightInTiles);

    i = 0;
    while (i < heightInTiles) {
        int j = 0;
        while (j < widthInTiles) {
            int tile;

            tile = _recalculateTile(pBaseData[j + i * widthInTiles]
                    , LO_HORIZONTAL_MIRROR);
            pHorizontalMirrorData[widthInTiles - j - 1 + i * widthInTiles]
                    = tile;

            tile = _recalculateTile(pBaseData[j + i * widthInTiles]
                    , LO_VERTICAL_MIRROR);
            pVerticalMirrorData[j + (heightInTiles - i - 1) * widthInTiles]
                    = tile;
            j++;
        }
        i++;
    }

    i = 0;
    while (i < heightInTiles * widthInTiles) {
        int tile;

        tile = _recalculateTile(pBaseData[i], LO_MIRROR_BOTH);
        pBothMirrorData[heightInTiles * widthInTiles - i - 1] = tile;
        i++;
    }

    return ERR_OK;
}

/** Release all static data */
void cleanLevel() {
    gfmTilemap_free(&pMap);
    free(pDataBuffer);

    widthInTiles = 0;
    heightInTiles = 0;

    pBaseData = 0;
    pHorizontalMirrorData = 0;
    pVerticalMirrorData = 0;
    pBothMirrorData = 0;
}

/** Retrieve the level's width in pixels */
int getLevelWidth() {
    return widthInTiles * 8;
}

/** Retrieve the level's height in pixels */
int getLevelHeight() {
    return heightInTiles * 8;
}


/**
 * Re-load the level into the given orientation
 *
 * @param  [ in]orientation Bitmask of the level's orientation
 */
err loadLevel(levelOrientation orientation) {
    int *pData;
    gfmRV rv;

    /** Load the new orientation into the map */
    rv = gfmTilemap_getData(&pData, pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    switch (orientation) {
        case LO_DEFAULT:
            memcpy(pData, pBaseData
                    , sizeof(int) * widthInTiles * heightInTiles);
            break;
        case LO_HORIZONTAL_MIRROR:
            memcpy(pData, pHorizontalMirrorData
                    , sizeof(int) * widthInTiles * heightInTiles);
            break;
        case LO_VERTICAL_MIRROR:
            memcpy(pData, pVerticalMirrorData
                    , sizeof(int) * widthInTiles * heightInTiles);
            break;
        case LO_MIRROR_BOTH:
            memcpy(pData, pBothMirrorData
                    , sizeof(int) * widthInTiles * heightInTiles);
            break;
        default:
            ASSERT(0, ERR_ARGUMENTBAD);
    }

    rv = gfmTilemap_recacheAnimations(pMap);
    ASSERT(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILEANIM, ERR_GFMERR);
    rv = gfmTilemap_recalculateAreas(pMap);
    ASSERT(rv == GFMRV_OK || rv == GFMRV_TILEMAP_NO_TILETYPE, ERR_GFMERR);

    rv = gfmQuadtree_initRoot(collision.pStaticQt, -8/*x*/, -8/*y*/
            , (widthInTiles + 2) * 8, (heightInTiles + 2) * 8, 8/*depth*/
            , 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_setStatic(collision.pStaticQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_populateTilemap(collision.pStaticQt, pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Modify a tile's orientation
 *
 * @param  [ in]tile        The tile, in its default orientation
 * @param  [ in]orientation The new orientation
 */
static inline int _recalculateTile(int tile, levelOrientation orientation) {
    switch (tile) {
#define MIRRORED_CASE(tile, offB, offC, offD) \
        case tile: \
        case tile + 4: \
        case tile + 8: \
        case tile + 12: \
            if (orientation == LO_HORIZONTAL_MIRROR)    return tile + 4 + offB; \
            else if (orientation == LO_VERTICAL_MIRROR) return tile + 8 + offC; \
            else if (orientation == LO_MIRROR_BOTH)     return tile + 12 + offD; \
            else                                        return tile; \
            break

        MIRRORED_CASE(64,  2,  32,  34); /* top left */
        MIRRORED_CASE(65,  0,  32,  32); /* top */
        MIRRORED_CASE(66, -2,  32,  30); /* top right */
        MIRRORED_CASE(80,  2,   0,   2); /* left */
        MIRRORED_CASE(81,  0,   0,   0); /* center */
        MIRRORED_CASE(82, -2,   0,  -2); /* right */
        MIRRORED_CASE(96,  2, -32, -30); /* bottom left */
        MIRRORED_CASE(97,  0, -32, -32); /* bottom */
        MIRRORED_CASE(98, -2, -32, -34); /* bottom right */
        MIRRORED_CASE(67,  0,   0,   0); /* diagonal shadow */
        MIRRORED_CASE(83,  0,   0,   0); /* full shadow */

#undef MIRRORED_CASE

        default: return tile;
    }
    return tile;
}

