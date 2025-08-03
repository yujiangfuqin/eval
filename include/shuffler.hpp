#include "config.hpp"
#include "prss.hpp"
#include "types.hpp"
#include "utils.hpp"

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

    template<typename U>
    std::vector<U> reveal(RepShare<std::vector<U>> rou, uint32_t size) { 
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
    U reveal(RepShare<U> idx, uint32_t size){
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

    template<typename U>
    void apply_permutation(RepShare<std::vector<U>>& a, RepShare<std::vector<uint32_t>> &rou, uint32_t size) { 
        RepShare<std::vector<uint32_t>> perms = generate(rou.shares[0].size());
        shuffle(a, perms, size);
        shuffle(rou, perms, rou.shares[0].size());

        std::vector<uint32_t> tmp = reveal(rou, rou.shares[0].size());
        apply_permutation_local(a.shares[0], tmp);
        apply_permutation_local(a.shares[1], tmp);
    }

    private:

    PRSS& prss;

};
