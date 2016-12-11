/**
 * @file include/ld37/playstate.h
 *
 * The game's main state
 */
#ifndef __LD37_PLAYSTATE_H__
#define __LD37_PLAYSTATE_H__

#include <base/error.h>

#define MAX_STUFF   128

#define VERTICAL_TILE   125
#define HORIZONTAL_TILE 126
#define JEWEL_TILE      127

err initPlaystate();
err loadPlaystate();
void cleanPlaystate();
err updatePlaystate();
err drawPlaystate();

#endif /* __LD37_PLAYSTATE_H__*/

