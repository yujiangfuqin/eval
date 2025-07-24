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
        Ps.resize(total_nodes);
        start_x = tmp.at(0)->idx;

        for(uint32_t i = 0; i < node_lens; i++){
            auto const& current_node = tmp.at(i);
            Ps[i].ln = current_node->left; Ps[i].rn = current_node->right;
            Ps[i].lf = tmp.at(current_node->left)->idx; Ps[i].rf = tmp.at(current_node->right)->idx;
            Ps[i].node_lens = total_nodes; Ps[i].data_lens = data_lens;
            if(Ps[i].ln == 0) Ps[i].ln = node_lens;
            if(Ps[i].rn == 0) Ps[i].rn = node_lens;
        }
        
        cal_t(Ps.data(), tmp, 0, 0);

        for(uint32_t i = node_lens; i < total_nodes; i++){
            Ps[i].ln = i + 1; Ps[i].rn = i + 1; Ps[i].lf = 0; Ps[i].rf = 0; Ps[i].t = 0;
            Ps[i].node_lens = total_nodes; Ps[i].data_lens = data_lens;
        }

        Ps[total_nodes - 1].ln = total_nodes - 1; Ps[total_nodes - 1].rn = total_nodes - 1;
        node_lens = total_nodes;

        Ps_rep.resize(node_lens); 
        uint32_t per_size = node_lens * 2 + 1;
        per.resize(per_size);
        per_rep.resize(per_size);
        per.zeta.resize(per_size);
        free_tree(tmp);
    }

    void genPer(){
        uint32_t point = 0;
        uint32_t end = node_lens * 2;

        std::vector<uint32_t> node_indices;
        for(uint32_t i = 0; i < node_lens; i++){
            node_indices.push_back(Ps[i].ln);
            node_indices.push_back(Ps[i].rn);
        }
        std::vector<uint32_t> aux = cal_aux(node_indices, node_lens);
        int repeat_count = 0;
       
        for(int i = 0 ; i < node_lens; i++){
            repeat_count = aux[i];
            per.zeta[i] = point;
            point++;
            for (int counter = 1; counter <= repeat_count; ++counter) {
                per.zeta[end] = point;
                end--;
                point++;
            }
        }

        point = 1; end = node_lens - deep + min_deep;
        per.sigma[0] = 0;

        for(int i = 0; i < node_lens * 2; i++){
            if(node_indices[i] < node_lens - deep + min_deep){
                per.sigma[node_indices[i]] = point;
                point++;
            }
            else{
                per.sigma[end] = point;
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
            Per per0, per1;
            per0.resize(node_lens*2+1);
            per1.resize(node_lens*2+1);
            uint32_t id[2], x[2];
            
            genPer();

            RAND_bytes(reinterpret_cast<uint8_t*>(Ps0.data()), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(Ps1.data()), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(per0.zeta.data()), sizeof(uint32_t) * per0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per1.zeta.data()), sizeof(uint32_t) * per1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per0.sigma.data()), sizeof(uint32_t) * per0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per1.sigma.data()), sizeof(uint32_t) * per1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per0.zeta_f.data()), sizeof(uint32_t) * per0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per1.zeta_f.data()), sizeof(uint32_t) * per1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per0.sigma_f.data()), sizeof(uint32_t) * per0.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(per1.sigma_f.data()), sizeof(uint32_t) * per1.zeta.size());
            RAND_bytes(reinterpret_cast<uint8_t*>(id), 2*sizeof(uint32_t));
            RAND_bytes(reinterpret_cast<uint8_t*>(x), 2*sizeof(uint32_t));
            for(int i = 0; i < node_lens; i++){
                Ps0[i].ln %= node_lens;Ps0[i].rn %= node_lens;Ps1[i].ln %= node_lens;Ps1[i].rn %= node_lens;
                Ps0[i].lf %= data_lens;Ps0[i].rf %= data_lens;Ps1[i].lf %= data_lens;Ps1[i].rf %= data_lens;
                Ps[i].ln = (Ps[i].ln - Ps0[i].ln - Ps1[i].ln + 2*node_lens) % node_lens;
                Ps[i].rn = (Ps[i].rn - Ps0[i].rn - Ps1[i].rn + 2*node_lens) % node_lens;
                Ps[i].lf = (Ps[i].lf - Ps0[i].lf - Ps1[i].lf + 2*data_lens) % data_lens;
                Ps[i].rf = (Ps[i].rf - Ps0[i].rf - Ps1[i].rf + 2*data_lens) % data_lens;
                Ps[i].t = (Ps[i].t - Ps0[i].t - Ps1[i].t);
                Ps[i].data_lens = Ps0[i].data_lens = Ps1[i].data_lens = data_lens;
                Ps[i].node_lens = Ps0[i].node_lens = Ps1[i].node_lens = node_lens;
                
            }
            for(int i = 0; i < node_lens*2+1; i++){
                per0.zeta[i] %= node_lens*2+1; per1.zeta[i] %= node_lens*2+1;
                per0.sigma[i] %= node_lens*2+1; per1.sigma[i] %= node_lens*2+1;
                per0.zeta_f[i] %= node_lens*2+1; per1.zeta_f[i] %= node_lens*2+1;
                per0.sigma_f[i] %= node_lens*2+1; per1.sigma_f[i] %= node_lens*2+1;
                per.zeta[i] = (per.zeta[i] - per0.zeta[i] - per1.zeta[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                per.sigma[i] = (per.sigma[i] - per0.sigma[i] - per1.sigma[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                per.zeta_f[i] = (per.zeta_f[i] - per0.zeta_f[i] - per1.zeta_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                per.sigma_f[i] = (per.sigma_f[i] - per0.sigma_f[i] - per1.sigma_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
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
           P2Pchannel::mychnl->send_per(Config::myconfig->get_suc(),  per0);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_suc(),  per1);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &id[0], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &x[0], sizeof(uint32_t));
           
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps1);
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_pre(),  per1);
           P2Pchannel::mychnl->send_per(Config::myconfig->get_pre(),  per);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &id[1], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &x[1], sizeof(uint32_t));
           Ps_rep = Ps0;
        }
        else{
            P2Pchannel::mychnl->recv_vector(owner, Ps);
            P2Pchannel::mychnl->recv_vector(owner, Ps_rep);

            node_lens = Ps.size();
            uint32_t per_size = node_lens * 2 + 1;
            per.resize(per_size);
            per_rep.resize(per_size);

            P2Pchannel::mychnl->recv_per(owner, per);
            P2Pchannel::mychnl->recv_per(owner, per_rep);
            P2Pchannel::mychnl->recv_data_from(owner, &start_n, sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(owner, &start_x, sizeof(uint32_t));
        }
    }

    bool inside_gen = false;

    public:
    std::vector<D> Ps, Ps_rep;
    T * vs = nullptr;
    Per per, per_rep;
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