#ifndef EVALV_HPP
#define EVALV_HPP

#include "dt.hpp"
#include "shuffler.hpp"

template<typename T>
class DtEvalv{
    private:
    Dt<T, Node<T>> * dt;
    Shuffler<T> shuffler;
    RepShare<std::vector<std::vector<T>>> features;
    RepShare<std::vector<std::vector<uint32_t>>> perms_n, perms_f;
    uint64_t data_nums;
    RepShare<std::vector<std::vector<Node<T>>>> Ps_list;
    std::vector<uint32_t> idx_nv, idx_fv;
    public:

    DtEvalv(Dt<T, Node<T>> * dt, Shuffler<T> shuffler, RepShare<std::vector<std::vector<T>>> features):dt(dt), shuffler(shuffler), features(features){
        if (!features.shares[0].empty()) {
            this->data_nums = features.shares[0].size();
        } else {
            this->data_nums = 0;
        }
        perms_n = this->shuffler.generate(data_nums, dt->node_lens);
        perms_f = this->shuffler.generate(data_nums, dt->data_lens);
        Ps_list.shares[0].resize(data_nums, std::vector<Node<T>>(dt->node_lens));
        Ps_list.shares[1].resize(data_nums, std::vector<Node<T>>(dt->node_lens));
        this->idx_nv.resize(data_nums, 0);
        this->idx_fv.resize(data_nums, 0);
    }

