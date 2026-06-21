#include "SDL.h"
#include "Mem.h"
#include "Render.h"
#include "Shared.h"

#if defined(_WIN32)
#define DynamicLibrarySuffix ".dll"
#elif defined(__APPLE__)
#define DynamicLibrarySuffix ".dylib"
#elif defined(__linux__)
#define DynamicLibrarySuffix ".so"
#else
#error "Unsupported platform for compilation"
#endif
#define DynamicLibraryName "Game"

#define CheckReturnBool(Function)                                            \
    if (!(Function))                                                         \
    {                                                                        \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError()); \
        Assert(0)                                                            \
    }

#define CheckReturnPtr(Ptr)                                                  \
    if (!(Ptr))                                                              \
    {                                                                        \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError()); \
        Assert(0)                                                            \
    }

//
// NOTE: Intenral
//

// NOTE: This is needed so it does not matter where you launch the game from and
// it still finds the lib.
static inline Bool GetLibAbsPath(char *Buf, Usize Size)
{
    Assert(Buf);
    Assert(Size > 0);

    const char *BasePath = SDL_GetBasePath();
    if (!BasePath)
    {
        return False;
    }

    int PathLen = SDL_snprintf(Buf, Size, "%s%s%s", BasePath, DynamicLibraryName, DynamicLibrarySuffix);
    if (PathLen < 0 || (Usize)PathLen >= Size)
    {
        return False;
    }

    return True;
}

// NOTE: Code

static inline Code CodeLoad(const char *Path)
{
    Code Result = {0};

    CheckReturnPtr(Result.Handle = SDL_LoadObject(Path));
    CheckReturnPtr(Result.AppUpdateAndRender = (UpdateAndRenderFunction *)SDL_LoadFunction(Result.Handle, "UpdateAndRender"));

    return Result;
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

            CheckReturnBool(SDL_SetRenderDrawColor(SDL->Renderer, Clear->Col.R, Clear->Col.G, Clear->Col.B, Clear->Col.A));
            CheckReturnBool(SDL_RenderClear(SDL->Renderer));
        }
        break;

        case RenderCommand_None:
        default:
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Unrecognized or not implemented render command (%d)", Cmd->Type);
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
}

SDL Init()
{
    SDL Result = {0};

    CheckReturnBool(SDL_Init(SDL_INIT_VIDEO));
    CheckReturnBool(SDL_CreateWindowAndRenderer("Game", 1280, 720, 0, &Result.Window, &Result.Renderer));

    CheckReturnBool(SDL_SetRenderLogicalPresentation(Result.Renderer, 1280, 720, SDL_LOGICAL_PRESENTATION_LETTERBOX));

    Void *Mem = SDL_malloc(Kb(64));
    if (!Mem)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Out of memory\n");
        Assert(0);
    }

    Result.MemAlloc = MemAllocInit(Mem, Kb(64));

    Result.RenderBuf = RenderBufInit(&Result.MemAlloc, Kb(32));
    if (!Result.RenderBuf.IsValid)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize Render Buffer\n");
        Assert(0);
    }

    char Path[1024];
    CheckReturnBool(GetLibAbsPath(Path, sizeof(Path)));

    SDL_Log("Dynamic library path is: %s\n", Path);
    Result.Code = CodeLoad(Path);

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
