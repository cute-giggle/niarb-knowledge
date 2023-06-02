// Author: cute-giggle@outlook.com

#ifndef BIGENDIANHELPER_HPP
#define BIGENDIANHELPER_HPP

#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>

namespace fsaverage
{

struct BigEndianHelper
{
    struct ReverseEndian
    {
        void operator() (int& num) const noexcept
        {
            num = ((num & 0XFF) << 24) | ((num & 0XFF00) << 8) | ((num & 0XFF0000) >> 8) | ((num & 0XFF000000) >> 24);
        }

        void operator() (float& num) const noexcept
        {
            int tmp = *reinterpret_cast<int*>(&num);
            this->operator()(tmp);
            num = *reinterpret_cast<float*>(&tmp);
        }

        void operator() (short& num) const noexcept
        {
            num = ((num & 0XFF) << 8) | ((num & 0XFF00) >> 8);
        }
    };

    template<typename T, typename U = void>
    struct CouldReverseEndian : std::false_type {};

    template<typename T>
    struct CouldReverseEndian<T, std::void_t<decltype(ReverseEndian()(std::declval<T&>()))>> : std::true_type {};

    static bool isBigEndian() noexcept
    {
        union JudgeEndian
        {
            int64_t i;
            char c[4];
        };
        static const JudgeEndian judge = {1};
        return static_cast<bool>(judge.c[3]);
    };

    static int read3Bytes(std::ifstream& input) noexcept
    {
        static unsigned char buffer[3] = {0};
        input.read(reinterpret_cast<char*>(buffer), 3);
        if (!isBigEndian())
        {
            return (static_cast<int>(buffer[0]) << 16) + (static_cast<int>(buffer[1]) << 8) + static_cast<int>(buffer[2]);
        }
        return (static_cast<int>(buffer[2]) << 16) + (static_cast<int>(buffer[1]) << 8) + static_cast<int>(buffer[0]);
    }

    static int read4Bytes(std::ifstream& input) noexcept
    {
        static int buffer = 0;
        input.read(reinterpret_cast<char*>(&buffer), 4);
        if (!isBigEndian())
        {
            ReverseEndian()(buffer);
        }
        return buffer;
    }

    static std::vector<int> read3BytesMany(std::ifstream& input, std::size_t count) noexcept
    {
        std::vector<int> ret(count);
        std::vector<unsigned char> buffer(count * 3);
        input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (!isBigEndian())
        {
            for (std::size_t i = 0; i < ret.size(); ++i)
            {
                ret[i] = (static_cast<int>(buffer[3 * i]) << 16);
                ret[i] += (static_cast<int>(buffer[3 * i + 1]) << 8) + static_cast<int>(buffer[3 * i + 2]);
            }
        }
        else
        {
            for (std::size_t i = 0; i < ret.size(); ++i)
            {
                ret[i] = static_cast<int>(buffer[3 * i]);
                ret[i] += (static_cast<int>(buffer[3 * i + 1]) << 8) + static_cast<int>(buffer[3 * i + 2] << 16);
            }
        }
        return ret;
    }

    template<typename T>
    static std::enable_if_t<CouldReverseEndian<T>::value, std::vector<T>>
    readSequence(std::ifstream& input, std::size_t count) noexcept
    {
        std::vector<T> ret(count);
        input.read(reinterpret_cast<char*>(ret.data()), ret.size() * sizeof(T));
        if (!isBigEndian())
        {
            std::for_each(ret.begin(), ret.end(), ReverseEndian());
        }
        return ret;
    }

    template<typename T>
    static std::enable_if_t<std::is_same_v<char, T>, std::string>
    readSequence(std::ifstream& input, std::size_t count) noexcept
    {
        std::string ret(count, static_cast<char>(0));
        input.read(ret.data(), count);
        return ret;
    }

};

}

#endif