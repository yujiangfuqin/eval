#ifndef CMP_HPP
#define CMP_HPP

#include "config.hpp"
#include "prss.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <limits>

template <typename T> bool Fcmp(int spec_party, RepShare<T> &x, RepShare<T> &y) {
    int party_id = Config::myconfig->get_idex();
    T rx = x.reveal();
    T ry = y.reveal();
    bool res = (rx >= ry) ? 1 : 0;
    if (party_id == spec_party) {
        return res;
    } else {
        return 0;
    }
}

template <typename T>
std::vector<bool> vector_Fcmp(std::vector<int> spec_party, std::vector<RepShare<T>> &x,
                              std::vector<RepShare<T>> &y) {
    int party_id = Config::myconfig->get_idex();
    auto vector_rx = vector_reveal(x);
    auto vector_ry = vector_reveal(y);
    std::vector<bool> vector_res(vector_rx.size());
    for (size_t i = 0; i < vector_rx.size(); i++) {
        bool res = (vector_rx[i] >= vector_ry[i]) ? 1 : 0;
        if (party_id == spec_party[i]) {
            vector_res[i] = res;
        } else {
            vector_res[i] = 0;
        }
    }
    return vector_res;
}

#endif // CMP_HPP