#if !defined(RENDER_H)
#define RENDER_H

#include "Mem.h"
#include "Types.h"

#define BUF_ALIGN 4
StaticAssert(IsPow2(BUF_ALIGN));

#define BufAlign(Value) AlignUp(Value, BUF_ALIGN)

// NOTE:
// 1. All positions are in pixels, where (0,0) is the top-left corner of the
// screen
// 2. Intenally renderer uses letterboxing with the resolution of 1280x720
// TODO: Probably will have more to say later

typedef enum
{
    RenderCommand_None,
    RenderCommand_Clear,
    RenderCommand_DrawDebugText,
} RenderCommand;

// NOTE: Every command must start with this header for iterators sake.
typedef struct
{
    RenderCommand Type;
    // NOTE: Must store aligned size here!!!
    Uint32 Size;
} RenderHeader;

// NOTE: Clears the entire screen with the specified `Color`.
typedef struct
{
    RenderHeader Header;

    Color Color;
} RenderClear;

// NOTE: There's a reason why this command is called 'draw DEBUG' text and
// that reason is that the implementation of it just uses SDL built-in
// SDL_RenderDebugText function, which provides just simple bitmap
// font rendering intended for, as you probably already guessed,
// debugging.
typedef struct
{
    RenderHeader Header;

    Vector2 Pos;
    Color Color;
    // NOTE: Scale multiplier.
    Vector2 Scale;

    Uint32 StrLen;
    // NOTE: The final newline is not appended!
    char Str[];
} RenderDrawDebugText;

typedef struct
{
    Uint8 *Mem;
    Uint32 Size;
    Uint32 Cap;

    Bool IsValid;
} RenderBuf;

static inline Void *RenderBufPush(RenderBuf *RenderBuf, Uint32 Bytes)
{
    Assert(RenderBuf);
    Assert(Bytes >= sizeof(RenderHeader)); // NOTE: Always must be room for the header.

    Uint32 Aligned = BufAlign(Bytes);

    Bool HasSpace = (RenderBuf->Size + Aligned) <= RenderBuf->Cap;
    Assert(HasSpace);

    if (HasSpace)
    {
        Uint32 Offset = BufAlign(RenderBuf->Size);
        Void *Result = RenderBuf->Mem + Offset;
        RenderBuf->Size = Offset + Aligned;

        return Result;
    }

    return 0;
}

static inline Void RenderBufClear(RenderBuf *RenderBuf, Color Color)
{
    RenderClear *Cmd = (RenderClear *)RenderBufPush(RenderBuf, sizeof(RenderClear));

    if (Cmd)
    {
        Cmd->Header.Type = RenderCommand_Clear;
        Cmd->Header.Size = BufAlign(sizeof(RenderClear));

        Cmd->Color = Color;
    }
}

static inline Void RenderBufDrawDebugText(RenderBuf *RenderBuf, Color Color, Vector2 Pos, Vector2 Scale, const char *Str, Uint32 StrLen)
{
    if (!Str || StrLen == 0)
    {
        return;
    }

    Uint32 StrCap = StrLen + 1;
    Uint32 Size = sizeof(RenderDrawDebugText) + StrCap;
    RenderDrawDebugText *Cmd = (RenderDrawDebugText *)RenderBufPush(RenderBuf, Size);

    if (Cmd)
    {
        Cmd->Header.Type = RenderCommand_DrawDebugText;
        Cmd->Header.Size = BufAlign(Size);

        Cmd->Pos = Pos;
        Cmd->Color = Color;
        Cmd->Scale = Scale;

        MemCopy(Cmd->Str, Str, StrLen);
        MemNullTerminate(Cmd->Str, StrCap, StrLen);
    }
}

static inline Void RenderBufDrawCStr(RenderBuf *RenderBuf, Color Color, Vector2 Pos, Vector2 Scale, const char *Str)
{
    RenderBufDrawDebugText(RenderBuf, Color, Pos, Scale, Str, CStrLen(Str));
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
