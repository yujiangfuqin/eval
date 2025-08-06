#ifndef SHUFFLER_HPP
#define SHUFFLER_HPP

#include "config.hpp"
#include "prss.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <limits>

template<typename T>
class Shuffler
{
    public:

    // 构造函数，初始化prss引用
    Shuffler(PRSS& prss_) : prss(prss_) {}


    static std::vector<uint32_t> generate_random_permutation(uint32_t n, oc::PRNG& prng) {
        std::vector<uint32_t> perms(n);
        for (uint32_t i = 0; i < n; ++i) perms[i] = i;
        for (uint32_t i = n - 1; i > 0; --i) {
            uint32_t j = prng.get<uint32_t>() % (i + 1);
            std::swap(perms[i], perms[j]);
        }
        return perms;
    }

    RepShare<std::vector<uint32_t>> generate(uint32_t n) {
        return RepShare<std::vector<uint32_t>>(
            generate_random_permutation(n, prss.getLeftPRNG()),
            generate_random_permutation(n, prss.getRightPRNG())
        );
    }

    RepShare<std::vector<std::vector<uint32_t>>> generate(uint32_t rows, uint32_t cols) {
        RepShare<std::vector<std::vector<uint32_t>>> result;
        result.shares[0].resize(rows);
        result.shares[1].resize(rows);
        for (uint32_t i = 0; i < rows; ++i) {
            result.shares[0][i] = generate_random_permutation(cols, prss.getLeftPRNG());
            result.shares[1][i] = generate_random_permutation(cols, prss.getRightPRNG());
        }
        return result;
    }


    Node<T> random_node(bool pos, uint32_t size) {   // pos为0用左密钥
        Node<T> r;
        if(pos == 0){
            r.ln = prss.left.get<uint32_t>() % size; r.rn = prss.left.get<uint32_t>() % size;
            r.lf = prss.left.get<uint32_t>() % size; r.rf = prss.left.get<uint32_t>() % size;
            r.t = prss.left.get<T>();
        }
        else {
            r.ln = prss.right.get<uint32_t>() % size; r.rn = prss.right.get<uint32_t>() % size;
            r.lf = prss.right.get<uint32_t>() % size; r.rf = prss.right.get<uint32_t>() % size;
            r.t = prss.right.get<T>();
        }
        return r;
    }

