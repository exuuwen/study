#ifndef _BRANCH_PREDICTION_H_
#define _BRANCH_PREDICTION_H_
#include "../CommonLib.h"
/**
 * Check if a branch is likely to be taken.
 *
 * This gcc builtin allows the developer to indicate if a branch is
 * likely to be taken. Example:
 *
 *   if (likely(x > 1))
 *      do_stuff();
 *
 */
#define likely(x)  __builtin_expect((x),1)

/**
 * Check if a branch is unlikely to be taken.
 *
 * This gcc builtin allows the developer to indicate if a branch is
 * unlikely to be taken. Example:
 *
 *   if (unlikely(x < 1))
 *      do_stuff();
 *
 */
#define unlikely(x)  __builtin_expect((x),0)

#endif /* _BRANCH_PREDICTION_H_ */
