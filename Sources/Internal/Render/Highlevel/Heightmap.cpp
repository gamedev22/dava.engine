#include "Render/Highlevel/Heightmap.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Heightmap)
{
    ReflectionRegistrator<Heightmap>::Begin()
    .Field("size", &Heightmap::size)[M::ReadOnly(), M::DisplayName("Size")]
    .Field("tileSize", &Heightmap::tileSize)[M::ReadOnly(), M::DisplayName("Tile size")]
    .End();
}

String Heightmap::FILE_EXTENSION(".heightmap");

Heightmap::Heightmap(int32 size)
{
    if (size)
    {
        ReallocateData(size);
    }
}

Heightmap::~Heightmap()
{
    SafeDeleteArray(data);
}

bool Heightmap::BuildFromImage(const DAVA::Image* image)
{
    DVASSERT(image);
    DVASSERT((image->GetWidth() == image->GetHeight()) && (IsPowerOf2(image->GetWidth())));

    Image* heightImage = Image::CreateFromData(image->GetWidth(), image->GetHeight(), image->GetPixelFormat(), image->GetData());
    heightImage->FlipVertical();

    ReallocateData(heightImage->width);

    if (FORMAT_A16 == heightImage->format)
    {
        Memcpy(data, heightImage->data, size * size * sizeof(uint16));
    }
    else if (FORMAT_A8 == heightImage->format)
    {
        uint16* dstData = data;
        uint8* srcData = heightImage->data;

        for (int32 i = size * size - 1; i >= 0; --i)
        {
            *dstData++ = *srcData++ * IMAGE_CORRECTION;
        }
    }
    else
    {
        Logger::Error("Heightmap build from wrong formatted image: format = %d", heightImage->format);
        return false;
    }

    SafeRelease(heightImage);
    return true;
}

void Heightmap::SaveToImage(const FilePath& filename)
{
    Image* image = Image::Create(size, size, FORMAT_A16);
    Memcpy(image->data, data, size * size * sizeof(uint16));
    image->FlipVertical();

    ImageSystem::Save(filename, image, image->format);
    SafeRelease(image);
}

void Heightmap::ReallocateData(int32 newSize)
{
    if (size != newSize)
    {
        SafeDeleteArray(data);

        size = newSize;
        data = new uint16[size * size];
    }
}

void Heightmap::Save(const FilePath& filePathname)
{
    if (0 == size)
    {
        Logger::Error("Heightmap::Save: heightmap is empty");
        return;
    }

    if (!filePathname.IsEqualToExtension(FileExtension()))
    {
        Logger::Error("Heightmap::Save wrong extension: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    File* file = File::Create(filePathname, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("Heightmap::Save failed to create file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    file->Write(&size, sizeof(size));
    file->Write(&tileSize, sizeof(tileSize));

    if (size && tileSize)
    {
        int32 blockCount = size / tileSize;
        for (int32 iRow = 0; iRow < blockCount; ++iRow)
        {
            for (int32 iCol = 0; iCol < blockCount; ++iCol)
            {
                int32 tileY = iRow * size * tileSize;
                int32 tileX = iCol * tileSize;
                for (int32 iTileRow = 0; iTileRow < tileSize; ++iTileRow, tileY += size)
                {
                    file->Write(data + tileY + tileX, tileSize * sizeof(data[0]));
                }
            }
        }
    }

    SafeRelease(file);
}

bool Heightmap::Load(const FilePath& filePathname)
{
    if (!filePathname.IsEqualToExtension(FileExtension()))
    {
        Logger::Error("Heightmap::Load failed with wrong extension: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }

    File* file = File::Create(filePathname, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("Heightmap::Load failed to create file: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }

    int32 mapSize = 0, mapTileSize = 0;

    file->Read(&mapSize, sizeof(mapSize));
    file->Read(&mapTileSize, sizeof(mapTileSize));

    if (mapSize && mapTileSize)
    {
        if (!IsPowerOf2(mapSize))
        {
            LoadNotPow2(file, mapSize, mapTileSize);
        }
        else
        {
            ReallocateData(mapSize);
            SetTileSize(mapTileSize);

            int32 blockCount = mapSize / mapTileSize;
            for (int32 iRow = 0; iRow < blockCount; ++iRow)
            {
                for (int32 iCol = 0; iCol < blockCount; ++iCol)
                {
                    int32 tileY = iRow * mapSize * mapTileSize;
                    int32 tileX = iCol * mapTileSize;
                    for (int32 iTileRow = 0; iTileRow < mapTileSize; ++iTileRow, tileY += mapSize)
                    {
                        file->Read(data + tileY + tileX, tileSize * sizeof(data[0]));
                    }
                }
            }
        }
    }

    SafeRelease(file);
    return true;
}

void Heightmap::LoadNotPow2(File* file, int32 readMapSize, int32 readTileSize)
{
    int32 mapSize = 1 << HighestBitIndex(readMapSize);
    int32 mapTileSize = 1 << HighestBitIndex(readTileSize);

    Logger::Warning("[Heightmap::Load] Heightmap was cropped to %dx%d with tile size %d", mapSize, mapSize, mapTileSize);

    ReallocateData(mapSize);
    SetTileSize(mapTileSize);

    uint16* tileRowData = new uint16[readTileSize];

    int32 blockCount = mapSize / mapTileSize;
    for (int32 iRow = 0; iRow < blockCount; ++iRow)
    {
        for (int32 iCol = 0; iCol < blockCount; ++iCol)
        {
            int32 tileY = iRow * mapSize * mapTileSize;
            int32 tileX = iCol * mapTileSize;
            for (int32 iTileRow = 0; iTileRow < readTileSize; ++iTileRow, tileY += mapSize)
            {
                file->Read(tileRowData, readTileSize * sizeof(data[0]));
                if (iTileRow < mapTileSize)
                    Memcpy(data + tileY + tileX, tileRowData, mapTileSize * sizeof(data[0]));
            }
        }
    }

    SafeDeleteArray(tileRowData);
}

void Heightmap::UpdateHeightmap(const Vector<uint16>& inputData, const Rect2i& rect)
{
    int32 x = rect.x;
    int32 y = rect.y;
    int32 dx = rect.dx;
    int32 dy = rect.dy;

    DVASSERT(x >= 0);
    DVASSERT(y >= 0);
    DVASSERT(x + dx <= size);
    DVASSERT(y + dy <= size);

    const uint16* inputDataPtr = inputData.data();
    for (int32 yIndex = y; yIndex < y + dy; ++yIndex)
    {
        for (int32 xIndex = x; xIndex < x + dx; ++xIndex)
        {
            uint16& srcPixel = data[yIndex * size + xIndex];
            srcPixel = *inputDataPtr;
            ++inputDataPtr;
        }
    }
}

Heightmap* Heightmap::Clone(DAVA::Heightmap* clonedHeightmap)
{
    Heightmap* createdHeightmap = (clonedHeightmap == nullptr) ? new Heightmap() : clonedHeightmap;
    if (size)
    {
        createdHeightmap->ReallocateData(size);

        memcpy(createdHeightmap->data, data, size * size * sizeof(uint16));
        createdHeightmap->SetTileSize(tileSize); // TODO: is it true?
    }

    return createdHeightmap;
}
};
