#include "Ent.h"
#include "Public/Math.h"
#include "Public/Mem.h"
#include "SDL.h"

CompTypeResult CompInit(World *World, Uint32 Size)
{
    Assert(World);

    CompTypeResult Result = {0};

    if (World->CompTypeCount >= MaxCompTypes)
    {
        LogCritical("Maximum component types (%u) exceeded.\n", MaxCompTypes);
        Result.ID = 0;
        Result.IsValid = False;
        return Result;
    }

    if (Size > MaxCompSize)
    {
        LogCritical("Comp size %u exceeds MaxCompSize (%u).\n", Size, MaxCompSize);
        Result.ID = 0;
        Result.IsValid = False;
        return Result;
    }

    Uint32 TypeID = World->CompTypeCount;
    World->CompSizes[TypeID] = Size;
    World->CompTypeCount++;

    Result.ID = TypeID;
    Result.IsValid = True;
    return Result;
}

EntResult EntInit(World *World)
{
    Assert(World);

    EntResult Result = {0};

    for (Uint32 I = 0; I < MaxEnts; ++I)
    {
        if (!World->EntActive[I])
        {
            World->EntActive[I] = True;
            Result.ID = I;
            Result.IsValid = True;
            return Result;
        }
    }

    LogCritical("Maximum entity capacity (%u) reached.\n", MaxEnts);
    Result.ID = 0;
    Result.IsValid = False;
    return Result;
}

Bool EntAddComp(World *World, Uint32 EntID, Uint32 TypeID, const Void *Mem)
{
    Assert(World);
    Assert(Mem);

    if (EntID >= MaxEnts)
    {
        LogCritical("Invalid Entity ID %u.\n", EntID);
        return False;
    }

    if (TypeID >= World->CompTypeCount)
    {
        LogCritical("Invalid Comp Type ID %u.\n", TypeID);
        return False;
    }

    if (!World->EntActive[EntID])
    {
        LogCritical("Entity %u is inactive.\n", EntID);
        return False;
    }

    if (!Mem)
    {
        LogCritical("Invalid dynamic component memory source.\n");
        return False;
    }

    Usize Size = World->CompSizes[TypeID];
    SDL_memcpy(World->CompData[EntID][TypeID], Mem, Size);
    World->CompPresent[EntID][TypeID] = True;
    return True;
}

CompResult EntGetComp(World *World, Uint32 EntID, Uint32 TypeID)
{
    Assert(World);

    CompResult Result = {0};

    if (EntID >= MaxEnts || TypeID >= World->CompTypeCount)
    {
        return Result;
    }

    if (World->EntActive[EntID] && World->CompPresent[EntID][TypeID])
    {
        Usize Size = World->CompSizes[TypeID];
        SDL_memcpy(Result.Mem, World->CompData[EntID][TypeID], Size);
        Result.IsValid = True;
    }

    return Result;
}

//
// NOTE: Res
//

ResID ResGetID(World *World, const char *NamePtr, Usize NameLen)
{
    Assert(World);

    if (!NamePtr || NameLen == 0 || NameLen >= MaxResNameLen)
    {
        LogCritical("Invalid resource registration attempt.\n");
        return ResID_Invalid;
    }

    Uint32 Hash = HashStr(NamePtr, NameLen);
    // TODO: Linear search bad :(
    for (Uint32 I = 0; I < World->ResCount; ++I)
    {
        if (World->Res[I].Hash == Hash)
        {
            return I;
        }
    }

    if (World->ResCount >= MaxRes)
    {
        LogCritical("Maximum res capacity (%u) reached", MaxRes);
        return ResID_Invalid;
    }

    ResID Result = World->ResCount++;
    World->Res[Result] = (Res){0};

    World->Res[Result].Hash = Hash;
    MemCopy(World->ResNames[Result], NamePtr, NameLen);
    MemNullTerminate(World->ResNames[Result], sizeof(World->ResNames[Result]), NameLen);

    return Result;
}

Uint32 ResGetVal(const World *World, ResID ResID)
{
    Assert(World);

    if (ResID >= World->ResCount)
    {
        LogCritical("Attempt to read out-of-bounds ResID: %u\n", ResID);
        return 0;
    }

    return World->Res[ResID].Value;
}

Bool ResSetVal(World *World, Uint32 ResID, Uint32 Value)
{
    Assert(World);

    if (ResID >= World->ResCount)
    {
        LogCritical("Attempt to read out-of-bounds ResID: %u\n", ResID);
        return 0;
    }

    World->Res[ResID].Value = Value;
    return True;
}
