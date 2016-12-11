/**
 * @file include/ld37/player.h
 *
 * Handle the player
 */
#ifndef __LD37_PLAYER_H__
#define __LD37_PLAYER_H__

#include <base/error.h>
#include <base/physics.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>

#define PLAYER_DEF_WIDTH    3
#define PLAYER_DEF_HEIGHT   7
#define PLAYER_DEF_OFFX     -2
#define PLAYER_DEF_OFFY     -1

#define PLAYER_JUMP         JUMP_SPEED(16/*frames-to-appex*/, 1.8/*tiles*/)
#define PLAYER_GRAV         JUMP_ACCELERATION(16/*frames-to-appex*/, 1.8/*tiles*/)
#define PLAYER_DRAG         10
#define PLAYER_SPEED        TILES_TO_PX(3)

/** The player's sprite */
extern gfmSprite *pPlayer;

/** Initialize the player */
err initPlayer();
/** Release all resources associated with the player */
void cleanPlayer();
/**
 * Load the player as defined by the parser
 *
 * @param  [ in]pParser The parser
 */
err loadPlayer(gfmParser *pParser);
/** Collide the player against the world */
void collidePlayer();
/** Handle input, update the player's physics and collide it */
err preUpdatePlayer();
/** Adjust everything after all updates. Mostly used to set the animation */
err postUpdatePlayer();
/** Draw the player */
err drawPlayer();

#endif /* __LD37_PLAYER_H__ */

