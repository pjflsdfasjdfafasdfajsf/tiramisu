#include "SDL.h"

int main()
{
    SDL SDL = Init();

    while (Poll())
    {
        SDL.Code.AppUpdateAndRender(0, &SDL.RenderBuf);
        
        Update(&SDL);
        Render(&SDL);
    }
}
