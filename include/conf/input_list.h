/**
 * @file include/conf/input_list.h
 *
 * List of available buttons and their default keys.
 *
 * To make the most of this header, a 'X' macro must be defined, which will do
 * something with each parameter. The first will is the name of the virtual
 * button (TBD whether an enum within an array or a member within a struct), the
 * second is the default keyboard mapping and the last (if available) is the
 * default gamepad mapping.
 */
#ifndef __INPUT_LIST_H__
#define __INPUT_LIST_H__

/** Create a list of buttons and their default mapping. */
#define X_RELEASE_BUTTON_LIST \
  X(pause \
      , gfmKey_esc \
      , gfmController_start) \
  X(fullscreen \
      , gfmKey_f12) \
  X(left \
      , gfmKey_left \
      , gfmController_laxis_left) \
  X(right \
      , gfmKey_right \
      , gfmController_laxis_right) \
  X(up \
      , gfmKey_up \
      , gfmController_laxis_up) \
  X(down \
      , gfmKey_down \
      , gfmController_laxis_down) \
  X(jump \
      , gfmKey_space \
      , gfmController_a) \
  X(grapple \
      , gfmKey_c \
      , gfmController_b) \
  X(reset \
      , gfmKey_r)

/** Add default alternate mappings for buttons */
#define X_ALTERNATE_BUTTON_MAPPING \
  X(left \
      , gfmKey_a) \
  X(right \
      , gfmKey_d) \
  X(up \
      , gfmKey_w) \
  X(down \
      , gfmKey_s) \
  X(jump \
      , gfmKey_x) \
  X(jump \
      , gfmKey_j) \
  X(grapple \
      , gfmKey_k \
      , gfmController_x)

/** Create a list of debug buttons */
#if defined(DEBUG)
#  define X_DEBUG_BUTTON_LIST \
     X(qt         , gfmKey_f11) \
     X(gif        , gfmKey_f10) \
     X(dbgStep    , gfmKey_f6, gfmController_l2) \
     X(dbgPause   , gfmKey_f5, gfmController_r2)
#else
#  define X_DEBUG_BUTTON_LIST
#endif

/** Concatenate both lists */
#define X_BUTTON_LIST \
  X_RELEASE_BUTTON_LIST \
  X_DEBUG_BUTTON_LIST

#endif /* __INPUT_LIST_H__ */

