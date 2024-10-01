
#include "Skeleton.h"

// This should eventually be created by a memory pool ? 
Skeleton*
CreateSkeleton(MemoryArena* arena, Vec3 pos, R32 scale)
{
    // Scary D:
    Skeleton* skele = PushStruct(arena, Skeleton);
    *skele = {0};
    return skele;
}
