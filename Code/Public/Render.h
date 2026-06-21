#if !defined(RENDER_H)
#define RENDER_H

#include "Mem.h"
#include "Types.h"

#define BUF_ALIGN 4
StaticAssert(IsPow2(BUF_ALIGN));

typedef enum
{
    RenderCommand_None,
    RenderCommand_Clear,
} RenderCommand;

// NOTE: Every command must start with this header for iterators sake.
typedef struct
{
    RenderCommand Type;
    Uint32 Size;
} RenderHeader;

typedef struct
{
    RenderHeader Head;
    Color Col;
} RenderClear;

typedef struct
{
    Uint8 *Mem;
    Uint32 Size;
    Uint32 Cap;

    Bool IsValid;
} RenderBuf;

static inline Void *RenderBufPush(RenderBuf *RenderBuf, Uint32 Bytes)
{
    Assert(Bytes >= sizeof(RenderHeader)); // NOTE: Always must be room for the header.

    Uint32 Aligned = AlignUp(Bytes, BUF_ALIGN);

    Bool HasSpace = (RenderBuf->Size + Aligned) <= RenderBuf->Cap;
    Assert(HasSpace);

    if (HasSpace)
    {
        Uint32 Offset = AlignUp(RenderBuf->Size, BUF_ALIGN);
        Void *Result = RenderBuf->Mem + Offset;
        RenderBuf->Size = Offset + Aligned;

        return Result;
    }

    return 0;
}

static inline Void RenderBufClear(RenderBuf *RenderBuf, Color Col)
{
    RenderClear *cmd = (RenderClear *)RenderBufPush(RenderBuf, sizeof(RenderClear));

    if (cmd)
    {
        cmd->Head.Type = RenderCommand_Clear;
        // NOTE: Must store aligned size here as that is what Push actually reserved.
        cmd->Head.Size = AlignUp(sizeof(RenderClear), BUF_ALIGN);
        cmd->Col = Col;
    }
}

static inline Void RenderBufReset(RenderBuf *RenderBuf)
{
    RenderBuf->Size = 0;
}

static inline RenderBuf RenderBufInit(MemAlloc *MemAlloc, Uint32 Cap)
{
    RenderBuf Result = {};

    Assert(MemAlloc);
    Assert(Cap > 0);

    Result.Mem = (Uint8 *)MemAllocPush(MemAlloc, Cap, BUF_ALIGN);
    if (Result.Mem)
    {
        Result.Size = 0;
        Result.Cap = Cap;
        Result.IsValid = True;

        return Result;
    }

    return Result;
}

// NOTE: This is used only internally. You do not need to port this.
#define RenderBufIterate(BufPtr, CmdPtrName)                       \
    for (RenderHeader *CmdPtrName = (RenderHeader *)(BufPtr)->Mem; \
         (Uint8 *)CmdPtrName < (BufPtr)->Mem + (BufPtr)->Size;     \
         CmdPtrName = (RenderHeader *)((Uint8 *)CmdPtrName + CmdPtrName->Size))

#endif
