//
// Created by jwc on 9/11/21.
//

#ifndef MERCURY_MERCURYCPU_H
#define MERCURY_MERCURYCPU_H

#include "Mercury.h"

//struct MercuryStack *mercuryAllocStack(int stackDepth, int Precision);
mercspec void *mercuryAllocateStack(long Size);
mercspec void mercuryDeallocateStack(void *stack);

mercspec void mercuryPrint(void *stack, int Precision, uint *val);

#endif //MERCURY_MERCURYCPU_H
