//
// NOTE: Memory utilities.
//
#if !defined(MEM_H)
#define MEM_H

#include "Types.h"

// NOTE: This is a simple bump allocator.
typedef struct
{
    Uint8 *Base;
    Uint32 Cap;
    Uint32 Used;
} MemAlloc;

static inline Void *MemAllocPush(MemAlloc *MemAlloc, Uint32 Bytes, Uint32 Align)
{
    Assert(Bytes > 0);
    Assert(IsPow2(Align));

    Uint32 Offset = AlignUp(MemAlloc->Used, Align);
    Uint32 Req = Offset + Bytes;

    Bool HasSpace = Req <= MemAlloc->Cap;
    Assert(HasSpace);

    if (HasSpace)
    {
        Void *Result = MemAlloc->Base + Offset;
        MemAlloc->Used = Req;

        return Result;
    }

    return 0;
}

static inline Void MemAllocClear(MemAlloc *MemAlloc)
{
    MemAlloc->Used = 0;
}

static inline MemAlloc MemAllocInit(void *Mem, Uint32 Cap)
{
    MemAlloc Result = {};

    Assert(Mem);
    Assert(Cap > 0);

    Result.Base = (Uint8 *)Mem;
    Result.Cap = Cap;
    Result.Used = 0;

    return Result;
}

#endif