    void offline(){
        // P2Pchannel::mychnl->set_flush(false);

        RepShare<std::vector<std::vector<uint32_t>>> rep_perms_n;
        rep_perms_n.shares[0].resize(data_nums, std::vector<uint32_t>(dt->node_lens, 0));
        rep_perms_n.shares[1].resize(data_nums, std::vector<uint32_t>(dt->node_lens, 0));

        if(Config::myconfig->get_idex() == 0) {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->node_lens; ++j) {
                    rep_perms_n.shares[0][i][j] = i; // 1,2,3,...
                    rep_perms_n.shares[1][i][j] = 0;
                }
            }
        } else if(Config::myconfig->get_idex() == 2) {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->node_lens; ++j) {
                    rep_perms_n.shares[0][i][j] = 0;
                    rep_perms_n.shares[1][i][j] = i;
                }
            }
        } else {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->node_lens; ++j) {
                    rep_perms_n.shares[0][i][j] = 0;
                    rep_perms_n.shares[1][i][j] = 0;
                }
            }
        }

        RepShare<std::vector<std::vector<uint32_t>>> rep_perms_f;
        rep_perms_f.shares[0].resize(data_nums, std::vector<uint32_t>(dt->data_lens, 0));
        rep_perms_f.shares[1].resize(data_nums, std::vector<uint32_t>(dt->data_lens, 0));

        if(Config::myconfig->get_idex() == 0) {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->data_lens; ++j) {
                    rep_perms_f.shares[0][i][j] = i; // 1,2,3,...
                    rep_perms_f.shares[1][i][j] = 0;
                }
            }
        } else if(Config::myconfig->get_idex() == 2) {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->data_lens; ++j) {
                    rep_perms_f.shares[0][i][j] = 0;
                    rep_perms_f.shares[1][i][j] = i;
                }
            }
        } else {
            for(size_t i = 0; i < data_nums; ++i) {
                for(size_t j = 0; j < dt->data_lens; ++j) {
                    rep_perms_f.shares[0][i][j] = 0;
                    rep_perms_f.shares[1][i][j] = 0;
                }
            }
        }

        shuffler.shuffle(rep_perms_f, perms_f, dt->data_lens);

        shuffler.shuffle(rep_perms_n, perms_n, dt->node_lens);

        //行数为data_nums
        RepShare<std::vector<std::vector<uint32_t>>> delta_n{
            std::vector<std::vector<uint32_t>>(data_nums, std::vector<uint32_t>(2 * dt->node_lens + 1, 0)),std::vector<std::vector<uint32_t>>(data_nums, std::vector<uint32_t>(2 * dt->node_lens + 1, 0))};
        RepShare<std::vector<std::vector<uint32_t>>> delta_f{
            std::vector<std::vector<uint32_t>>(data_nums, std::vector<uint32_t>(2 * dt->node_lens + 1, 0)),std::vector<std::vector<uint32_t>>(data_nums, std::vector<uint32_t>(2 * dt->node_lens + 1, 0))};

        for(int i = 0; i < data_nums; i++) {
            delta_n.shares[0][i][0] = rep_perms_n.shares[0][i][0] % dt->node_lens; delta_n.shares[1][i][0] = rep_perms_n.shares[1][i][0] % dt->node_lens;
            for(int j = 1; j < dt->node_lens; j++){
                delta_n.shares[0][i][j] =(rep_perms_n.shares[0][i][j] - rep_perms_n.shares[0][i][j-1] + dt->node_lens) % dt->node_lens; 
                delta_n.shares[1][i][j] = (rep_perms_n.shares[1][i][j] - rep_perms_n.shares[1][i][j-1] + dt->node_lens) % dt->node_lens;
            }
        }
        for(int i = 0; i < data_nums; i++) {
            delta_f.shares[0][i][0] = rep_perms_f.shares[0][i][0] % dt->data_lens; delta_f.shares[1][i][0] = rep_perms_f.shares[1][i][0] % dt->data_lens;       
            for(int j = 1; j < dt->data_lens; j++){ 
                delta_f.shares[0][i][j] = (rep_perms_f.shares[0][i][j] - rep_perms_f.shares[0][i][j-1]+ dt->data_lens) % dt->data_lens;
                delta_f.shares[1][i][j] = (rep_perms_f.shares[1][i][j] - rep_perms_f.shares[1][i][j-1]+ dt->data_lens) % dt->data_lens;
            }
        }

        RepShare<std::vector<uint32_t>> tmp_p{
            dt->perms.shares[0].zeta, dt->perms.shares[1].zeta
        };

        shuffler.apply_permutation_1D(delta_n, tmp_p, dt->node_lens);

        //对delta_n求 前缀和
        for(int i = 0; i < data_nums; i++) {
            for(int j = 1; j < 2 * dt->node_lens + 1; j++){
                delta_n.shares[0][i][j] = (delta_n.shares[0][i][j] + delta_n.shares[0][i][j-1]) % dt->node_lens;
                delta_n.shares[1][i][j] = (delta_n.shares[1][i][j] + delta_n.shares[1][i][j-1]) % dt->node_lens;
            }
        }

        tmp_p.shares[0] = dt->perms.shares[0].sigma; tmp_p.shares[1] = dt->perms.shares[1].sigma;

        shuffler.apply_permutation_1D(delta_n, tmp_p, dt->node_lens);

        tmp_p.shares[0] = dt->perms.shares[0].zeta_f; tmp_p.shares[1] = dt->perms.shares[1].zeta_f;

        shuffler.apply_permutation_1D(delta_f, tmp_p, dt->data_lens);

        //对delta_f求 前缀和
        for(int i = 0; i < data_nums; i++) {
            for(int j = 1; j < 2 * dt->data_lens + 1; j++){
                delta_f.shares[0][i][j] = (delta_f.shares[0][i][j] + delta_f.shares[0][i][j-1]) % dt->data_lens;
                delta_f.shares[1][i][j] = (delta_f.shares[1][i][j] + delta_f.shares[1][i][j-1]) % dt->data_lens;
            }
        }
        tmp_p.shares[0] = dt->perms.shares[0].sigma_f;tmp_p.shares[1] = dt->perms.shares[1].sigma_f;

        shuffler.apply_permutation_1D(delta_f, tmp_p, dt->data_lens);

        for(int i = 0; i < data_nums; i++) {
            for(int j = 0; j < dt->node_lens; j++) {
                Ps_list.shares[0][i][j].ln = delta_n.shares[0][i][2*j+1]; Ps_list.shares[1][i][j].ln = delta_n.shares[1][i][2*j+2];
                Ps_list.shares[0][i][j].rn = delta_n.shares[0][i][j]; Ps_list.shares[1][i][j].rn = delta_n.shares[1][i][2*j+2];
                Ps_list.shares[0][i][j].lf = delta_f.shares[0][i][2*j+1]; Ps_list.shares[1][i][j].lf = delta_f.shares[1][i][2*j+2];
                Ps_list.shares[0][i][j].rf = delta_f.shares[0][i][2*j+1]; Ps_list.shares[1][i][j].rf = delta_f.shares[1][i][2*j+2];
            }
        }

        shuffler.shuffle(Ps_list, perms_n, data_nums);

        RepShare<std::vector<uint32_t>> tmp_n, tmp_f;
        tmp_n.shares[0].resize(data_nums, 0);
        tmp_n.shares[1].resize(data_nums, 0);
        tmp_f.shares[0].resize(data_nums, 0);
        tmp_f.shares[1].resize(data_nums, 0);
        for(int i = 0; i < data_nums; i++) {    
            tmp_n.shares[0][i] = delta_n.shares[0][i][0]; tmp_n.shares[1][i] = delta_n.shares[1][i][0];
            tmp_f.shares[0][i] = delta_f.shares[0][i][0]; tmp_f.shares[1][i] = delta_f.shares[1][i][0];
        }

        idx_nv = shuffler.reveal(tmp_n, dt->node_lens);
        idx_fv = shuffler.reveal(tmp_f, dt->data_lens);
    }


    std::vector<T> online(){
        RepShare<std::vector<T>> cur_feature, R;
        RepShare<std::vector<Node<T>>> cur_node;
        RepShare<std::vector<uint32_t>> ln_share, rn_share, lf_share, rf_share;
        RepShare<std::vector<T>> t_share;
        cur_feature.shares[0].resize(data_nums); cur_feature.shares[1].resize(data_nums);
        cur_node.shares[0].resize(data_nums); cur_node.shares[1].resize(data_nums);
        ln_share.shares[0].resize(data_nums, 0); ln_share.shares[1].resize(data_nums, 0);
        rn_share.shares[0].resize(data_nums, 0); rn_share.shares[1].resize(data_nums, 0);
        lf_share.shares[0].resize(data_nums, 0); lf_share.shares[1].resize(data_nums, 0);
        rf_share.shares[0].resize(data_nums, 0); rf_share.shares[1].resize(data_nums, 0);
        t_share.shares[0].resize(data_nums, 0); t_share.shares[1].resize(data_nums, 0);

        shuffler.shuffle(features, perms_f, dt->data_lens);

        R.shares[0].resize(data_nums, 0);
        R.shares[1].resize(data_nums, 0);

        for(int i = 0; i < dt->depth; i++){
            for(int j = 0; j < data_nums; j++){
                cur_node.shares[0][j] = Ps_list.shares[0][j][idx_nv[j]];
                cur_node.shares[1][j] = Ps_list.shares[1][j][idx_nv[j]];
                cur_feature.shares[0][j] = features.shares[0][j][idx_fv[j]];
                cur_feature.shares[1][j] = features.shares[1][j][idx_fv[j]];

                ln_share.shares[0][j] = cur_node.shares[0][j].ln; ln_share.shares[1][j] = cur_node.shares[1][j].ln;
                rn_share.shares[0][j] = cur_node.shares[0][j].rn; rn_share.shares[1][j] = cur_node.shares[1][j].rn;
                lf_share.shares[0][j] = cur_node.shares[0][j].lf; lf_share.shares[1][j] = cur_node.shares[1][j].lf;
                rf_share.shares[0][j] = cur_node.shares[0][j].rf; rf_share.shares[1][j] = cur_node.shares[1][j].rf;
                t_share.shares[0][j] = cur_node.shares[0][j].t; t_share.shares[1][j] = cur_node.shares[1][j].t;
            
                R.shares[0][j] += t_share.shares[0][j];
                R.shares[1][j] += t_share.shares[1][j];
            }
        }
        return shuffler.reveal(R, std::numeric_limits<T>::max());
    }
};

#endif // EVALV_HPP