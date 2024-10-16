//
// Created by 李世佳 on 24-10-15.
//
#ifndef HEXCOORDHASH_H
#define HEXCOORDHASH_H
#include <functional>
#include <utility>

namespace piecetype {
    struct HexCoord;
}

namespace std {
    template<>
    struct hash<piecetype::HexCoord> {
        size_t operator()(const piecetype::HexCoord& coord) const noexcept {
            // 使用 std::hash<int> 来为每个坐标计算哈希码，并组合它们
            size_t h1 = std::hash<size_t>()(coord.q);
            size_t h2 = std::hash<size_t>()(coord.r);
            return std::hash<int>()(coord.r) ^ std::hash<int>()(coord.q);
        }
    };
}

#endif //HEXCOORDHASH_H
