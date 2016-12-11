/**
 * @file include/ld37/hook.h
 *
 * Handle the hook and flinging the player
 */
#ifndef __LD37_HOOK_H__
#define __LD37_HOOK_H__

#include <base/error.h>
#include <base/physics.h>

#define HOOK_DEF_WIDTH  2
#define HOOK_DEF_HEIGHT 2
#define HOOK_DEF_OFFX   -1
#define HOOK_DEF_OFFY   -1
#define HOOK_INIT_FRAME 640
#define CHAIN_FRAME     648
#define CHAIN_LEN       32
#define MAX_HOOK_DIST   TILES_TO_PX(10)
/** Time (in frames!!!) that take the hook to reach its target */
#define HOOK_TIME       30

/** Initialize the hook */
err initHook();
/** Release all memory used by the hook */
void cleanHook();
/** Reset the hook back to its initial state */
void resetHook();
/** Update the hook, if active */
err updateHook();
/** Draw the hook, if active */
err drawHook();

#endif /* __LD37_HOOK_H__ */
