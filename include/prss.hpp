#ifndef PRSS_HPP
#define PRSS_HPP
#include <cryptoTools/Crypto/PRNG.h>
#include <cstdint>

using u64 = uint64_t;
struct Share { u64 a, b; };  // Rep-SSS: P_i持有与邻居的两段

struct PRSS {
    oc::PRNG left, right;     // 两把预共享的种子（与P_{i-1}, P_{i+1}）
    u64 ctr = 0;
    Share rand() {            // 返回一份随机份额 ⟦α⟧
        return { left.get<u64>(), right.get<u64>() };
    }
};

#endif // PRSS_HPP