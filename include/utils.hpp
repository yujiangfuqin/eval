#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

template<typename T>
void apply_permutation_local(std::vector<T>& vec, const std::vector<uint32_t>& rou) {
    std::vector<T> tmp(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        tmp[rou[i]] = vec[i];
    }
    vec = std::move(tmp);
}

template<typename T>
void apply_permutation_local(std::vector<std::vector<T>>& vec, const std::vector<std::vector<uint32_t>>& perms) {
    if (vec.size() != perms.size()) {
        throw std::runtime_error("Dimension mismatch in apply_permutation_local for 2D vectors.");
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        apply_permutation_local(vec[i], perms[i]);
    }
}

#endif // UTILS_HPP