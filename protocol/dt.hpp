#ifndef _DT_H
#define _DT_H

#include "dt_helper.hpp"
#include "timer.hpp"
#include "config.hpp"
#include "net.hpp"
#include "types.hpp"
#include <vector>
#include <openssl/rand.h>

template <typename T, typename D>
class Dt{
    private:   
    std::string owner;

    void from_path(std::string path){
        std::map<int,node*> tmp = read_from_file(path);
        depth = find_deep(tmp, 0);
        min_depth = find_min_deep(tmp, 0);
        node_lens = tmp.size();
        uint32_t total_nodes = node_lens + depth - min_depth;
        Ps.shares[0].resize(total_nodes);
        idx_f = tmp.at(0)->idx;

        for(uint32_t i = node_lens; i < total_nodes; i++){
            Ps.shares[0][i].ln = i + 1; Ps.shares[0][i].rn = i + 1; Ps.shares[0][i].lf = data_lens-1; Ps.shares[0][i].rf = data_lens-1; Ps.shares[0][i].t = 0;
            Ps.shares[0][i].node_lens = total_nodes; Ps.shares[0][i].data_lens = data_lens;
        }
        for(uint32_t i = 0; i < node_lens; i++){
            auto const& current_node = tmp.at(i);
            Ps.shares[0][i].ln = current_node->left; Ps.shares[0][i].rn = current_node->right;
            Ps.shares[0][i].lf = (current_node->left == 0 ) ? (data_lens - 1) : tmp.at(current_node->left)->idx;
            Ps.shares[0][i].rf = (current_node->right == 0 ) ? (data_lens - 1) : tmp.at(current_node->right)->idx;
            Ps.shares[0][i].node_lens = total_nodes; Ps.shares[0][i].data_lens = data_lens;
            if(Ps.shares[0][i].ln == 0) Ps.shares[0][i].ln = node_lens;
            if(Ps.shares[0][i].rn == 0) Ps.shares[0][i].rn = node_lens;
        }



        cal_t(Ps.shares[0].data(), tmp, 0, 0);


        Ps.shares[0][total_nodes - 1].ln = total_nodes - 1; Ps.shares[0][total_nodes - 1].rn = total_nodes - 1;
        node_lens = total_nodes;

        Ps.shares[0].resize(node_lens); 
        Ps.shares[1].resize(node_lens);
        uint32_t per_size = node_lens * 2 + 1;

        perms.shares[0].resize(per_size);
        perms.shares[1].resize(per_size);
        free_tree(tmp);
    }

    void genPerm(){
        uint32_t point = 0;
        uint32_t end = node_lens * 2;

        std::vector<uint32_t> node_indices, features_indices;
        for(uint32_t i = 0; i < node_lens; i++){
            node_indices.push_back(Ps.shares[0][i].ln);
            node_indices.push_back(Ps.shares[0][i].rn);
        }
        std::vector<uint32_t> aux = cal_aux(node_indices, node_lens);
        int repeat_count = 0;
       
        for(int i = 0 ; i < node_lens; i++){
            repeat_count = aux[i];
            perms.shares[0].zeta[i] = point;
            point++;
            for (int counter = 1; counter <= repeat_count; ++counter) {
                perms.shares[0].zeta[end] = point;
                end--;
                point++;
            }
        }

        point = 1; end = node_lens - depth + min_depth;
        perms.shares[0].sigma[0] = 0;

        for(int i = 0; i < node_lens * 2; i++){
            if(node_indices[i] < node_lens - depth + min_depth){
                perms.shares[0].sigma[node_indices[i]] = point;
                point++;
            }
            else{
                perms.shares[0].sigma[end] = point;
                point++;
                end++;
            }
        }

        for(uint32_t i = 0; i < node_lens; i++){
            features_indices.push_back(Ps.shares[0][i].lf);
            features_indices.push_back(Ps.shares[0][i].rf);
        }
        std::vector<uint32_t> aux_f = cal_aux(features_indices, data_lens);
        aux_f[idx_f]++;
        std::cout << std::endl;

        point = 0; end = node_lens * 2;
        repeat_count = 0;
        for(int i = 0 ; i < data_lens; i++){
            repeat_count = aux_f[i];
            perms.shares[0].zeta_f[i] = point;
            point++;
            for (int counter = 1; counter <= repeat_count; ++counter) {
                perms.shares[0].zeta_f[end] = point;
                end--;
                point++;
            }
        }
        std::vector<uint32_t> pointers(data_lens, 0);
        pointers[0] = 0;
        for(int i = 1; i < data_lens; i++){
            pointers[i] = pointers[i - 1] + aux_f[i - 1] + 1;
        }
        perms.shares[0].sigma_f[pointers[idx_f]] = 0;
        pointers[idx_f]++;
        for(int i = 0; i < node_lens * 2; i++){
            perms.shares[0].sigma_f[pointers[features_indices[i]]] = i+1;
            pointers[features_indices[i]]++;
        }
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
                perms.shares[0].zeta[i] = (perms.shares[0].zeta[i] - perm0.zeta[i] - perm1.zeta[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perms.shares[0].sigma[i] = (perms.shares[0].sigma[i] - perm0.sigma[i] - perm1.sigma[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perms.shares[0].zeta_f[i] = (perms.shares[0].zeta_f[i] - perm0.zeta_f[i] - perm1.zeta_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
                perms.shares[0].sigma_f[i] = (perms.shares[0].sigma_f[i] - perm0.sigma_f[i] - perm1.sigma_f[i] + 2*(node_lens*2+1)) % (node_lens*2+1);
            }

           P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(),  Ps0);
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_suc(),  Ps1);
           P2Pchannel::mychnl->send_perm(Config::myconfig->get_suc(),  perm0);
           P2Pchannel::mychnl->send_perm(Config::myconfig->get_suc(),  perm1);
           
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps1);
           P2Pchannel::mychnl->send_vector(Config::myconfig->get_pre(),  Ps.shares[0]);
           P2Pchannel::mychnl->send_perm(Config::myconfig->get_pre(),  perm1);
           P2Pchannel::mychnl->send_perm(Config::myconfig->get_pre(),  perms.shares[0]);
           Ps.shares[1] = Ps0;
           perms.shares[1] = perm0;
        }
        else{
            P2Pchannel::mychnl->recv_vector(owner, Ps.shares[0]);
            P2Pchannel::mychnl->recv_vector(owner, Ps.shares[1]);

            node_lens = Ps.shares[0].size();
            uint32_t per_size = node_lens * 2 + 1;
            perms.shares[0].resize(per_size);
            perms.shares[1].resize(per_size);

            P2Pchannel::mychnl->recv_perm(owner, perms.shares[0]);
            P2Pchannel::mychnl->recv_perm(owner, perms.shares[1]);
        }
    }

    public:
    RepShare<std::vector<D>> Ps;
    RepShare<Perm> perms;
    uint32_t data_lens, node_lens, depth, min_depth;
    uint32_t idx_n = 0, idx_f = 0;

    Dt(std::string path, uint32_t data_lens, std::string owner):owner(owner), data_lens(data_lens){
        if(Config::myconfig->check(owner)){
            from_path(path);
        }

        deposit();
    }

    ~Dt(){
    }

};


#endif // _DT_H