#ifndef EVAL_HPP
#define EVAL_HPP

#include "config.hpp"
#include "net.hpp"
#include "cot.hpp"
#include "dt.hpp"
#include "shuffler.hpp"

template <typename T> class DtEval {
  private:
    Dt<T, Node<T>> *dt;
    Shuffler<T> shuffler;
    RepShare<std::vector<T>> features;
    RepShare<std::vector<uint32_t>> perms_n, perms_f;
    cotp<T> cot_executor;
    PRSS &prss;

  public:
    DtEval(Dt<T, Node<T>> *dt, Shuffler<T> shuffler, RepShare<std::vector<T>> features, PRSS &prss_)
        : dt(dt), shuffler(shuffler), features(features), prss(prss_) {
        perms_n = this->shuffler.generate(dt->node_lens);
        perms_f = this->shuffler.generate(dt->data_lens);
    }

    void offline() {
        // P2Pchannel::mychnl->set_flush(false);
        cot_executor.cot_preprocess(prss);

        RepShare<std::vector<uint32_t>> rep_perms_n;
        rep_perms_n.shares[0].resize(dt->node_lens, 0);
        rep_perms_n.shares[1].resize(dt->node_lens, 0);

        // 三方复制式 secret sharing 初始化
        // 0方负责生成顺序向量并发送给2方
        if (Config::myconfig->get_idex() == 0) {
            for (size_t i = 0; i < dt->node_lens; ++i) {
                rep_perms_n.shares[0][i] = i; // 1,2,3,...
                rep_perms_n.shares[1][i] = 0;
            }
        } else if (Config::myconfig->get_idex() == 2) {
            for (size_t i = 0; i < dt->node_lens; ++i) {
                rep_perms_n.shares[0][i] = 0;
                rep_perms_n.shares[1][i] = i;
            }
        } else {
            for (size_t i = 0; i < dt->node_lens; ++i) {
                rep_perms_n.shares[0][i] = 0;
                rep_perms_n.shares[1][i] = 0;
            }
        }

        RepShare<std::vector<uint32_t>> rep_perms_f;
        rep_perms_f.shares[0].resize(dt->data_lens, 0);
        rep_perms_f.shares[1].resize(dt->data_lens, 0);

        // 三方复制式 secret sharing 初始化
        // 0方负责生成顺序向量并发送给2方
        if (Config::myconfig->get_idex() == 0) {
            for (size_t i = 0; i < dt->data_lens; ++i) {
                rep_perms_f.shares[0][i] = i; // 1,2,3,...
                rep_perms_f.shares[1][i] = 0;
            }
        } else if (Config::myconfig->get_idex() == 2) {
            for (size_t i = 0; i < dt->data_lens; ++i) {
                rep_perms_f.shares[0][i] = 0;
                rep_perms_f.shares[1][i] = i;
            }
        } else {
            for (size_t i = 0; i < dt->data_lens; ++i) {
                rep_perms_f.shares[0][i] = 0;
                rep_perms_f.shares[1][i] = 0;
            }
        }

        shuffler.shuffle(rep_perms_f, perms_f, dt->data_lens);

        shuffler.shuffle(rep_perms_n, perms_n, dt->node_lens);

        RepShare<std::vector<uint32_t>> delta_n{std::vector<uint32_t>(2 * dt->node_lens + 1, 0),
                                                std::vector<uint32_t>(2 * dt->node_lens + 1, 0)};
        RepShare<std::vector<uint32_t>> delta_f{std::vector<uint32_t>(2 * dt->node_lens + 1, 0),
                                                std::vector<uint32_t>(2 * dt->node_lens + 1, 0)};

        delta_n.shares[0][0] = rep_perms_n.shares[0][0] % dt->node_lens;
        delta_n.shares[1][0] = rep_perms_n.shares[1][0] % dt->node_lens;
        for (int i = 1; i < dt->node_lens; i++) {
            delta_n.shares[0][i] =
                (rep_perms_n.shares[0][i] - rep_perms_n.shares[0][i - 1] + dt->node_lens) %
                dt->node_lens;
            delta_n.shares[1][i] =
                (rep_perms_n.shares[1][i] - rep_perms_n.shares[1][i - 1] + dt->node_lens) %
                dt->node_lens;
        }

        delta_f.shares[0][0] = rep_perms_f.shares[0][0] % dt->data_lens;
        delta_f.shares[1][0] = rep_perms_f.shares[1][0] % dt->data_lens;
        for (int i = 1; i < dt->data_lens; i++) {
            delta_f.shares[0][i] =
                (rep_perms_f.shares[0][i] - rep_perms_f.shares[0][i - 1] + dt->data_lens) %
                dt->data_lens;
            delta_f.shares[1][i] =
                (rep_perms_f.shares[1][i] - rep_perms_f.shares[1][i - 1] + dt->data_lens) %
                dt->data_lens;
        }

        RepShare<std::vector<uint32_t>> tmp_p{dt->perms.shares[0].zeta, dt->perms.shares[1].zeta};

        shuffler.apply_permutation(delta_n, tmp_p, dt->node_lens);

        // 对delta_n求 前缀和
        for (int i = 1; i < 2 * dt->node_lens + 1; i++) {
            delta_n.shares[0][i] =
                (delta_n.shares[0][i] + delta_n.shares[0][i - 1]) % dt->node_lens;
            delta_n.shares[1][i] =
                (delta_n.shares[1][i] + delta_n.shares[1][i - 1]) % dt->node_lens;
        }

        tmp_p.shares[0] = dt->perms.shares[0].sigma;
        tmp_p.shares[1] = dt->perms.shares[1].sigma;
        shuffler.apply_permutation(delta_n, tmp_p, dt->node_lens);

        tmp_p.shares[0] = dt->perms.shares[0].zeta_f;
        tmp_p.shares[1] = dt->perms.shares[1].zeta_f;

        shuffler.apply_permutation(delta_f, tmp_p, dt->data_lens);

        // 对delta_f求 前缀和
        for (int i = 1; i < 2 * dt->node_lens + 1; i++) {
            delta_f.shares[0][i] =
                (delta_f.shares[0][i] + delta_f.shares[0][i - 1]) % dt->data_lens;
            delta_f.shares[1][i] =
                (delta_f.shares[1][i] + delta_f.shares[1][i - 1]) % dt->data_lens;
        }

        tmp_p.shares[0] = dt->perms.shares[0].sigma_f;
        tmp_p.shares[1] = dt->perms.shares[1].sigma_f;

        shuffler.apply_permutation(delta_f, tmp_p, dt->data_lens);

        for (int i = 0; i < dt->node_lens; i++) {
            dt->Ps.shares[0][i].ln = delta_n.shares[0][2 * i + 1];
            dt->Ps.shares[1][i].ln = delta_n.shares[1][2 * i + 2];
            dt->Ps.shares[0][i].rn = delta_n.shares[0][2 * i + 1];
            dt->Ps.shares[1][i].rn = delta_n.shares[1][2 * i + 2];
            dt->Ps.shares[0][i].lf = delta_f.shares[0][2 * i + 1];
            dt->Ps.shares[1][i].lf = delta_f.shares[1][2 * i + 2];
            dt->Ps.shares[0][i].rf = delta_f.shares[0][2 * i + 1];
            dt->Ps.shares[1][i].rf = delta_f.shares[1][2 * i + 2];
        }
        shuffler.shuffle(dt->Ps, perms_n, dt->node_lens);

        RepShare<uint32_t> tmp_n, tmp_f;
        tmp_n.shares[0] = delta_n.shares[0][0];
        tmp_n.shares[1] = delta_n.shares[1][0];
        tmp_f.shares[0] = delta_f.shares[0][0];
        tmp_f.shares[1] = delta_f.shares[1][0];

        dt->idx_n = shuffler.reveal(tmp_n, dt->node_lens);
        dt->idx_f = shuffler.reveal(tmp_f, dt->data_lens);
        // std::cout << "idx_n: " << dt->idx_n << ", idx_f: " << dt->idx_f << std::endl;
    }

    T online() {
        RepShare<T> cur_feature, R;
        RepShare<Node<T>> cur_node;
        RepShare<uint32_t> ln_share, rn_share, lf_share, rf_share;
        RepShare<T> t_share;

        shuffler.shuffle(features, perms_f, dt->data_lens);

        R.shares[0] = 0;
        R.shares[1] = 0;

        for (int i = 0; i < dt->depth; i++) {
            cur_node.shares[0] = dt->Ps.shares[0][dt->idx_n];
            cur_node.shares[1] = dt->Ps.shares[1][dt->idx_n];
            cur_feature.shares[0] = features.shares[0][dt->idx_f];
            cur_feature.shares[1] = features.shares[1][dt->idx_f];

            ln_share.shares[0] = cur_node.shares[0].ln;
            ln_share.shares[1] = cur_node.shares[1].ln;
            rn_share.shares[0] = cur_node.shares[0].rn;
            rn_share.shares[1] = cur_node.shares[1].rn;
            lf_share.shares[0] = cur_node.shares[0].lf;
            lf_share.shares[1] = cur_node.shares[1].lf;
            rf_share.shares[0] = cur_node.shares[0].rf;
            rf_share.shares[1] = cur_node.shares[1].rf;
            t_share.shares[0] = cur_node.shares[0].t;
            t_share.shares[1] = cur_node.shares[1].t;

            R.shares[0] += t_share.shares[0];
            R.shares[1] += t_share.shares[1];
            cot_executor.cot_online(t_share, cur_feature, ln_share, rn_share, lf_share, rf_share,
                                    dt->idx_n, dt->idx_f, prss);
            // 通过cot更新idx_n和idx_f
        }

        return shuffler.reveal(R, std::numeric_limits<T>::max());
    }

    ~DtEval() {}
};

#endif // EVAL_HPP