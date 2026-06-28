//
// Created by jwc on 9/11/21.
//
#include <stdlib.h>
#include <stdio.h>
#include "MercuryCPU.h"
/*
 * MercuryStack
{
    int sp;
    int datasize;
    int precision;

    unsigned int *stack;
};
 */

void *mercuryAllocateStack(long Size)
{
    void *stack = malloc(Size);
    *(ulong *)stack = 0;
    return stack;
}

void mercuryDeallocateStack(void *stack)
{
    free(stack);
}

void mercuryPrint(void *stack, int Precision, uint *val)
{
    char txt[32768];
    mercuryToString(stack, Precision, val, txt, 32768);
    printf("%s\n", txt);
}

/*
struct MercuryStack *mercuryAllocStack(int stackDepth, int Precision)
{
    struct MercuryStack *ret = (struct MercuryStack *) malloc(sizeof(struct MercuryStack));
    ret->sp = 0;
    ret->dataSize = (Precision + 2);
    ret->precision = Precision;
    ret->stackDepth = stackDepth;
    ret->stack = (unsigned int *) malloc(sizeof(unsigned int) * ret->dataSize *stackDepth);

    return ret;
}
void mercuryFreeStack(struct MercuryStack *stack)
{
    free(stack->stack);
    stack->stack = NULL;
    free(stack);
}
*/