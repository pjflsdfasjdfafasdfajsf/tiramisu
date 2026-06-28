#include <SDK.h>

Init(Init)
{
    {
        EntResult Ent = EntInit();

        CompTransform Transform = {
            .Pos = V2Make(100.0f, 100.0f),
            .Size = V2Make(100.0f, 100.0f),
        };
        EntAddComp(Ent.ID, ResGetVal(ResGetIDCStr(CompTransformName)), &Transform);

        CompRenderable Renderable = {
            .Type = RenderableType_Rect,
        };
        Renderable.Rect.Color = Red;
        Renderable.Rect.Filled = Filled;
        EntAddComp(Ent.ID, ResGetVal(ResGetIDCStr(CompRenderableName)), &Renderable);
    }

    PrintCStr("Hi miguel");
}

UpdateAndRender(UpdateAndRender)
{
}
