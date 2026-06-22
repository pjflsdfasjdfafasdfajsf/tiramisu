//
// NOTE: Memory utilities.
//
#if !defined(MEM_H)
#define MEM_H

#include "Types.h"

//
// NOTE: Allocators
//

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

//
// NOTE: String utilities
//

static inline Uint32 CStrLen(const char *CStr)
{
    Assert(CStr);

    Uint32 Len = 0;
    while (CStr[Len])
    {
        Len++;
    }

    return Len;
}

//
// NOTE: Memory utilities
//

static inline Void MemCopy(Void *DestInit, const Void *SrcInit, Usize Size)
{
    Assert(DestInit);
    Assert(SrcInit);

    if (Size == 0)
    {
        return;
    }

    Uint8 *Dest = (Uint8 *)DestInit;
    const Uint8 *Src = (const Uint8 *)SrcInit;

    for (Usize I = 0; I < Size; ++I)
    {
        Dest[I] = Src[I];
    }
}

static inline Void MemNullTerminate(char *Buffer, Usize Cap, Usize Len)
{
    Assert(Buffer);
    Assert(Cap > 0);

    if (Len < Cap)
    {
        Buffer[Len] = '\0';
    }
    else
    {
        Buffer[Cap - 1] = '\0';
    }
}

#endif
