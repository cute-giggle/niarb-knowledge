// Author: cute-giggle@outlook.com

#include "annotation.h"

#include <iostream>
#include <fstream>

namespace fsaverage
{

namespace
{
constexpr uint32_t COLOR_MAX_NAME_LENGTH = 64U;
}

Annotation Annotation::load(const std::filesystem::path& path) noexcept
{
    if (!std::filesystem::exists(path))
    {
        std::cout << "File " << path << "does not exist!" << std::endl;
        return {};
    }

    if (path.stem().string().find("annotation") != 0UL || path.extension() != ".data")
    {
        std::cout << "Only support [annotation.xxx.data]!" << std::endl;
        return {};
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open())
    {
        std::cout << "Open " << path << "failed!" << std::endl;
        return {};
    }

    uint32_t colorCount{};
    input.read(reinterpret_cast<char*>(&colorCount), sizeof(uint32_t));
    Annotation annotation;
    annotation.colorTable.resize(colorCount);
    for (uint32_t i = 0U; i < colorCount; ++i)
    {
        input.read(reinterpret_cast<char*>(&(annotation.colorTable[i])), sizeof(int) * 4U);
        uint32_t nameLength{};
        input.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
        char name[COLOR_MAX_NAME_LENGTH] = {0};
        input.read(name, nameLength);
        annotation.colorTable[i].name = name;
    }

    uint32_t labelCount{};
    input.read(reinterpret_cast<char*>(&labelCount), sizeof(uint32_t));
    annotation.labelIndex.resize(labelCount, 0U);
    input.read(reinterpret_cast<char*>(annotation.labelIndex.data()), labelCount * sizeof(uint32_t));
    return annotation;
}

}