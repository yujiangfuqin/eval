#ifndef _DT_H
#define _DT_H

#include "dt_helper.hpp"
#include "timer.hpp"
#include "config.hpp"
#include "net.hpp"
#include "factory.hpp"
#include <vector>

template <typename T> 
struct Node_orig{
    uint32_t k; T t;
};
template <typename T>
struct Node{
    uint32_t ln, rn, lf, rf; 
    T t;

    uint32_t node_lens = 16;
    uint32_t data_lens = 16;

    Node& operator+=(const Node& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->ln = (this->ln + other.ln) % this->node_lens;
        this->rn = (this->rn + other.rn) % this->node_lens;
        this->lf = (this->lf + other.lf) % this->data_lens;
        this->rf = (this->rf + other.rf) % this->data_lens;
        this->t += other.t;
        return *this;
    }
    Node& operator-=(const Node& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->ln = (this->ln + this->node_lens - other.ln) % this->node_lens;
        this->rn = (this->rn + this->node_lens - other.rn) % this->node_lens;
        this->lf = (this->lf + this->data_lens - other.lf) % this->data_lens;
        this->rf = (this->rf + this->data_lens - other.rf) % this->data_lens;
        this->t -= other.t;
        return *this;
    }
    const Node operator*(const uint32_t &other) const{
        Node<T> newnode;
        newnode.ln = (this->ln * other) % this->node_lens;
        newnode.rn = (this->rn * other) % this->node_lens;
        newnode.lf = (this->lf * other) % this->data_lens;
        newnode.rf = (this->rf * other) % this->data_lens;
        newnode.t = this->t * other;
        newnode.data_lens = this->data_lens;
        newnode.node_lens = this->node_lens;
        return newnode;
    }
    friend std::ostream & operator<<(std::ostream & out, Node A){
        out << (uint32_t)A.ln <<" "<< (uint32_t)A.rn <<" "<<(uint32_t)A.lf <<" "<<(uint32_t)A.rf <<" "<<(uint32_t)A.t <<" "<<std::endl;
        return out;
    }
};

template <typename T, typename D>
class Dt{
    private:
    
    std::string owner;
    std::string S[2];
    
    void from_path(std::string path){
        std::map<int,node*> tmp = read_from_file(path);
        deep = find_deep(tmp, 0);
        min_deep = find_min_deep(tmp, 0);
        node_lens = tmp.size();
        uint32_t total_nodes = node_lens + deep - min_deep;
        Ps.shares[0].resize(total_nodes);
        start_x = tmp.at(0)->idx;

        for(uint32_t i = 0; i < node_lens; i++){
            auto const& current_node = tmp.at(i);
            Ps.shares[0][i].ln = current_node->left; Ps.shares[0][i].rn = current_node->right;
            Ps.shares[0][i].lf = tmp.at(current_node->left)->idx; Ps.shares[0][i].rf = tmp.at(current_node->right)->idx;
            Ps.shares[0][i].node_lens = total_nodes; Ps.shares[0][i].data_lens = data_lens;
            if(Ps.shares[0][i].ln == 0) Ps.shares[0][i].ln = node_lens;
            if(Ps.shares[0][i].rn == 0) Ps.shares[0][i].rn = node_lens;
        }
        
        cal_t(Ps.shares[0].data(), tmp, 0, 0);

        for(uint32_t i = node_lens; i < total_nodes; i++){
            Ps.shares[0][i].ln = i + 1; Ps.shares[0][i].rn = i + 1; Ps.shares[0][i].lf = 0; Ps.shares[0][i].rf = 0; Ps.shares[0][i].t = 0;
            Ps.shares[0][i].node_lens = total_nodes; Ps.shares[0][i].data_lens = data_lens;
        }

        Ps.shares[0][total_nodes - 1].ln = total_nodes - 1; Ps.shares[0][total_nodes - 1].rn = total_nodes - 1;
        node_lens = total_nodes;

        Ps.shares[0].resize(node_lens); 
        Ps.shares[1].resize(node_lens);
        uint32_t per_size = node_lens * 2 + 1;

        perm.shares[0].resize(per_size);
        perm.shares[1].resize(per_size);
        free_tree(tmp);
    }

    void genPerm(){
        uint32_t point = 0;
        uint32_t end = node_lens * 2;

        std::vector<uint32_t> node_indices;
        for(uint32_t i = 0; i < node_lens; i++){
            node_indices.push_back(Ps.shares[0][i].ln);
            node_indices.push_back(Ps.shares[0][i].rn);
        }
        std::vector<uint32_t> aux = cal_aux(node_indices, node_lens);
        int repeat_count = 0;
       
        for(int i = 0 ; i < node_lens; i++){
            repeat_count = aux[i];
            perm.shares[0].zeta[i] = point;
            point++;
            for (int counter = 1; counter <= repeat_count; ++counter) {
                perm.shares[0].zeta[end] = point;
                end--;
                point++;
            }
        }

        point = 1; end = node_lens - deep + min_deep;
        perm.shares[0].sigma[0] = 0;

        for(int i = 0; i < node_lens * 2; i++){
            if(node_indices[i] < node_lens - deep + min_deep){
                perm.shares[0].sigma[node_indices[i]] = point;
                point++;
            }
            else{
                perm.shares[0].sigma[end] = point;
                point++;
                end++;
            }
        }
        // 生成permutation sigma
        // 生成permutation zeta_f
        // 生成permutation sigma_f
    }

