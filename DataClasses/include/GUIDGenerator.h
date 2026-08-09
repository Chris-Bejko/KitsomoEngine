#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>


struct FromGUID
{
    std::string guid;
    explicit FromGUID(const std::string& g) : guid(g) {}
};

class EngineGUID
{
public:
    static std::string Generate()
    {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        uint64_t part1 = dis(gen);
        uint64_t part2 = dis(gen);

        // Set version (4) and variant (RFC 4122)
        part1 &= 0xFFFFFFFFFFFF0FFFULL;
        part1 |= 0x0000000000004000ULL;

        part2 &= 0x3FFFFFFFFFFFFFFFULL;
        part2 |= 0x8000000000000000ULL;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        ss << std::setw(8) << (uint32_t)(part1 >> 32) << "-";
        ss << std::setw(4) << (uint16_t)(part1 >> 16) << "-";
        ss << std::setw(4) << (uint16_t)(part1) << "-";
        ss << std::setw(4) << (uint16_t)(part2 >> 48) << "-";
        ss << std::setw(12) << (uint64_t)(part2 & 0x0000FFFFFFFFFFFFULL);

        return ss.str();
    }
};

