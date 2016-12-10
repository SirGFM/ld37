/**
 * @file include/base/physcs.h
 *
 * Helpers for setting constant values
 */
#ifndef __BASE_PHYSICS_H__
#define __BASE_PHYSICS_H__

/**
 * Calculate the initial jump speed.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_SPEED(timeToAppex, jumpHeight) \
  (-2.0 * TILES_TO_PX(jumpHeight) / FRAMES_TO_S(timeToAppex))

/**
 * Calculate the gravity acceleration.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_ACCELERATION(timeToAppex, jumpHeight) \
  (2.0 * TILES_TO_PX(jumpHeight) / (FRAMES_TO_S(timeToAppex) * \
    FRAMES_TO_S(timeToAppex)))

/** Convert from tiles to pixels. Each tile is considered to be 8x8 pixels. */
#define TILES_TO_PX(n) (n * 8)

/** Convert from frames to seconds. To make it framerate independent, this
 * considers the game to be running at 60 FPS. */
#define FRAMES_TO_S(n) (n / 60.0)

#endif /* __BASE_PHYSICS_H__ */

