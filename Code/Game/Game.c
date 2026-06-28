#include <SDK.h>

static EntResult Ent;

UpdateAndRender(UpdateAndRender)
{
    if (!Ent.IsValid)
    {
        Ent = AddEnt();
    }

    CompRenderable Renderable = {0};
    Renderable.Type = RenderableType_Rect;
    Renderable.Rect.Color = Red;
    Renderable.Rect.Filled = Filled;
    EntAddRenderable(Ent.ID, Renderable);

    CompTransform Transform = {0};
    Transform.Pos = V2Make(30.0f, 30.0f);
    Transform.Size = V2Make(100.0f, 100.0f);
    EntAddTransform(Ent.ID, Transform);
}
