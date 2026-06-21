#include "SDL.h"
#include "Host.h"

//
// NOTE: Intenral
//

#define GAME_WASM_FILE "Game.wasm"

static inline Int64 GetFileModTime(const char *Path)
{
    SDL_PathInfo Info;
    if (SDL_GetPathInfo(Path, &Info))
    {
        return Info.modify_time;
    }

    return 0;
}

// NOTE: This is needed so it does not matter where you launch the game from and
// it still finds the lib.
static inline Bool GetAbsPath(char *Buf, Usize Size, const char *Name)
{
    Assert(Buf);
    Assert(Name);
    Assert(Size > 0);

    const char *BasePath = SDL_GetBasePath();
    if (!BasePath)
    {
        return False;
    }

    Int32 PathLen = SDL_snprintf(Buf, Size, "%s%s", BasePath, Name);
    if (PathLen < 0 || (Usize)PathLen >= Size)
    {
        return False;
    }

    return True;
}

//
// NOTE: SDL
//

Void Render(SDL *SDL)
{
    Assert(SDL);

    RenderBufIterate(&SDL->RenderBuf, Cmd)
    {
        switch (Cmd->Type)
        {
        case RenderCommand_Clear:
        {
            RenderClear *Clear = (RenderClear *)Cmd;

            if (!SDL_SetRenderDrawColor(SDL->Renderer, Clear->Col.R, Clear->Col.G, Clear->Col.B, Clear->Col.A))
            {
                LogCritical("%s", SDL_GetError());
                Assert(0);
            }
            if (!SDL_RenderClear(SDL->Renderer))
            {
                LogCritical("%s", SDL_GetError());
                Assert(0);
            }
        }
        break;

        case RenderCommand_None:
        default:
        {
            LogCritical("Unrecognized or not implemented render command (%d)", Cmd->Type);
            Assert(0);
        }
        break;
        }
    }

    SDL_RenderPresent(SDL->Renderer);
    RenderBufReset(&SDL->RenderBuf);
}

Void Update(SDL *SDL)
{
    Assert(SDL);

    char Path[1024];
    if (GetAbsPath(Path, sizeof(Path), GAME_WASM_FILE))
    {
        Int64 CurrentTime = GetFileModTime(Path);
        if (CurrentTime > SDL->Host.LastWriteTime && CurrentTime)
        {
            SDL_Delay(50);
            SDL_Log("Reload\n");

            // NOTE: Need to save ExtraMem since that's where state is stored
            if (SDL->Host.ModuleInst && SDL->Host.ExtraMem)
            {
                Void *NativeExtraMem = wasm_runtime_addr_app_to_native(SDL->Host.ModuleInst, SDL->Host.ExtraMem);
                if (NativeExtraMem)
                {
                    SDL_memcpy(SDL->ExtraMem, NativeExtraMem, Kb(16));
                }
            }
            HostDeinit(&SDL->Host);
            if (HostLoadOne(&SDL->Host, Path))
            {
                SDL->Host.LastWriteTime = CurrentTime;

                // NOTE: And restore the saved ExtraMem
                if (SDL->Host.ModuleInst && SDL->Host.ExtraMem)
                {
                    Void *NativeExtraMem = wasm_runtime_addr_app_to_native(SDL->Host.ModuleInst, SDL->Host.ExtraMem);
                    if (NativeExtraMem)
                    {
                        SDL_memcpy(NativeExtraMem, SDL->ExtraMem, Kb(16));
                    }
                }
            }
        }
    }
    // NOTE: This handles the case where you got an error on compilation
    if (SDL->Host.IsValid)
    {
        if (!HostUpdate(&SDL->Host, &SDL->State, &SDL->RenderBuf))
        {
            Assert(0);
        }
    }
}

SDL Init()
{
    SDL Result = {0};

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogCritical("%s", SDL_GetError());
        Assert(0);
    }
    if (!SDL_CreateWindowAndRenderer("Game", 1280, 720, 0, &Result.Window, &Result.Renderer))
    {
        LogCritical("%s", SDL_GetError());
        Assert(0);
    }

    if (!SDL_SetRenderLogicalPresentation(Result.Renderer, 1280, 720, SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        LogCritical("%s", SDL_GetError());
        Assert(0);
    }

    Void *Mem = SDL_malloc(Kb(64));
    if (!Mem)
    {
        Assert(0);
    }
    Result.MemAlloc = MemAllocInit(Mem, Kb(64));

    Result.RenderBuf = RenderBufInit(&Result.MemAlloc, Kb(32));
    if (!Result.RenderBuf.IsValid)
    {
        Assert(0);
    }

    Result.ExtraMem = SDL_calloc(1, Kb(16));
    if (!Result.ExtraMem)
    {
        Assert(0);
    }

    Result.Host = HostInit();
    if (!Result.Host.IsValid)
    {
        Assert(0);
    }

    {
        char Path[1024];

        if (!GetAbsPath(Path, sizeof(Path), GAME_WASM_FILE))
        {
            LogCritical("%s", SDL_GetError());
            Assert(0);
        }

        if (!HostLoadOne(&Result.Host, Path))
        {
            Assert(0);
        }

        Result.Host.LastWriteTime = GetFileModTime(Path);
    }

    return Result;
}

Bool Poll()
{
    SDL_Event Ev;
    while (SDL_PollEvent(&Ev))
    {
        switch (Ev.type)
        {
        case SDL_EVENT_QUIT:
        {
            return False;
        }
        break;
        }
    }

    return True;
}