    void reshare(RepShare<std::vector<Node<T>>> &a, uint32_t st, uint32_t size) {
        if(Config::myconfig->get_idex() == (st + 2) % 3) { // pre
            Node<T> r0 = random_node(0, size), r1 = random_node(0, size);
            for(int i = 0; i < a.shares[0].size(); i++) {
                a.shares[0][i] -= r0;
                a.shares[0][i] -= r1;
                a.shares[1][i] += r0;
            }
            P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(), a.shares[1]);
        }
        else if(Config::myconfig->get_idex() == (st + 1) % 3) { // suc
            Node<T> r0 = random_node(1, size), r1 = random_node(1, size);
            for(int i = 0; i < a.shares[0].size(); i++) {
                a.shares[0][i] += r1;
                a.shares[1][i] -= r0;
                a.shares[1][i] -= r1;
            }
            P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(), a.shares[0]);
        }
        else {
            P2Pchannel::mychnl->recv_vector(Config::myconfig->get_pre(), a.shares[0]);
            P2Pchannel::mychnl->recv_vector(Config::myconfig->get_suc(), a.shares[1]);
        }
    }
    template <typename U>
    void reshare(RepShare<std::vector<U>> &a, uint32_t st, uint32_t size) {
        std::vector<T> tmp0(a.shares[0].size()), tmp1(a.shares[1].size());
        if(Config::myconfig->get_idex() == (st + 2) % 3) {  // pre
            for(int i = 0; i < a.shares[0].size(); i++) {
                T r0 = prss.left.get<T>() % size, r1 = prss.left.get<T>() % size;

                a.shares[0][i] = (a.shares[0][i] - r0 - r1 + 2 * size) % size;
                a.shares[1][i] = (a.shares[1][i] + r0) % size;
            }
            P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(), a.shares[1]);
        }
        else if(Config::myconfig->get_idex() == (st + 1) % 3) { //suc
            for(int i = 0; i < a.shares[0].size(); i++) {
                T r0 = prss.right.get<T>() % size, r1 = prss.right.get<T>() % size;

                a.shares[0][i] = (a.shares[0][i]+ r1) % size;
                a.shares[1][i] = (a.shares[1][i] - r0 - r1 + 2 * size) % size;
            }
            P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(), a.shares[0]);
        }
        else {
            P2Pchannel::mychnl->recv_vector(Config::myconfig->get_pre(), a.shares[0]);
            P2Pchannel::mychnl->recv_vector(Config::myconfig->get_suc(), a.shares[1]);
        }
    }

    //向量版本
    template <typename U>
    void reshare(RepShare<std::vector<std::vector<U>>> &a, uint32_t st, uint32_t size) {
        if (a.shares[0].empty()) return;
        size_t num_rows = a.shares[0].size();
        size_t num_cols = a.shares[0][0].size();

        if(Config::myconfig->get_idex() == (st + 2) % 3) {  // pre
            for(size_t i = 0; i < num_rows; ++i) {
                for (size_t j = 0; j < num_cols; ++j) {
                    T r0 = prss.left.get<T>() % size, r1 = prss.left.get<T>() % size;
                    a.shares[0][i][j] = (a.shares[0][i][j] - r0 - r1 + 2 * size) % size;
                    a.shares[1][i][j] = (a.shares[1][i][j] + r0) % size;
                }
            }
            P2Pchannel::mychnl->send_vectors(Config::myconfig->get_suc(), a.shares[1]);
        }
        else if(Config::myconfig->get_idex() == (st + 1) % 3) { //suc
            for(size_t i = 0; i < num_rows; ++i) {
                for (size_t j = 0; j < num_cols; ++j) {
                    T r0 = prss.right.get<T>() % size, r1 = prss.right.get<T>() % size;
                    a.shares[0][i][j] = (a.shares[0][i][j] + r1) % size;
                    a.shares[1][i][j] = (a.shares[1][i][j] - r0 - r1 + 2 * size) % size;
                }
            }
            P2Pchannel::mychnl->send_vectors(Config::myconfig->get_pre(), a.shares[0]);
        }
        else {
            P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_pre(), a.shares[0]);
            P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_suc(), a.shares[1]);
        }
    }

    void reshare(RepShare<std::vector<std::vector<Node<T>>>> &a, uint32_t st, uint32_t size) {
        if (a.shares[0].empty()) return;
        size_t num_rows = a.shares[0].size();
        size_t num_cols = a.shares[0][0].size();

        if(Config::myconfig->get_idex() == (st + 2) % 3) { // pre
            for(size_t i = 0; i < num_rows; ++i) {
                for (size_t j = 0; j < num_cols; ++j) {
                    Node<T> r0 = random_node(0, size), r1 = random_node(0, size);
                    a.shares[0][i][j] -= r0;
                    a.shares[0][i][j] -= r1;
                    a.shares[1][i][j] += r0;
                }
            }
            P2Pchannel::mychnl->send_vectors(Config::myconfig->get_suc(), a.shares[1]);
        }
        else if(Config::myconfig->get_idex() == (st + 1) % 3) { // suc
            for(size_t i = 0; i < num_rows; ++i) {
                for (size_t j = 0; j < num_cols; ++j) {
                    Node<T> r0 = random_node(1, size), r1 = random_node(1, size);
                    a.shares[0][i][j] += r1;
                    a.shares[1][i][j] -= r0;
                    a.shares[1][i][j] -= r1;
                }
            }
            P2Pchannel::mychnl->send_vectors(Config::myconfig->get_pre(), a.shares[0]);
        }
        else {
            P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_pre(), a.shares[0]);
            P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_suc(), a.shares[1]);
        }
    }

    template<typename U>
    std::vector<U> reveal(RepShare<std::vector<U>> rou, T size) { 
        P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(), rou.shares[0]);
        P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(), rou.shares[1]);
        std::vector<U> tmp(rou.shares[0].size()),tmp1(rou.shares[1].size());
        P2Pchannel::mychnl->recv_vector(Config::myconfig->get_suc(), tmp1);
        P2Pchannel::mychnl->recv_vector(Config::myconfig->get_pre(), tmp);
        if(tmp == tmp1) {
            for(int i = 0; i < tmp.size(); i++) {
                tmp[i] = (tmp[i] + rou.shares[0][i] + rou.shares[1][i]) % size;
            }
        }
        else {
            throw std::runtime_error("Revealed permutations do not match");
        }
        return tmp;
    }

    template<typename U>
    std::vector<std::vector<U>> reveal(RepShare<std::vector<std::vector<U>>> rou, T size) {
        if (rou.shares[0].empty()) {
            return {};
        }
        P2Pchannel::mychnl->send_vectors(Config::myconfig->get_suc(), rou.shares[0]);
        P2Pchannel::mychnl->send_vectors(Config::myconfig->get_pre(), rou.shares[1]);
        
        size_t num_rows = rou.shares[0].size();
        size_t num_cols = rou.shares[0][0].size();

        std::vector<std::vector<U>> tmp(num_rows, std::vector<U>(num_cols));
        std::vector<std::vector<U>> tmp1(num_rows, std::vector<U>(num_cols));

        P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_suc(), tmp1);
        P2Pchannel::mychnl->recv_vectors(Config::myconfig->get_pre(), tmp);

        if(tmp == tmp1) {
            for(size_t i = 0; i < num_rows; i++) {
                for (size_t j = 0; j < num_cols; j++) {
                    tmp[i][j] = (tmp[i][j] + rou.shares[0][i][j] + rou.shares[1][i][j]) % size;
                }
            }
        }
        else {
            throw std::runtime_error("Revealed permutations do not match");
        }
        return tmp;
    }

    template<typename U>
    U reveal(RepShare<U> idx, T size){
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &idx.shares[0], sizeof(U));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &idx.shares[1], sizeof(U));
        U tmp, tmp1;
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &tmp1, sizeof(U));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &tmp, sizeof(U));
        if(tmp == tmp1) {
            tmp += idx.shares[0];
            tmp += idx.shares[1];
            tmp = tmp % size; 
            return tmp;
        }
        
        else {
            throw std::runtime_error("Revealed indices do not match");
        }
    }
    
    template<typename U>
    void shuffle(RepShare<std::vector<U>>& a, RepShare<std::vector<uint32_t>> perms , uint32_t size) {
        uint32_t n = a.shares[0].size();

        uint32_t idx = Config::myconfig->get_idex();

        for(int i = 0; i < 3 ;i++) {
            if(idx == (i + 2) % 3) { //pre
                apply_permutation_local(a.shares[0], perms.shares[1]);
                apply_permutation_local(a.shares[1], perms.shares[1]);
            }
            else if(idx == i) { //self
                apply_permutation_local(a.shares[0], perms.shares[0]);
                apply_permutation_local(a.shares[1], perms.shares[0]);
            }

            reshare(a, (i + 1) % 3, size);
        }

    }

    //向量版本
    template<typename U>
    void shuffle(RepShare<std::vector<std::vector<U>>>& a, RepShare<std::vector<std::vector<uint32_t>>>& perms, uint32_t size) {
        if (a.shares[0].empty()) return;
        uint32_t idx = Config::myconfig->get_idex();

        for(int i = 0; i < 3; i++) {
            if(idx == (i + 2) % 3) { //pre
                apply_permutation_local(a.shares[0], perms.shares[1]);
                apply_permutation_local(a.shares[1], perms.shares[1]);
            }
            else if(idx == i) { //self
                apply_permutation_local(a.shares[0], perms.shares[0]);
                apply_permutation_local(a.shares[1], perms.shares[0]);
            }
            reshare(a, (i + 1) % 3, size);
        }
    }

    // New shuffle overload: Apply a 1D permutation to each row of a 2D vector.
    template<typename U>
    void shuffle(RepShare<std::vector<std::vector<U>>>& a, RepShare<std::vector<uint32_t>>& perms, uint32_t size) {
        if (a.shares[0].empty()) return;
        uint32_t idx = Config::myconfig->get_idex();

        for(int i = 0; i < 3; i++) {
            if(idx == (i + 2) % 3) { //pre
                for(auto& row : a.shares[0]) {
                    apply_permutation_local(row, perms.shares[1]);
                }
                for(auto& row : a.shares[1]) {
                    apply_permutation_local(row, perms.shares[1]);
                }
            }
            else if(idx == i) { //self
                for(auto& row : a.shares[0]) {
                    apply_permutation_local(row, perms.shares[0]);
                }
                for(auto& row : a.shares[1]) {
                    apply_permutation_local(row, perms.shares[0]);
                }
            }
            reshare(a, (i + 1) % 3, size);
        }
    }

    template<typename U>
    void apply_permutation(RepShare<std::vector<U>>& a, RepShare<std::vector<uint32_t>> &rou, uint32_t size) { 
        RepShare<std::vector<uint32_t>> perms = generate(rou.shares[0].size());
        shuffle(a, perms, size);
        shuffle(rou, perms, rou.shares[0].size());

        std::vector<uint32_t> tmp = reveal(rou, rou.shares[0].size());
        apply_permutation_local(a.shares[0], tmp);
        apply_permutation_local(a.shares[1], tmp);
    }

    template<typename U>
    void apply_permutation(RepShare<std::vector<std::vector<U>>>& a, RepShare<std::vector<std::vector<uint32_t>>>& rou, uint32_t size) {
        if (a.shares[0].empty()) return;
        uint32_t num_rows = a.shares[0].size();
        uint32_t num_cols = a.shares[0][0].size();

        RepShare<std::vector<std::vector<uint32_t>>> perms = generate(num_rows, num_cols);
        shuffle(a, perms, size);
        shuffle(rou, perms, num_cols);

        std::vector<std::vector<uint32_t>> tmp = reveal(rou, num_cols);
        apply_permutation_local(a.shares[0], tmp);
        apply_permutation_local(a.shares[1], tmp);
    }

    // New apply_permutation overload: Apply a 1D permutation rule to each row of a 2D vector.
    template<typename U>
    void apply_permutation_1D(RepShare<std::vector<std::vector<U>>>& a, RepShare<std::vector<uint32_t>>& rou, uint32_t size) {
        if (a.shares[0].empty()) return;
        uint32_t num_cols = a.shares[0][0].size();

        RepShare<std::vector<uint32_t>> perms = generate(num_cols);
        
        // We need to shuffle each row of 'a' with the same permutation 'perms'.
        // The new shuffle overload handles this.
        shuffle(a, perms, size);
        
        // The permutation rule 'rou' is also shuffled with 'perms'.
        shuffle(rou, perms, rou.shares[0].size());

        std::vector<uint32_t> tmp = reveal(rou, rou.shares[0].size());
        
        // Apply the revealed permutation to each row of 'a'.
        for(auto& row : a.shares[0]) {
            apply_permutation_local(row, tmp);
        }
        for(auto& row : a.shares[1]) {
            apply_permutation_local(row, tmp);
        }
    }

    private:

    PRSS& prss;

};

#endif // SHUFFLER_HPP