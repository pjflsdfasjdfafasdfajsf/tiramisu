#include "SDL.h"
#include "Types.h"
#include "Zip.h"

static const Uint8 Mem[] = {
#embed "Archive.zip"
};

Int32 main()
{
    ZipArchive Zip = ZipOpen(Mem, sizeof(Mem));
    if (!Zip.IsValid)
    {
        LogCritical("Invalid test data.\n");

        return 1;
    }

    for (Uint32 I = 0; I < Zip.Count; I++)
    {
        ZipEntry Entry = ZipGetEntByIndex(&Zip, I);
        if (Entry.IsValid)
        {
            SDL_Log("%u: %.*s (%u uncompressed bytes, %u compressed bytes)\n",
                   I, (int)Entry.NameLen, (const char *)Entry.NamePtr, Entry.UncompressedSize, Entry.CompressedSize);
        }
    }

    ZipEntry Entry = ZipGetEntByName(&Zip, "Build/Test/Data/Archive/Hello.txt");
    if (Entry.IsValid)
    {
        Uint8 *Buffer = SDL_malloc(Entry.UncompressedSize + 1);
        if (Buffer)
        {
            if (ZipReadEnt(&Zip, &Entry, Buffer, Entry.UncompressedSize))
            {
                Buffer[Entry.UncompressedSize] = '\0';
                SDL_Log("%s", (char *)Buffer);
            }
            else
            {
                LogCritical("Extraction failed.\n");
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        LogCritical("Invalid test data.\n");
    }

    return 0;
}
