//
// NOTE: Main game file.
//
#include "Render.h"
#include <SDK.h>

UpdateAndRender(UpdateAndRender)
{
    if (!State->IsInitialized)
    {
        PrintCStr("(Game): Initialized");
        State->IsInitialized = True;
    }

    RenderBufClear(RenderBuf, Black);
    RenderBufDrawCStr(RenderBuf, Black, V2(10.0f, 10.0f), V2(2.0f, 2.0f), "Hello, World!\n");
}
