#include "defines.h"

#include <core/assert.h>

/**
 * @file
 * 
 * This translation unit is just used to verify stuff from defines.h,
 * such as the size of fixed-width integers. This doesn't need to be
 * in the header so it makes more sense to put it in its own translation
 * unit.
 */

LSTATIC_ASSERT(sizeof(u8) == 1, "u8 is the wrong size");
LSTATIC_ASSERT(sizeof(u16) == 2, "u16 is the wrong size");
LSTATIC_ASSERT(sizeof(u32) == 4, "u32 is the wrong size");
LSTATIC_ASSERT(sizeof(u64) == 8, "u64 is the wrong size");
LSTATIC_ASSERT(sizeof(i8) == 1, "i8 is the wrong size");
LSTATIC_ASSERT(sizeof(i16) == 2, "i16 is the wrong size");
LSTATIC_ASSERT(sizeof(i32) == 4, "i32 is the wrong size");
LSTATIC_ASSERT(sizeof(i64) == 8, "i64 is the wrong size");
LSTATIC_ASSERT(sizeof(f32) == 4, "f32 is the wrong size");
LSTATIC_ASSERT(sizeof(f64) == 8, "f64 is the wrong size");
LSTATIC_ASSERT(sizeof(b8) == 1, "b8 is the wrong size");
