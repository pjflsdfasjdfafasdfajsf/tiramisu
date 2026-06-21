#include "Host.h"
#include "SDK.h"
#include "SDL.h"

#include "wasm_export.h"

//
// NOTE: Host-provided funcs.
//

static Void HostPrintLine(wasm_exec_env_t ExecEnv, Uint32 PtrOffset, Uint32 Len)
{
    wasm_module_inst_t ModuleInst = wasm_runtime_get_module_inst(ExecEnv);
    if (!wasm_runtime_validate_app_str_addr(ModuleInst, PtrOffset))
    {
        LogCritical("Memory bounds violation.\n");
        return;
    }

    const char *Ptr = (const char *)wasm_runtime_addr_app_to_native(ModuleInst, PtrOffset);
    if (Ptr && Len > 0)
    {
        SDL_Log("%.*s\n", (Int32)Len, Ptr);
    }
}

//
// NOTE: Implementation.
//

Host HostInit(Void)
{
    Host Result = {0};

    RuntimeInitArgs InitArgs = {0};
    InitArgs.mem_alloc_type = Alloc_With_Allocator;
    InitArgs.mem_alloc_option.allocator.malloc_func = SDL_malloc;
    InitArgs.mem_alloc_option.allocator.realloc_func = SDL_realloc;
    InitArgs.mem_alloc_option.allocator.free_func = SDL_free;

    if (!wasm_runtime_full_init(&InitArgs))
    {
        LogCritical("wasm_runtime_full_init\n");
        return Result;
    }

    Result.IsValid = True;
    return Result;
}

Void HostDeinit(Host *Host)
{
    Assert(Host);

    if (Host->ExecEnv)
    {
        wasm_runtime_destroy_exec_env(Host->ExecEnv);
        Host->ExecEnv = 0;
    }
    if (Host->ModuleInst)
    {
        wasm_runtime_deinstantiate(Host->ModuleInst);
        Host->ModuleInst = 0;
    }
    if (Host->Module)
    {
        wasm_runtime_unload(Host->Module);
        Host->Module = 0;
    }
    if (Host->Bytes)
    {
        SDL_free(Host->Bytes);
        Host->Bytes = 0;
    }

    Host->IsValid = False;
}

Bool HostLoadOne(Host *Host, const char *Path)
{
    Assert(Host);
    Assert(Path);

    Usize Size = 0;
    Host->Bytes = SDL_LoadFile(Path, &Size);
    if (!Host->Bytes)
    {
        LogCritical("%s\n", SDL_GetError());
        return False;
    }

    char ErrorBuf[128];

    NativeSymbol Natives[] = {
        {"PrintLine", HostPrintLine, "(ii)", 0}};
    if (!wasm_runtime_register_natives("env", Natives, sizeof(Natives) / sizeof(Natives[0])))
    {
        LogCritical("wasm_runtime_register_natives\n");
        return False;
    }

    Host->Module = wasm_runtime_load((Uint8 *)Host->Bytes, (Uint32)Size, ErrorBuf, sizeof(ErrorBuf));
    if (!Host->Module)
    {
        LogCritical("wasm_runtime_load: %s\n", ErrorBuf);
        return False;
    }

    Host->ModuleInst = wasm_runtime_instantiate(Host->Module, Kb(64), Kb(64), ErrorBuf, sizeof(ErrorBuf));
    if (!Host->ModuleInst)
    {
        LogCritical("wasm_runtime_instantiate: %s\n", ErrorBuf);
        return False;
    }

    Host->ExecEnv = wasm_runtime_create_exec_env(Host->ModuleInst, Kb(16));
    if (!Host->ExecEnv)
    {
        LogCritical("wasm_runtime_create_exec_env\n");
        return False;
    }

    //
    // NOTE: Imports.
    //

#define Function(Name, Signature)                                       \
    Host->Name = wasm_runtime_lookup_function(Host->ModuleInst, #Name); \
    if (!Host->Name)                                                    \
    {                                                                   \
        LogCritical("wasm_runtime_lookup_function  ('%s')\n"            \
                    "NOTE: Host expects '%s' to be exported.\n",        \
                    #Name, #Signature);                                 \
        return False;                                                   \
    }

    Function(UpdateAndRender, Void UpdateAndRender(*State, *ExtraMem, *RenderBuf));

    Function(GetState, State * GetState(Void));
    Function(GetRenderBuf, RenderBuf * GetRenderBuf(Void));
    Function(GetExtraMem, Void * GetExtraMem(Void));

#undef Function

#define Pointer(Name)                                                           \
    {                                                                           \
        Uint32 Results[1] = {0};                                                \
        if (wasm_runtime_call_wasm(Host->ExecEnv, Host->Get##Name, 0, Results)) \
        {                                                                       \
            Host->Name = Results[0];                                            \
        }                                                                       \
    }

    Pointer(State);
    Pointer(RenderBuf);
    Pointer(ExtraMem);

#undef Pointer

    Host->IsValid = True;
    return True;
}

Bool HostUpdate(Host *Host, State *HostState, RenderBuf *HostRenderBuf)
{
    Assert(Host);
    Assert(Host->UpdateAndRender);
    Assert(Host->ModuleInst);

    // NOTE: State
    if (!wasm_runtime_validate_app_addr(Host->ModuleInst, Host->State, sizeof(State)))
    {
        LogCritical("State memory is out of bounds.\n");
        return False;
    }
    State *NativeState = (State *)wasm_runtime_addr_app_to_native(Host->ModuleInst, Host->State);

    // NOTE: RenderBuf
    if (!wasm_runtime_validate_app_addr(Host->ModuleInst, Host->RenderBuf, 16))
    {
        LogCritical("RenderBuf header is out of bounds.\n");
        return False;
    }
    Uint32 *NativeRenderBuf = (Uint32 *)wasm_runtime_addr_app_to_native(Host->ModuleInst, Host->RenderBuf);

    // NOTE: Copy in.
    SDL_memcpy(NativeState, HostState, sizeof(State));

    Uint32 Args[3] = {Host->State, Host->ExtraMem, Host->RenderBuf};
    if (!wasm_runtime_call_wasm(Host->ExecEnv, Host->UpdateAndRender, 3, Args))
    {
        LogCritical("wasm_runtime_call_wasm  (runtime error): %s\n", wasm_runtime_get_exception(Host->ModuleInst));

        return False;
    }

    // NOTE: Copy out.
    SDL_memcpy(HostState, NativeState, sizeof(State));

    // typedef struct
    // {
    //     Uint8 *Mem; 0-3
    //     Uint32 Size; 4-7
    //     Uint32 Cap; 8-11

    //     Bool IsValid; 12-15
    // } RenderBuf;

    Uint32 RenderBufMemOffset = NativeRenderBuf[0];
    Uint32 RenderBufSizeOffset = NativeRenderBuf[1];

    if (RenderBufSizeOffset == 0)
    {
        return True;
    }

    if (!wasm_runtime_validate_app_addr(Host->ModuleInst, RenderBufMemOffset, RenderBufSizeOffset))
    {
        LogCritical("RenderBuf memory is out of bounds.\n");

        return False;
    }

    Void *RenderBufMem = (Void *)wasm_runtime_addr_app_to_native(Host->ModuleInst, RenderBufMemOffset);

    Void *Dest = RenderBufPush(HostRenderBuf, RenderBufSizeOffset);
    if (Dest)
    {
        SDL_memcpy(Dest, RenderBufMem, RenderBufSizeOffset);
        NativeRenderBuf[1] = 0;
    }
    else
    {
        LogCritical("Out of memory.\n");

        return False;
    }

    return True;
}
