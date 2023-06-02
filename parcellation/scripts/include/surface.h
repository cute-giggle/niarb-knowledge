// Author: cute-giggle@outlook.com

#include <vector>
#include <filesystem>
#include <fstream>

namespace fsaverage
{

struct Surface
{
    std::vector<float> point;
    std::vector<int> face;

    bool empty() const noexcept
    {
        return point.empty() ||face.empty();
    }

    static Surface load(const std::filesystem::path& path) noexcept;
};

}