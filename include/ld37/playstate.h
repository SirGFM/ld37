/**
 * @file include/ld37/playstate.h
 *
 * The game's main state
 */
#ifndef __LD37_PLAYSTATE_H__
#define __LD37_PLAYSTATE_H__

#include <base/error.h>

err initPlaystate();
err loadPlaystate();
void cleanPlaystate();
err updatePlaystate();
err drawPlaystate();

#endif /* __LD37_PLAYSTATE_H__*/

