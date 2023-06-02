// Author: cute-giggle@outlook.com

#include <iostream>
#include <fstream>
#include <set>
#include <filesystem>

#include "surface.h"

#include "BigEndianHelper.h"

namespace fsaverage
{
// Refer to [nibabel.freesurfer.io.py].
// About freesurfer surrface file formats, see [http://www.grahamwideman.com/gw/brain/fs/surfacefileformats.htm].

namespace
{
constexpr int TRIA_MAGIC = 16777214;
constexpr int QUAD_MAGIC = 16777215;
constexpr int NEWQ_MAGIC = 16777213;

constexpr auto SURFACE_FILE_NAME_FORMAT = "[lh/rh].[orig/white/pial/inflated]";

bool checkSurfaceFilePath(const std::filesystem::path& lpath, const std::filesystem::path& rpath) noexcept
{
    if (!std::filesystem::exists(lpath) || !std::filesystem::exists(rpath))
    {
        return false;
    }

    auto lstem = lpath.stem();
    auto rstem = rpath.stem();
    auto lextension = lpath.extension();
    auto rextension = rpath.extension();
    static const std::set<std::filesystem::path> extensionSet{".orig", ".white", ".pial", ".inflated"};
    return lstem == "lh" && rstem == "rh" && lextension == rextension && extensionSet.count(lextension);
}

auto calculateSurfaceRange(const Surface& surface, uint32_t xyz) noexcept
{
    if (xyz > 2U)
    {
        std::cout << "Param xyz must be one of [0, 1, 2] to mark [x, y, z] axis!" << std::endl;
        return std::make_pair(0.f, 0.f);
    }

    std::pair<float, float> range{0.f, 0.f};
    for (auto iter = surface.point.begin(); iter != surface.point.end(); iter += 3)
    {
        range.first = std::min(range.first, *(iter + xyz));
        range.second = std::max(range.second, *(iter + xyz));
    }
    return range;
}

Surface load(std::ifstream& input) noexcept
{
    // read magic
    int magic = BigEndianHelper::read3Bytes(input);

    if (magic == TRIA_MAGIC)
    {
        // jump two lines file information
        std::string informations;
        std::getline(input, informations);
        std::getline(input, informations);

        // get vertex count
        int vertCount = BigEndianHelper::read4Bytes(input);
        // get face count
        int faceCount = BigEndianHelper::read4Bytes(input);

        // read vertex data
        auto point = BigEndianHelper::readSequence<float>(input, vertCount * 3);
        // read face data
        auto face = BigEndianHelper::readSequence<int>(input, faceCount * 3);

        return {std::move(point), std::move(face)};
    }
    else if (magic == QUAD_MAGIC || magic == NEWQ_MAGIC)
    {
        // NOTE: not test!!!

        // read vertex count
        int vertCount = BigEndianHelper::read3Bytes(input);
        // read face count
        int faceCount = BigEndianHelper::read3Bytes(input);

        Surface surface;
        if (magic == QUAD_MAGIC)
        {
            // read vertex data
            std::vector<short> point = BigEndianHelper::readSequence<short>(input, vertCount * 3);
            auto operation = [](short val) -> float { return static_cast<float>(val) / 100.f; };
            std::transform(point.begin(), point.end(), std::back_inserter(surface.point), operation);
        }
        else
        {
            // read vertex data
            surface.point = BigEndianHelper::readSequence<float>(input, vertCount * 3);
        }

        // read face data
        std::vector<int> face(BigEndianHelper::read3BytesMany(input, faceCount << 2));
        for (auto iter = face.begin(); iter != face.end(); iter += 4)
        {
            std::vector<int> indecies = (*iter % 2 == 0 ? std::vector<int>{0, 1, 3, 2, 3, 1} : std::vector<int>{0, 1, 2, 0, 2, 3});
            std::transform(indecies.begin(), indecies.end(), std::back_inserter(surface.face), [iter](int i) -> int{ return *(iter + i); });
        }

        return surface;
    }
    else
    {
        std::cout << "File does not appear to be a freesurfer surface!" << std::endl;
        return {};
    }
}

Surface load(const std::filesystem::path& lpath, const std::filesystem::path& rpath) noexcept
{
    if (!checkSurfaceFilePath(lpath, rpath))
    {
        std::cout << "File not exist or invalid file name!" << std::endl; 
        return {};
    }

    std::ifstream linput(lpath, std::ios::binary);
    if (!linput.is_open())
    {
        std::cout << "Open " << lpath << " failed!" << std::endl;
        return {};
    }

    std::ifstream rinput(rpath, std::ios::binary);
    if (!rinput.is_open())
    {
        std::cout << "Open " << rpath << " failed!" << std::endl;
        return {};
    }

    // load left and right surface
    auto lsurface = load(linput);
    auto rsurface = load(rinput);
    if (lsurface.empty() || rsurface.empty())
    {
        return {};
    }

    // adjust coordinate
    auto [lmin, lmax] = calculateSurfaceRange(lsurface, 0U);
    for (auto iter = lsurface.point.begin(); iter != lsurface.point.end(); iter += 3)
    {
        *iter += (-lmax);
    }
    auto [rmin, rmax] = calculateSurfaceRange(rsurface, 0U);
    for (auto iter = rsurface.point.begin(); iter != rsurface.point.end(); iter += 3)
    {
        *iter += (-rmin);
    }

    // merge left and right surface
    Surface surface{std::move(lsurface)};
    auto lsurfacePointCount = surface.point.size() / 3U;
    std::copy(rsurface.point.begin(), rsurface.point.end(), std::back_inserter(surface.point));
    auto operation = [lsurfacePointCount](uint32_t i) -> uint32_t { return i + lsurfacePointCount; };
    std::transform(rsurface.face.begin(), rsurface.face.end(), std::back_inserter(surface.face), operation);
    
    return surface;
}

void showSurfaceInformation(const Surface& surface) noexcept
{
    std::cout << "Surface information:" << std::endl;
    std::cout << "    Point count: " << surface.point.size() / 3U << std::endl;
    std::cout << "    Face  count: " << surface.face.size()  / 3U << std::endl;
}

void save(const std::filesystem::path& path, const Surface& surface) noexcept
{
    std::ofstream output(path, std::ios::binary);
    uint32_t count = surface.point.size() / 3U;
    output.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    output.write(reinterpret_cast<const char*>(surface.point.data()), count * 3U * sizeof(float));
    count = surface.face.size() / 3U;
    output.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    output.write(reinterpret_cast<const char*>(surface.face.data()), count * 3U * sizeof(uint32_t));
    
    std::cout << "Save surface to " << std::filesystem::absolute(path) << std::endl;
}

}

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Using [TransformSurface] [left surface file] [right surface file]!" << std::endl;
        return 0;
    }

    auto surface = fsaverage::load(argv[1], argv[2]);
    fsaverage::showSurfaceInformation(surface);
    auto filename = "surface" + std::filesystem::path(argv[1]).extension().string() + ".data";
    fsaverage::save(filename, surface);

    return 0;
}