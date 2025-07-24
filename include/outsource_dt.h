#ifndef _OUT_SOURCING_H__
#define _OUT_SOURCING_H__

#include "dt_helper.h"
#include "timer.hpp"
#include "config.hpp"
#include "net.hpp"
#include "factory.hpp"

template <typename T> 
struct Node{
    uint32_t k; T t;
};
template <typename T>
struct Node_el{
    uint32_t nl,nr,xl,xr; 
    T t, v;
    
    uint32_t node_lens = 16;
    uint32_t data_lens = 16;
    Node_el& operator+=(const Node_el& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->nl = (this->nl + other.nl) % this->node_lens;
        this->nr = (this->nr + other.nr) % this->node_lens;
        this->xl = (this->xl + other.xl) % this->data_lens;
        this->xr = (this->xr + other.xr) % this->data_lens;
        this->t += other.t;
        this->v += other.v;
        return *this;
    }
    Node_el& operator-=(const Node_el& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->nl = (this->nl + this->node_lens - other.nl) % this->node_lens;
        this->nr = (this->nr + this->node_lens - other.nr) % this->node_lens;
        this->xl = (this->xl + this->data_lens - other.xl) % this->data_lens;
        this->xr = (this->xr + this->data_lens - other.xr) % this->data_lens;
        this->t -= other.t;
        this->v -= other.v;
        return *this;
    }
    const Node_el operator*(const uint32_t &other) const{
        Node_el<T> newnode;
        newnode.nl = (this->nl * other) % this->node_lens;
        newnode.nr = (this->nr * other) % this->node_lens;
        newnode.xl = (this->xl * other) % this->data_lens;
        newnode.xr = (this->xr * other) % this->data_lens;
        newnode.t = this->t * other;
        newnode.v = this->v * other;
        newnode.data_lens = this->data_lens;
        newnode.node_lens = this->node_lens;
        return newnode;
    }
    friend std::ostream & operator<<(std::ostream & out, Node_el A){
        out << (uint32_t)A.nl <<" "<< (uint32_t)A.nr <<" "<<(uint32_t)A.xl <<" "<<(uint32_t)A.xr <<" "<<(uint32_t)A.t <<" "<<A.v<<std::endl;
        return out;
    }
};
uint32_t find_deep(std::map<int,node*> tmp, uint32_t idx){
    uint32_t ldeep = 0, rdeep = 0;
    if(tmp[idx]->left != 0)
    ldeep = find_deep(tmp, tmp[idx]->left);
    if(tmp[idx]->right != 0)
    rdeep = find_deep(tmp, tmp[idx]->right);
    return std::max(ldeep, rdeep) + 1;


}
template <typename T, typename D> 
class Dt{
    private:
    
    std::string owner;
    std::string S[2];
    
