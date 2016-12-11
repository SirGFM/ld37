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
#define HOOK_SPRITESET  gfx.pSset8x8
#define HOOK_INIT_FRAME 144
#define CHAIN_FRAME     (HOOK_INIT_FRAME + 8)
#define CHAIN_LEN       32
#define MAX_HOOK_DIST   TILES_TO_PX(10)
/** Time (in frames!!!) that take the hook to reach its target */
#define HOOK_TIME       24
/** How many pixels the player may be pulled each frame, while grounded */
#define HOOK_GROUNDED_FORCE 1

/** Initialize the hook */
err initHook();
/** Release all memory used by the hook */
void cleanHook();
/** Reset the hook back to its initial state */
void resetHook();
/** Called whenever the hook grapple onto something */
void onGrapple();
/** Update the hook, if active */
err updateHook();
/** Draw the hook, if active */
err drawHook();
/** Whether the hook did grapple onto something */
int isGrappled();

#endif /* __LD37_HOOK_H__ */

