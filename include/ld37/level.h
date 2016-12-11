/**
 * @file include/ld37/level.h
 *
 * Handle loading/switching the level
 */
#ifndef __LD37_LEVEL_H__
#define __LD37_LEVEL_H__

#include <base/error.h>
#include <GFraMe/gfmTilemap.h>

#define TM_DEF_WIDTH    40
#define TM_DEF_HEIGHT   30
#define TM_DEF_TILE     -1
#define TM_DEF_MAP      "map/map_map.gfm"
#define TM_DEF_MAP_LEN  (sizeof(TM_DEF_MAP) - 1)

/** The game's main/only tilemap */
extern gfmTilemap *pMap;

enum enLevelOrientation {
    LO_DEFAULT           = 0x0
  , LO_HORIZONTAL_MIRROR = 0x1
  , LO_VERTICAL_MIRROR   = 0x2
  , LO_MIRROR_BOTH       = (LO_HORIZONTAL_MIRROR | LO_VERTICAL_MIRROR)
};
typedef enum enLevelOrientation levelOrientation;

/** Initialize the level's static data */
err initLevel();
/** Release all static data */
void cleanLevel();
/** Retrieve the level's width in pixels */
int getLevelWidth();
/** Retrieve the level's height in pixels */
int getLevelHeight();
/**
 * Re-load the level into the given orientation
 *
 * @param  [ in]orientation Bitmask of the level's orientation
 */
err loadLevel(levelOrientation orientation);

#endif /* __LD37_LEVEL_H__ */

