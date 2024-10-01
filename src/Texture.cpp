#include <gl3w.h>

#include "Texture.h"
#include "Memory.h"

void
InitAtlas(MemoryArena *arena, TextureAtlas *atlas, U32 max_regions, U32 width, U32 height)
{
    *atlas = {0};
    atlas->regions = CreateArray<AtlasRegion>(arena, max_regions);

    glGenTextures(1, &atlas->handle);
    // WHy do i bind here?
    glBindTexture(GL_TEXTURE_2D, atlas->handle);
}

AtlasRegion *
AddAtlasRegion(TextureAtlas* atlas, int x, int y, int width, int height)
{
    AtlasRegion *region = atlas->regions.PushBack();
    region->atlas = atlas;
    region->width = width;
    region->height = height;
    region->x = x;
    region->y = y;
    return region;
}

void
NormalizePositions(TextureAtlas* atlas, int totalWidth, int totalHeight)
{
    for(AtlasRegion& region : atlas->regions)
    {
        region.pos = V2(region.x/((R32)totalWidth), region.y/((R32)totalHeight));
        region.size = V2(region.width/((R32)totalWidth), region.height/((R32)totalHeight));
    }
}

void
SetTextureAtlasImageData(TextureAtlas* atlas, U8* data)
{
    memcpy(atlas->image, data, sizeof(unsigned char)*4*atlas->width*atlas->height);
    glBindTexture(GL_TEXTURE_2D, atlas->handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->width, atlas->height, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->image);
}

TextureAtlas *
MakeDefaultTexture(MemoryArena *arena, int circleRadius)
{
    TextureAtlas *atlas = PushStruct(arena, TextureAtlas);
    int width = circleRadius*2 + 2;
    int height = circleRadius*2 + 2;
    InitAtlas(arena, atlas, 2, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Not stored. Doesnt matter
    U32 *image = PushZeroArray(arena, U32, width*height);
    R32 circleX = (R32)circleRadius;
    R32 circleY = (R32)circleRadius;
    R32 R2 = (R32)circleRadius*(R32)circleRadius;
    for(int y = 0; y < width; y++)
    for(int x = 0; x < height; x++)
    {
        R32 cx = (R32)x+0.5f;
        R32 cy = (R32)y+0.5f;
        R32 dx = (R32)(cx-circleX);
        R32 dy = (R32)(cy-circleY);
        R32 d2 = dx*dx + dy*dy;
        image[x+y*width] = d2 < R2 ? RGBAColor(255, 255, 255, 255) : RGBAColor(0, 255, 255, 255);
    }

    image[width-1+(height-1)*width] = RGBAColor(255, 255, 255, 255);

    glBindTexture(GL_TEXTURE_2D, atlas->handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    AddAtlasRegion(atlas, 0, 0, circleRadius*2, circleRadius*2);
    AddAtlasRegion(atlas, width-1, height-1, 0, 0);
    NormalizePositions(atlas, width, height);

    // Total hack, adding half a pixel to the coordinate so white region is accurate.
    R32 xx = 1.0f/width;
    R32 yy = 1.0f/height;
    atlas->regions[1].pos.x+=xx*0.5f;
    atlas->regions[1].pos.y+=yy*0.5f;

    return atlas;
}
