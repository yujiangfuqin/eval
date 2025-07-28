template<typename T>
class Shuffle
{
    public:

    // 构造函数，初始化prss引用
    Shuffle(PRSS& prss_) : prss(prss_) {}

    // template<typename T>
    // std::vector<T> secure_shuffle(const std::vector<T>& a) {
    //     int n = input.size();
    //     if (n <= 1) return input; // 如果输入长度小于等于1，直接返回

    //     // 生成随机置换
    //     auto perms = generate(n);
    //     std::vector<T> b(n);
        
    //     // 使用生成的置换对输入进行洗牌
    //     for (int i = 0; i < n; ++i) {
    //         b[perms[0][i]] = a[i];
    //         b[perms[1][i]] = a[i];
    //     }
        
    //     return b;
    // }


    static std::vector<int> generate_random_permutation(int n, oc::PRNG& prng) {
        std::vector<int> perm(n);
        for (int i = 0; i < n; ++i) perm[i] = i;
        for (int i = n - 1; i > 0; --i) {
            int j = prng.get<uint32_t>() % (i + 1);
            std::swap(perm[i], perm[j]);
        }
        return perm;
    }

    RepShare<std::vector<int>> generate(int n) {
        return RepShare<std::vector<int>>(
            generate_random_permutation(n, prss.getLeftPRNG()),
            generate_random_permutation(n, prss.getRightPRNG())
        );
    }

    private:

    PRSS& prss;

};