    void deposit(){
        if(Config::myconfig->check(owner)){
            std::vector<D> Ps0(node_lens), Ps1(node_lens);
            Perm perm0, perm1;
            perm0.resize(node_lens*2+1);
            perm1.resize(node_lens*2+1);
            uint32_t id[2], x[2];
            
            genPerm();

            RAND_bytes(reinterpret_cast<uint8_t*>(Ps0.data()), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(Ps1.data()), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(perm0.zeta.data()), sizeof(uint32_t) * perm0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm1.zeta.data()), sizeof(uint32_t) * perm1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm0.sigma.data()), sizeof(uint32_t) * perm0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm1.sigma.data()), sizeof(uint32_t) * perm1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm0.zeta_f.data()), sizeof(uint32_t) * perm0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm1.zeta_f.data()), sizeof(uint32_t) * perm1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm0.sigma_f.data()), sizeof(uint32_t) * perm0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(perm1.sigma_f.data()), sizeof(uint32_t) * perm1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(id), 2*sizeof(uint32_t));
            RAND_bytes(reinterpret_cast<uint8_t*>(x), 2*sizeof(uint32_t));
            for(int i = 0; i < node_lens; i++){
                Ps0[i].ln %= node_lens;Ps0[i].rn %= node_lens;Ps1[i].ln %= node_lens;Ps1[i].rn %= node_lens;
                Ps0[i].lf %= data_lens;Ps0[i].rf %= data_lens;Ps1[i].lf %= data_lens;Ps1[i].rf %= data_lens;
                Ps.shares[0][i].ln = (Ps.shares[0][i].ln - Ps0[i].ln - Ps1[i].ln + 2*node_lens) % node_lens;
                Ps.shares[0][i].rn = (Ps.shares[0][i].rn - Ps0[i].rn - Ps1[i].rn + 2*node_lens) % node_lens;
                Ps.shares[0][i].lf = (Ps.shares[0][i].lf - Ps0[i].lf - Ps1[i].lf + 2*data_lens) % data_lens;
                Ps.shares[0][i].rf = (Ps.shares[0][i].rf - Ps0[i].rf - Ps1[i].rf + 2*data_lens) % data_lens;
                Ps.shares[0][i].t = (Ps.shares[0][i].t - Ps0[i].t - Ps1[i].t);
                Ps.shares[0][i].data_lens = Ps0[i].data_lens = Ps1[i].data_lens = data_lens;
                Ps.shares[0][i].node_lens = Ps0[i].node_lens = Ps1[i].node_lens = node_lens;
                
            }
            for(int i = 0; i < node_lens*2+1; i++){
                perm0.zeta[i] %= node_lens*2+1; perm1.zeta[i] %= node_lens*2+1;
                perm0.sigma[i] %= node_lens*2+1; perm1.sigma[i] %= node_lens*2+1;
                perm0.zeta_f[i] %= node_lens*2+1; perm1.zeta_f[i] %= node_lens*2+1;
                perm0.sigma_f[i] %= node_lens*2+1; perm1.sigma_f[i] %= node_lens*2+1;
                perm.shares[0].zeta[i] = (perm.shares[0].zeta[i] - perm0.zeta[i] - perm1.zeta[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perm.shares[0].sigma[i] = (perm.shares[0].sigma[i] - perm0.sigma[i] - perm1.sigma[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perm.shares[0].zeta_f[i] = (perm.shares[0].zeta_f[i] - perm0.zeta_f[i] - perm1.zeta_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perm.shares[0].sigma_f[i] = (perm.shares[0].sigma_f[i] - perm0.sigma_f[i] - perm1.sigma_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
            }
            id[0] %= node_lens;id[1] %= node_lens;
            x[0] %= data_lens;x[1] %= data_lens;
            start_n = (start_n - id[0] - id[1] + 2*node_lens) % node_lens;
            start_x = (start_x - x[0] - x[1] + 2*data_lens) % data_lens;
            /*0, 1 -> 0
              1, 2 -> 1
              2, 0 -> 2
            */
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(),  Ps0);
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(),  Ps1);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_suc(),  perm0);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_suc(),  perm1);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &id[0], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &x[0], sizeof(uint32_t));
           
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps1);
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps.shares[0]);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_pre(),  perm1);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_pre(),  perm.shares[0]);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &id[1], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &x[1], sizeof(uint32_t));
           Ps.shares[1] = Ps0;
           perm.shares[1] = perm0;
        }
        else{
            P2Pchannel::mychnl->recv_vector(owner, Ps.shares[0]);
            P2Pchannel::mychnl->recv_vector(owner, Ps.shares[1]);

            node_lens = Ps.shares[0].size();
            uint32_t per_size = node_lens * 2 + 1;
            perm.shares[0].resize(per_size);
            perm.shares[1].resize(per_size);

            P2Pchannel::mychnl->recv_per(owner, perm.shares[0]);
            P2Pchannel::mychnl->recv_per(owner, perm.shares[1]);
            P2Pchannel::mychnl->recv_data_from(owner, &start_n, sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(owner, &start_x, sizeof(uint32_t));
        }
    }

    bool inside_gen = false;

    public:
    RepShare<std::vector<D>> Ps;
    T * vs = nullptr;
    RepShare<Perm> perm;
    uint32_t data_lens, node_lens, deep, min_deep;
    uint32_t start_n = 0, start_x;

    Dt(std::string path, uint32_t data_lens, std::string owner, std::string S0, std::string S1):owner(owner), data_lens(data_lens){
        S[0] = S0; S[1] = S1;
        inside_gen = true;
        
        if(Config::myconfig->check(owner)){
            from_path(path);
        }

        deposit();
    }


    ~Dt(){
        if(vs != nullptr){
            free(vs);
        }
    }

};


#endif // _DT_H