//
// Created by 李世佳 on 24-10-16.
//
#ifndef HEXCOORD_H
#define HEXCOORD_H
#include <random>
#include <vector>
#include <memory>
#include <iostream>

namespace piecetype {
    struct HexCoord {
        int q; // q-axis coordinate
        int r; // r-axis coordinate

        HexCoord(int q = 0, int r = 0) : q(q), r(r) {}

        int distance(const HexCoord& other) const {
            int dx = std::abs(q - other.q);
            int dy = std::abs(r - other.r);
            int dz = std::abs((q + r) - (other.q + other.r));
            return (dx + dy + dz) / 2;
        }

        std::vector<HexCoord> neighbors() const {
            std::vector<HexCoord> result;
            result.emplace_back(q + 1, r);
            result.emplace_back(q + 1, r - 1);
            result.emplace_back(q, r - 1);
            result.emplace_back(q - 1, r);
            result.emplace_back(q - 1, r + 1);
            result.emplace_back(q, r + 1);
            return result;
        }
        bool operator==(const HexCoord& other) const {
            // 比较逻辑
            return q ==other.q&&r ==other.r;
        }
    };
}
#endif //HEXCOORD_H
