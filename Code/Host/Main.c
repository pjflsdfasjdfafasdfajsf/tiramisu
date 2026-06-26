#include "SDL.h"

Int32 main()
{
    SDL App = Init();

    const int Compiled = SDL_VERSION;
const int Linked = SDL_GetVersion();

SDL_Log("compiled %d.%d.%dn",
        SDL_VERSIONNUM_MAJOR(Compiled),
        SDL_VERSIONNUM_MINOR(Compiled),
        SDL_VERSIONNUM_MICRO(Compiled));

SDL_Log("running d.%d.%d\n",
        SDL_VERSIONNUM_MAJOR(Linked),
        SDL_VERSIONNUM_MINOR(Linked),
        SDL_VERSIONNUM_MICRO(Linked));

    while (Poll(&App))
    {
        Update(&App);
        Render(&App);
    }
}
