#ifndef COT_HPP
#define COT_HPP

#include "cmp.hpp"
#include "config.hpp"
#include "prss.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <limits>

//根据x和y的大小关系，在分享值l_n，r_n中选择一个并输出明文result_n，在分享值l_f，r_f中选择一个并输出明文result_f，当x>=y时选择左侧，反之选择右侧
template <typename T> class cotp {
private:
    RepShare<T> x_prime1, x_prime2, y_prime1, y_prime2;
    std::array<std::vector<uint32_t>, 2> mask;
    // Party i.mask[0] == Party i-1.mask[1]
    // Party i.mask[1] == Party i+1.mask[0]

public:
    void cot_preprocess(PRSS &prss) {
        int party_id = Config::myconfig->get_idex();
        // for prime1
        if (party_id == 0) {
            x_prime1.get_share_index(party_id, 2) = prss.right.get<T>();
            y_prime1.get_share_index(party_id, 2) = prss.right.get<T>();
        } else if (party_id == 1) {
            x_prime1.get_share_index(party_id, 2) = prss.left.get<T>();
            y_prime1.get_share_index(party_id, 2) = prss.left.get<T>();
        }

        if (party_id == 0) {
            x_prime1.get_share_index(party_id, 1) = prss.left.get<T>();
            y_prime1.get_share_index(party_id, 1) = prss.left.get<T>();
        } else if (party_id == 2) {
            x_prime1.get_share_index(party_id, 1) = prss.right.get<T>();
            y_prime1.get_share_index(party_id, 1) = prss.right.get<T>();
        }

        // for prime2
        if (party_id == 0) {
            x_prime2.get_share_index(party_id, 2) = prss.right.get<T>();
            y_prime2.get_share_index(party_id, 2) = prss.right.get<T>();
        } else if (party_id == 1) {
            x_prime2.get_share_index(party_id, 2) = prss.left.get<T>();
            y_prime2.get_share_index(party_id, 2) = prss.left.get<T>();
        }

        if (party_id == 1) {
            x_prime2.get_share_index(party_id, 0) = prss.right.get<T>();
            y_prime2.get_share_index(party_id, 0) = prss.right.get<T>();
        } else if (party_id == 2) {
            x_prime2.get_share_index(party_id, 0) = prss.left.get<T>();
            y_prime2.get_share_index(party_id, 0) = prss.left.get<T>();
        }

        // mask
        for (int idx = 0; idx < 2; idx++) {
            mask[0].push_back(prss.left.get<uint32_t>());
            mask[1].push_back(prss.right.get<uint32_t>());
        }
    }

    void cot_online(RepShare<T> &x, RepShare<T> &y, RepShare<uint32_t> &l_n,
                    RepShare<uint32_t> &r_n, RepShare<uint32_t> &l_f, RepShare<uint32_t> &r_f,
                    uint32_t &result_n, uint32_t &result_f, PRSS &prss) {
        int party_id = Config::myconfig->get_idex();

        // step1: 重构x', y', x'', y''
        {
            T dx1, dx2, dx3, dx4, dy1, dy2, dy3, dy4;
            // send阶段
            if (party_id == 1 || party_id == 2) {
                dx1 =
                    x.get_share_index(party_id, 3 - party_id) - x_prime1.get_share_index(party_id, 3 - party_id);
                dy1 =
                    y.get_share_index(party_id, 3 - party_id) - y_prime1.get_share_index(party_id, 3 - party_id);
                if (party_id == 1) {
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &dx1, sizeof(T));
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &dy1, sizeof(T));
                } else {
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &dx1, sizeof(T));
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &dy1, sizeof(T));
                }
            }

            if (party_id == 0 || party_id == 2) {
                dx3 =
                    x.get_share_index(party_id, 2 - party_id) - x_prime2.get_share_index(party_id, 2 - party_id);
                dy3 =
                    y.get_share_index(party_id, 2 - party_id) - y_prime2.get_share_index(party_id, 2 - party_id);
                if (party_id == 0) {
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &dx3, sizeof(T));
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &dy3, sizeof(T));
                } else {
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &dx3, sizeof(T));
                    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &dy3, sizeof(T));
                }
            }

            // recv阶段
            if (party_id == 1 || party_id == 2) {
                if (party_id == 1) {
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &dx2,
                                                       sizeof(T));
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &dy2,
                                                       sizeof(T));
                } else {
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &dx2,
                                                       sizeof(T));
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &dy2,
                                                       sizeof(T));
                }
                x_prime1.get_share_index(party_id, 0) = x.get_share_index(party_id, 0) + dx1 + dx2;
                y_prime1.get_share_index(party_id, 0) = y.get_share_index(party_id, 0) + dy1 + dy2;
            }

            if (party_id == 0 || party_id == 2) {
                if (party_id == 0) {
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &dx4,
                                                       sizeof(T));
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &dy4,
                                                       sizeof(T));
                } else {
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &dx4,
                                                       sizeof(T));
                    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &dy4,
                                                       sizeof(T));
                }
                x_prime2.get_share_index(party_id, 1) = x.get_share_index(party_id, 1) + dx3 + dx4;
                y_prime2.get_share_index(party_id, 1) = y.get_share_index(party_id, 1) + dy3 + dy4;
            }
        }

        // step2: 计算delta（即L-R）
        RepShare<uint32_t> delta_n(l_n.shares[0] - r_n.shares[0], l_n.shares[1] - r_n.shares[1]);
        RepShare<uint32_t> delta_f(l_f.shares[0] - r_f.shares[0], l_f.shares[1] - r_f.shares[1]);

        // step3: cmp
        std::vector<int> vector_spec_party({0, 1});
        std::vector<RepShare<T>> vector_x({x_prime1, x_prime2});
        std::vector<RepShare<T>> vector_y({y_prime1, y_prime2});
        auto vector_z = vector_Fcmp(vector_spec_party, vector_x, vector_y);
        bool z_prime1 = vector_z[0];
        bool z_prime2 = vector_z[1];

        // step4: calculate I
        std::vector<uint32_t> vector_I(2);
        for (int idx = 0; idx < 2; idx++) {
            RepShare<uint32_t> *delta;
            RepShare<uint32_t> *R;
            if (idx == 0) {
                delta = &delta_n;
                R = &r_n;
            } else {
                delta = &delta_f;
                R = &r_f;
            }
            if (party_id == 0) {
                vector_I[idx] = z_prime1 * delta->get_share_index(party_id, 1) +
                                z_prime1 * delta->get_share_index(party_id, 2);
            } else if (party_id == 1) {
                vector_I[idx] = z_prime2 * delta->get_share_index(party_id, 0) +
                                z_prime1 * delta->get_share_index(party_id, 2);
            } else if (party_id == 2) {
                vector_I[idx] = z_prime2 * delta->get_share_index(party_id, 0) +
                                z_prime1 * delta->get_share_index(party_id, 1);
            }
            vector_I[idx] += R->shares[0];
            uint32_t masked_I1 = vector_I[idx] - mask[1][idx];
            uint32_t masked_I2 = vector_I[idx] + mask[0][idx];
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &masked_I1,
                                             sizeof(uint32_t));
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &masked_I2,
                                             sizeof(uint32_t));
        }
        for (int idx = 0; idx < 2; idx++) {
            uint32_t masked_I1;
            uint32_t masked_I2;
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &masked_I1,
                                               sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &masked_I2,
                                               sizeof(uint32_t));
            vector_I[idx] += masked_I1 + masked_I2;
            if (idx == 0) {
                result_n = vector_I[idx];
            } else {
                result_f = vector_I[idx];
            }
        }
    }
};
#endif // COT_HPP