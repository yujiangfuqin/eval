#ifndef _PROTOCOL_TYPES_H
#define _PROTOCOL_TYPES_H

#include <vector>
#include <cstdint>
#include <array>


template<typename T>
struct RepShare {
    std::array<T, 2> shares; 

    RepShare() = default;
    RepShare(const T& left, const T& right) : shares{left, right} {}

};

struct Perm{
    std::vector<uint32_t> zeta, sigma, zeta_f, sigma_f;

    void resize(size_t new_size) {
        zeta.resize(new_size);
        sigma.resize(new_size);
        zeta_f.resize(new_size);
        sigma_f.resize(new_size);
    }
};

#endif // _PROTOCOL_TYPES_H
