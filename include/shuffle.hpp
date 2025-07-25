template<typename T>
class Shuffle
{
public:

    static std::vector<int> generate_random_permutation(int n, oc::PRNG& prng) {
        std::vector<int> perm(n);
        for (int i = 0; i < n; ++i) perm[i] = i;
        for (int i = n - 1; i > 0; --i) {
            int j = prng.get<uint32_t>() % (i + 1);
            std::swap(perm[i], perm[j]);
        }
        return perm;
    }
};