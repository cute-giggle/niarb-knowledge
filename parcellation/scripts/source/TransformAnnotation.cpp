// Author: cute-giggle@outlook.com

#include <iostream>
#include <filesystem>
#include <unordered_map>

#include "annotation.h"

#include "BigEndianHelper.h"

namespace fsaverage
{
// Refer to [nibabel.freesurfer.io.py].
// About freesurfer annotation file formats, see [https://surfer.nmr.mgh.harvard.edu/fswiki/LabelsClutsAnnotationFiles].

namespace
{

constexpr auto ANNOTATION_FILE_NAME_FORMAT = "[lh/rh].xxx.annot";

bool checkAnnotationFilePath(const std::filesystem::path& lpath, const std::filesystem::path& rpath) noexcept
{
    if (!std::filesystem::exists(lpath) || !std::filesystem::exists(rpath))
    {
        return false;
    }

    if (lpath.filename().string().find("lh") != 0U || rpath.filename().string().find("rh") != 0U)
    {
        return false;
    }
    return lpath.extension() == ".annot" && rpath.extension() == ".annot";
}

auto readOldVersionColorTable(std::ifstream& input, int entriesCount) noexcept
{
    int fileNameLength = BigEndianHelper::read4Bytes(input);
    std::string fileName = BigEndianHelper::readSequence<char>(input, fileNameLength).substr(0UL, fileNameLength - 1);
    std::cout << "Original old version color table file name: " << fileName << std::endl;

    std::vector<ColorTableItem> color;
    for (int i = 0; i < entriesCount; ++i)
    {
        int labelNameLength = BigEndianHelper::read4Bytes(input);
        std::string labelName = BigEndianHelper::readSequence<char>(input, labelNameLength).substr(0UL, labelNameLength - 1);
        std::vector<int> rgba = BigEndianHelper::readSequence<int>(input, 4);
        color.emplace_back(ColorTableItem{rgba[0], rgba[1], rgba[2], 255 - rgba[3], labelName});
    }
    return std::move(color);
}

auto readNewVersionColorTable(std::ifstream& input, int newVersion) noexcept
{
    if (newVersion != -2)
    {
        std::cout << "Not support this new color table version: " << newVersion << std::endl;
        return std::vector<ColorTableItem>();
    }

    // read max structure id
    BigEndianHelper::read4Bytes(input);

    int fileNameLength = BigEndianHelper::read4Bytes(input);
    std::string fileName = BigEndianHelper::readSequence<char>(input, fileNameLength).substr(0UL, fileNameLength - 1);
    std::cout << "Original new version color table file name: " << fileName << std::endl;

    // read item count
    int entriesCount = BigEndianHelper::read4Bytes(input);

    std::vector<ColorTableItem> color;
    for (int i = 0; i < entriesCount; ++i)
    {
        // read item id (not useful?)
        BigEndianHelper::read4Bytes(input);
        int labelNameLength = BigEndianHelper::read4Bytes(input);
        std::string labelName = BigEndianHelper::readSequence<char>(input, labelNameLength).substr(0UL, labelNameLength - 1);
        std::vector<int> rgba = BigEndianHelper::readSequence<int>(input, 4);
        color.emplace_back(ColorTableItem{rgba[0], rgba[1], rgba[2], 255 - rgba[3], labelName});
    }
    return std::move(color);
}

Annotation load(std::ifstream& input) noexcept
{
    int pointsCount = BigEndianHelper::read4Bytes(input);
    std::vector<int> data = BigEndianHelper::readSequence<int>(input, pointsCount * 2);

    BigEndianHelper::read4Bytes(input);

    int entriesCount = BigEndianHelper::read4Bytes(input);
    auto color = entriesCount > 0 ? readOldVersionColorTable(input, entriesCount) : readNewVersionColorTable(input, entriesCount);

    uint32_t currentIndex = 0U;
    std::unordered_map<int, uint32_t> mapper;
    auto packRGB = [&currentIndex](const ColorTableItem& label) -> std::pair<int, uint32_t>
    {
        return std::make_pair((label.B << 16) + (label.G << 8) + label.R, currentIndex++);
    };
    std::transform(color.begin(), color.end(), std::inserter(mapper, mapper.begin()), packRGB);
    if (mapper.size() != color.size())
    {
        // NOTE: This is not an error?
        std::cout << "The colors in the color table are not unique!" << std::endl;
    }

    auto label = std::vector<uint32_t>(pointsCount);
    for (auto iter = data.begin(); iter != data.end(); iter += 2)
    {
        auto pr = mapper.find(*(iter + 1));
        // NOTE: There may have bug!
        label[*iter] = (pr == mapper.end() ? 0U : pr->second);
    }

    return {std::move(color), std::move(label)};
}

Annotation load(const std::filesystem::path& lpath, const std::filesystem::path& rpath) noexcept
{
    if (!checkAnnotationFilePath(lpath, rpath))
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

    auto lannotation = load(linput);
    auto rannotation = load(rinput);
    if (lannotation.empty() || rannotation.empty())
    {
        return {};
    }

    // merge left and right annotation
    std::unordered_map<std::string, uint32_t> mapper;
    std::vector<uint32_t> index(rannotation.colorTable.size());
    for (uint32_t i = 0U; i < lannotation.colorTable.size(); ++i)
    {
        mapper.emplace(lannotation.colorTable[i].name, i);
    }
    for (uint32_t i = 0U; i < rannotation.colorTable.size(); ++i)
    {
        auto iter = mapper.find(rannotation.colorTable[i].name);
        if (iter == mapper.end())
        {
            lannotation.colorTable.emplace_back(std::move(rannotation.colorTable[i]));
            index[i] = lannotation.colorTable.size() - 1U;
        }
        else
        {
            index[i] = iter->second;
        }
    }
    std::for_each(rannotation.labelIndex.begin(), rannotation.labelIndex.end(), [&index](uint32_t& i) { i = index[i]; });
    std::copy(rannotation.labelIndex.begin(), rannotation.labelIndex.end(), std::back_inserter(lannotation.labelIndex));

    // only hold useful color item
    std::vector<bool> flag(lannotation.colorTable.size(), false);
    std::for_each(lannotation.labelIndex.begin(), lannotation.labelIndex.end(), [&flag](uint32_t i) { flag[i] = true; });
    Annotation annotation{{}, std::move(lannotation.labelIndex)};
    index.resize(lannotation.colorTable.size());
    for (uint32_t i = 0U; i < flag.size(); ++i)
    {
        if (flag[i])
        {
            annotation.colorTable.emplace_back(std::move(lannotation.colorTable[i]));
            index[i] = annotation.colorTable.size() - 1U;
        }
    }
    std::for_each(annotation.labelIndex.begin(), annotation.labelIndex.end(), [&index](uint32_t& i) { i = index[i]; });

    return annotation;
}

void showAnnotationInformation(const Annotation& annotation) noexcept
{
    std::cout << "Annotation information:" << std::endl;
    std::cout << "    Color table size: " << annotation.colorTable.size() << std::endl;
    std::cout << "    Label index size: " << annotation.labelIndex.size() << std::endl;

    std::cout << "Label name:" << std::endl;
    for (auto& colorItem : annotation.colorTable)
    {
        std::cout << "    " << colorItem.name << std::endl;
    }
}

void save(const std::filesystem::path& path, const Annotation& annotation) noexcept
{
    std::ofstream output(path, std::ios::binary);
    uint32_t colorCount = annotation.colorTable.size();
    output.write(reinterpret_cast<const char*>(&colorCount), sizeof(uint32_t));
    for (auto& colorItem : annotation.colorTable)
    {
        output.write(reinterpret_cast<const char*>(&colorItem), 4U * sizeof(int));
        uint32_t nameLength = colorItem.name.length();
        output.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));
        output.write(colorItem.name.data(), nameLength);
    }
    uint32_t labelCount = annotation.labelIndex.size();
    output.write(reinterpret_cast<const char*>(&labelCount), sizeof(uint32_t));
    output.write(reinterpret_cast<const char*>(annotation.labelIndex.data()), labelCount * sizeof(uint32_t));

    std::cout << "Save annotation to " << std::filesystem::absolute(path) << std::endl;
}

}

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Using [TransformAnnotation] [left annotation file] [right annotation file]!" << std::endl;
        return 0;
    }

    auto annotation = fsaverage::load(argv[1], argv[2]);
    fsaverage::showAnnotationInformation(annotation);
    auto stem = std::filesystem::path(argv[1]).stem().string();
    auto pos = stem.find_first_of('.');
    auto filename = "annotation." + (pos == stem.npos ? "" : stem.substr(pos + 1)) + ".data";
    fsaverage::save(filename, annotation);

    return 0;
}