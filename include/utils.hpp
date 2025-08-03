#include <vector>

template<typename T>
void apply_permutation_local(std::vector<T>& vec, const std::vector<uint32_t>& rou) {
    std::vector<T> tmp(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        tmp[rou[i]] = vec[i];
    }
    vec = std::move(tmp);
}