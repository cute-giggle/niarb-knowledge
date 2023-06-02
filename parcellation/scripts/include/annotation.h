// Author: cute-giggle@outlook.com

#include <vector>
#include <string>
#include <filesystem>

namespace fsaverage
{

struct ColorTableItem
{
    int R{};
    int G{};
    int B{};
    int A{};
    std::string name;
};

struct Annotation
{
    std::vector<ColorTableItem> colorTable;
    std::vector<uint32_t> labelIndex;

    bool empty() const noexcept
    {
        return colorTable.empty() || labelIndex.empty();
    }

    static Annotation load(const std::filesystem::path& path) noexcept;
};

}