//
// NOTE: Shared definitions between SDL and the app.
//
#if !defined(SHARED_H)
#define SHARED_H

#include "Render.h"
#include "Types.h"

typedef struct State
{
    Bool IsInitialized;
} State;

#define UpdateAndRender(Name) Void Name(State *State, RenderBuf *RenderBuf)
typedef UpdateAndRender(UpdateAndRenderFunction);

#endif
