// TODO: Document that stuff

#include <SDK.h>

Init(Init)
{
    EntResult Ent = EntInit();

    CompTransform Transform = {
        .Pos = V2Make(300.0f, 300.0f),
        .Size = V2Make(100.0f, 100.0f),
    };
    EntAddComp(Ent.ID, CompTransformHash, &Transform);

    CompRenderable Renderable = {
        .Type = RenderableType_DebugText,
        .DebugText = {
            .Color = White,
            .Scale = V2Make(2.0f, 2.0f),
            .Str = "Hello Example Mod!"
        }};

    EntAddComp(Ent.ID, CompRenderableHash, &Renderable);
}

UpdateAndRender(UpdateAndRender)
{
}
