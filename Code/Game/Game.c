//
// NOTE: Main game file.
//
#include <SDK.h>

UpdateAndRender(UpdateAndRender)
{
    if (!State->IsInitialized)
    {
        State->SpriteAtlas = AllocTexture("GameAtlas.png");
        
        PrintCStr("(Game): Initialized");
        State->IsInitialized = True;
    }

    RenderBufClear(RenderBuf, Black);
    RenderBufDrawCStr(RenderBuf, White, V2Make(10.0f, 10.0f), V2Make(2.0f, 2.0f), "Hello, World!\n");

    RenderBufDrawRect(RenderBuf, State->SpriteAtlas, RectMake(100.0f, 100.0f, 100.0f, 100.0f), RectMake(0, 0, 275, 183), White);
}
