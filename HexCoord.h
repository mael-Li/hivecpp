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
        // 添加小于运算符
        bool operator<(const HexCoord& other) const {
            if (q != other.q) {
                return q < other.q;
            }
            return r < other.r;
        }
        const HexCoord operator-(const HexCoord & hex_coord) const {
            return HexCoord(q - hex_coord.q, r - hex_coord.r);
        }

        HexCoord &operator+=(const HexCoord & hex_coord) {
            r += hex_coord.r;
            q += hex_coord.q;
            return *this;
        }
        const HexCoord&operator+(const HexCoord & hex_coord)const {
            return HexCoord(q+hex_coord.q,r+hex_coord.r);
        }
        bool operator!=(const HexCoord & hex_coord) const {
            return r != hex_coord.r || q != hex_coord.q;
        }

    };
}
#endif //HEXCOORD_H
