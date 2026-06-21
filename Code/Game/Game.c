//
// NOTE: Main game file.
//
#include "SDK.h"

UpdateAndRender(UpdateAndRender)
{
    if (!State->IsInitialized)
    {
        Print("(Game): Initialized");
        State->IsInitialized = True;
    }

    RenderBufClear(RenderBuf, Color3(255, 255, 255));
}
