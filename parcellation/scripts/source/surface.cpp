// Author: cute-giggle@outlook.com

#include "surface.h"

#include <iostream>
#include <unordered_set>

namespace fsaverage
{

Surface Surface::load(const std::filesystem::path& path) noexcept
{
    if (!std::filesystem::exists(path))
    {
        std::cout << "File " << path << " does not exist!" << std::endl;
        return {};
    }

    static const std::unordered_set<std::string> setter{
        "surface.inflated.data", "surface.orig.data", "surface.pial.data", "surface.white.data"};
    if (!setter.count(path.filename().string()))
    {
        std::cout << "Only support [surface.[inflated/orig/pial/white].data]!" << std::endl;
        return {};
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open())
    {
        std::cout << "Open " << path << "failed!" << std::endl;
        return {};
    }

    uint32_t pointCount{};
    input.read(reinterpret_cast<char*>(&pointCount), sizeof(uint32_t));
    Surface surface;
    surface.point.resize(pointCount * 3U, 0.f);
    input.read(reinterpret_cast<char*>(surface.point.data()), pointCount * 3U * sizeof(float));

    uint32_t faceCount{};
    input.read(reinterpret_cast<char*>(&faceCount), sizeof(uint32_t));
    surface.face.resize(faceCount * 3U, 0U);
    input.read(reinterpret_cast<char*>(surface.face.data()), faceCount * 3U * sizeof(uint32_t));
    return surface;
}

}