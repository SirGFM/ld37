/**
 * @file include/conf/type.h
 *
 * Define all in-game types.
 */
#ifndef __CONF_TYPE_H__
#define __CONF_TYPE_H__

#include <GFraMe/gfmTypes.h>

/** Mask that return the proper 16 bit type */
#define T_MASK 0x0000ffff
/** Number of bits per type */
#define T_BITS 16
/**
 * How many bits there are for any given "base type". Different types that share
 * the same base one will be rendered within the quadtree with the same color.
 */
#define T_BASE_NBITS 5

/** Retrieve an object's type (mask out all non-type bits) */
#define TYPE(type) \
    (type & T_MASK)

/** Define a new sub-type */
#define SUBTYPE(base, id) \
    (id << T_BASE_NBITS) | base

enum enType {
      T_FLOOR        = gfmType_reserved_5  /* 10 = purple    */
    , T_PLAYER       = gfmType_reserved_7  /*  8 = light red */
    , T_LEFT_CORNER  = SUBTYPE(T_FLOOR, 1)
    , T_RIGHT_CORNER = SUBTYPE(T_FLOOR, 2)
    , T_HOOK         = SUBTYPE(T_PLAYER, 1)
};
typedef enum enType type;

#endif /* __CONF_TYPE_H__ */

