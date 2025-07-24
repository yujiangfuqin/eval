#ifndef _PROTOCOL_TYPES_H
#define _PROTOCOL_TYPES_H

#include <vector>
#include <cstdint>

struct Per{
    std::vector<uint32_t> zeta, sigma, zeta_f, sigma_f;

    void resize(size_t new_size) {
        zeta.resize(new_size);
        sigma.resize(new_size);
        zeta_f.resize(new_size);
        sigma_f.resize(new_size);
    }
};

#endif // _PROTOCOL_TYPES_H
