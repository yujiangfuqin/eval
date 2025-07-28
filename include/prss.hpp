#ifndef PRSS_HPP
#define PRSS_HPP
#include <cryptoTools/Crypto/PRNG.h>
#include <cstdint>

using u64 = uint64_t;
struct Share { u64 a, b; };  // Rep-SSS: P_i持有与邻居的两段

struct PRSS {
    oc::PRNG left, right;     // 两把预共享的种子（与P_{i-1}, P_{i+1}）
    u64 ctr = 0;

    oc::PRNG& getLeftPRNG() { return left; }
    oc::PRNG& getRightPRNG() { return right; }
};

#endif // PRSS_HPP