    void from_path(std::string path){
        std::map<int,node*> tmp = read_from_file(path);
        deep = find_deep(tmp, 0);
        node_lens = tmp.size();
        Ps = (D*) malloc(sizeof(D) * (node_lens + 1));
        start_x = tmp[0]->idx;
        for(int i = 0; i < node_lens; i++){
            Ps[i].nl = tmp[i]->left;Ps[i].nr = tmp[i]->right;Ps[i].xl = tmp[tmp[i]->left]->idx;Ps[i].xr = tmp[tmp[i]->right]->idx;Ps[i].t = tmp[i]->weight;
            Ps[i].v = (tmp[i]->left == 0 && tmp[i]->right == 0 ? tmp[i]->weight:0);
            Ps[i].node_lens = node_lens+1;
            Ps[i].data_lens = data_lens;
            if(Ps[i].nl == 0) Ps[i].nl = node_lens;
            if(Ps[i].nr == 0) Ps[i].nr = node_lens;
        }
        Ps[node_lens].nl = node_lens;Ps[node_lens].nr = node_lens;
        Ps[node_lens].xl = 0;Ps[node_lens].xr = 0;Ps[node_lens].t = 0;Ps[node_lens].v = 0;
        Ps[node_lens].node_lens = node_lens+1;Ps[node_lens].data_lens = data_lens;
        node_lens++;
        free_tree(tmp);
    }
    void from_sim(uint32_t node_size_t, uint32_t deep_t, uint32_t data_len, bool cons){
        deep = deep_t;
        data_lens = data_len;
            node_lens = ( 1 << (deep_t - 1) ) - 1;
            v_lens = node_lens + 1;
            Ps = (D*) malloc(sizeof(D) * node_lens);
            vs = (T*) malloc(sizeof(T) * v_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(Ps), sizeof(D) * node_lens);
            for(int i = 0; i < v_lens; i++) vs[i] = i;
            for(int i = 0; i < node_lens; i ++){
                Ps[i].k %= data_len;
            }
    }
    void from_sim(uint32_t node_size_t, uint32_t deep_t, uint32_t data_len){
        data_lens = data_len;
        deep = deep_t;
            node_lens = ( 1 << deep_t ) - 1;
            if(node_size_t < node_lens) node_lens = node_size_t;
            Ps = (D*) malloc(sizeof(D) * (node_lens + 1));
            for(int i = 0; i < node_lens; i++){
                Ps[i].nl = i + 1;Ps[i].nr = i + 1;Ps[i].xl = 0;Ps[i].xr = 0;Ps[i].t = 0;
                Ps[i].v = 0;
                Ps[i].node_lens = node_lens;
                Ps[i].data_lens = data_lens;
            }
    }
    void despoit1(){
        if(Config::myconfig->check(owner)){
        
            D * Ps0, *Ps1;
            T *vs0, *vs1;
            Ps0 = (D *)malloc(sizeof(D) * node_lens);
            Ps1 = (D *)malloc(sizeof(D) * node_lens);
            vs0 = (T*) malloc(sizeof(T) * v_lens);
            vs1 = (T*) malloc(sizeof(T) * v_lens);
            for(int i = 0; i < node_lens; i++){
                RAND_bytes(reinterpret_cast<uint8_t*>(&Ps0[i].k), sizeof(T));
                RAND_bytes(reinterpret_cast<uint8_t*>(&Ps0[i].t), sizeof(T));
                Ps0[i].k %= data_lens;
                Ps1[i].k = (Ps[i].k - Ps0[i].k + data_lens) % data_lens;
                Ps1[i].t = (Ps[i].t - Ps0[i].t);
                
            }
            for(int i = 0; i < v_lens; i++){
                RAND_bytes(reinterpret_cast<uint8_t*>(&vs0[i]), sizeof(T));
                vs1[i] = vs[i] - vs0[i];
            }
            P2Pchannel::mychnl->send_data_to(S[0], Ps0, sizeof(D) * node_lens);
            P2Pchannel::mychnl->send_data_to(S[1], Ps1, sizeof(D) * node_lens);
            P2Pchannel::mychnl->send_data_to(S[0], vs0, sizeof(T) * v_lens);
            P2Pchannel::mychnl->send_data_to(S[1], vs1, sizeof(T) * v_lens);
            free(Ps0);free(Ps1);
            free(vs0);free(vs1);
        }else{
            for(int i = 0; i < 2; i++ ){
                if(Config::myconfig->check(S[i])){
                    P2Pchannel::mychnl->recv_data_from(owner, Ps, sizeof(D) * node_lens);
                    P2Pchannel::mychnl->recv_data_from(owner, vs, sizeof(T) * v_lens);
                }
            }
        }
    }
    void despoit2(){
        if(Config::myconfig->check(owner)){
            D * Ps0, *Ps1;
            uint32_t id[2];
            uint32_t x[2];
            Ps0 = (D *)malloc(sizeof(D) * node_lens);
            Ps1 = (D *)malloc(sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(Ps0), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(Ps1), sizeof(D) * node_lens);
            RAND_bytes(reinterpret_cast<uint8_t*>(id), 2*sizeof(uint32_t));
            RAND_bytes(reinterpret_cast<uint8_t*>(x), 2*sizeof(uint32_t));
            for(int i = 0; i < node_lens; i++){
                Ps0[i].nl %= node_lens;Ps0[i].nr %= node_lens;Ps1[i].nl %= node_lens;Ps1[i].nr %= node_lens;
                Ps0[i].xl %= data_lens;Ps0[i].xr %= data_lens;Ps1[i].xl %= data_lens;Ps1[i].xr %= data_lens;
                Ps[i].nl = (Ps[i].nl - Ps0[i].nl - Ps1[i].nl + 2*node_lens) % node_lens;
                Ps[i].nr = (Ps[i].nr - Ps0[i].nr - Ps1[i].nr + 2*node_lens) % node_lens;
                Ps[i].xl = (Ps[i].xl - Ps0[i].xl - Ps1[i].xl + 2*data_lens) % data_lens;
                Ps[i].xr = (Ps[i].xr - Ps0[i].xr - Ps1[i].xr + 2*data_lens) % data_lens;
                Ps[i].t = (Ps[i].t - Ps0[i].t - Ps1[i].t);
                Ps[i].v = (Ps[i].v - Ps0[i].v - Ps1[i].v);
                Ps[i].data_lens = Ps0[i].data_lens = Ps1[i].data_lens = data_lens;
                Ps[i].node_lens = Ps0[i].node_lens = Ps1[i].node_lens = node_lens;
                
            }
            id[0] %= node_lens;id[1] %= node_lens;
            x[0] %= data_lens;x[1] %= data_lens;
            start_n = (start_n - id[0] - id[1] + 2*node_lens) % node_lens;
            start_x = (start_x - x[0] - x[1] + 2*data_lens) % data_lens;
            /*0, 1 -> 0
              1, 2 -> 1
              2, 0 -> 2
            */
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  Ps0, sizeof(D) * node_lens);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  Ps1, sizeof(D) * node_lens);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &id[0], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(),  &x[0], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  Ps1, sizeof(D) * node_lens);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  Ps, sizeof(D) * node_lens);
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &id[1], sizeof(uint32_t));
           P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(),  &x[1], sizeof(uint32_t));
           memcpy(Ps_rep, Ps0, sizeof(D) * node_lens);
           free(Ps1);free(Ps0);
        }
        else{
            P2Pchannel::mychnl->recv_data_from(owner, Ps, sizeof(D) * node_lens);
            P2Pchannel::mychnl->recv_data_from(owner, Ps_rep, sizeof(D) * node_lens);
            P2Pchannel::mychnl->recv_data_from(owner, &start_n, sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(owner, &start_x, sizeof(uint32_t));
        }
    }
    bool inside_gen = false;
    public:
    D* Ps, *Ps_rep = nullptr;
    T * vs = nullptr;
    uint32_t data_lens, node_lens, v_lens, deep;
    uint32_t start_n = 0, start_x;
    Dt(std::string path, uint32_t data_lens, std::string owner, std::string S0, std::string S1):owner(owner), data_lens(data_lens){
        S[0] = S0; S[1] = S1;
        inside_gen = true;
        from_path(path);
        Ps_rep = (D *)malloc(sizeof(D) * node_lens); 
        despoit2();
    }
    Dt(uint32_t node_size_t, uint32_t deep_t, uint32_t data_len, bool cons, std::string owner, std::string S0, std::string S1):owner(owner){
        S[0] = S0; S[1] = S1;
        inside_gen = true;
        from_sim(node_size_t, deep_t, data_len, cons);
        despoit1();
        
    }
    Dt(uint32_t node_size_t, uint32_t deep_t, uint32_t data_len, std::string owner, std::string S0, std::string S1):owner(owner){
        S[0] = S0; S[1] = S1;
        inside_gen = true;
        from_sim(node_size_t, deep_t, data_len);
        Ps_rep = (D *)malloc(sizeof(D) * node_lens); 
        despoit2();
    }
    Dt(D * Ps, T * vs, uint32_t data_lens, uint32_t node_lens, uint32_t v_lens, std::string owner, std::string S0, std::string S1):Ps(Ps),vs(vs), data_lens(data_lens), node_lens(node_lens), v_lens(v_lens), owner(owner){
        S[0] = S0; S[1] = S1;
        
        despoit1();
    }
    Dt(D *Ps, uint32_t node_lens, uint32_t data_lens, uint32_t start_x, std::string owner, uint32_t deep, std::string S0, std::string S1):Ps(Ps), data_lens(data_lens), node_lens(node_lens), start_x(start_x), owner(owner), deep(deep){
        
        /*for Node_el*/
        Ps_rep = (D *)malloc(sizeof(D) * node_lens); 
        S[0] = S0; S[1] = S1;
        despoit2();
    }
    ~Dt(){
        if(Ps_rep != nullptr){
            free(Ps_rep);
        }
        if(vs != nullptr){
            free(vs);
        }
        if(inside_gen) free(Ps);
    }

};

#endif // _OUT_SOURCING_H__