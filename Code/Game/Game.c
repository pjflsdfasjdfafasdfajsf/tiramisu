#include "Ent.h"
#include <SDK.h>

static EntResult Ent;

// TODO: Global bad
static Bool IsInitialized = False;
static CompID TransformID;
static CompID RenderableID;

UpdateAndRender(UpdateAndRender)
{
    if (!IsInitialized)
    {
        TransformID = (CompID)ResGetVal(ResGetIDCStr(CompTransformName));
        RenderableID = (CompID)ResGetVal(ResGetIDCStr(CompRenderableName));

        Ent = EntInit();

        CompRenderable Renderable = {0};
        Renderable.Type = RenderableType_Rect;
        Renderable.Rect.Color = Red;
        Renderable.Rect.Filled = Filled;
        EntAddComp(Ent.ID, RenderableID, &Renderable);

        CompTransform Transform = {0};
        Transform.Pos = V2Make(30.0f, 30.0f);
        Transform.Size = V2Make(100.0f, 100.0f);
        EntAddComp(Ent.ID, TransformID, &Transform);

        IsInitialized = True;
    }
}
