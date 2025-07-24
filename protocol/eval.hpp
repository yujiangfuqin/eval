#include "dt.h"

class DtEval{
    // private:
    Dt<T, Node<T>> * dt;

    DtEval(Dt<T, Node<T>> * dt):dt(dt){
        // Constructor for internal use, no data to initialize
    }

    void offline(){
        P2Pchannel::mychnl->set_flush(false);

    }
    // T* data0, *data1;
    // uint32_t data_lens;
    // SOT<T> * ot_x;
    // SOT<Node_el<T>> *ot_n;
    // std::vector<CSOT<T>*> csot_xs, csot_ns;
    // public:
    // DtEval(Dt<T, Node_el<T>> * dt, T* data0, T* data1, uint32_t data_lens):dt(dt),data0(data0),data1(data1),data_lens(data_lens){
    //     ot_x = new SOT<T>(data0, data1, data_lens);
    //     ot_n = new SOT<Node_el<T>>(dt->Ps, dt->Ps_rep, dt->node_lens);
    //     for(int i = 0; i < dt->deep - 1; i++){
    //         CSOT<T>* csot_x = new CSOT<T>();CSOT<T>* csot_n = new CSOT<T>();
    //         csot_xs.push_back(csot_x);
    //         csot_ns.push_back(csot_n);
    //     }
        
    //     ot_x->offline(dt->deep);
    //     ot_n->offline(dt->deep);
        
        
    // }
    // void offline(){
    //     P2Pchannel::mychnl->set_flush(false);
    //     for(int i = 0; i < dt->deep - 1; i++){
    //         csot_xs[i]->offline_1();
    //         csot_ns[i]->offline_1();
    //     }
    //     P2Pchannel::mychnl->flush_all();
    //     for(int i = 0; i < dt->deep - 1; i++){
    //         csot_xs[i]->offline_2();
    //         csot_ns[i]->offline_2();
    //     }
    //     P2Pchannel::mychnl->flush_all();
    //     P2Pchannel::mychnl->set_flush(true);
        
    // }
    // ~DtEval(){
    //     delete ot_x;
    //     delete ot_n;
    //     for(auto & ele : csot_xs)
    //         delete ele;
    //     for(auto & ele : csot_ns)
    //         delete ele;

    // }
    // T online(){
    //     T y;
    //     memset(&y, 0, sizeof(y));
    //     uint32_t ki = dt->start_x;
    //     uint32_t idi = dt->start_n;
    //     T x_pick;
    //     Node_el<T> n_pick;
    //     for(int i = 1; i < dt->deep + 1; i++){
    //         P2Pchannel::mychnl->set_flush(false);
    //         ot_x->online_1(ki, i - 1);
    //         ot_n->online_1(idi, i - 1);
    //         P2Pchannel::mychnl->flush_all();
    //         x_pick = ot_x->online_2(ki, i - 1);
    //         n_pick = ot_n->online_2(idi, i - 1);
    //         P2Pchannel::mychnl->flush_all();
    //         y = y + n_pick.v;
    //         if(i >= dt->deep) {P2Pchannel::mychnl->set_flush(true);return y;}
    //         /*fetch id, k*/
    //         uint32_t block_x[2] = {n_pick.xl,n_pick.xr}, block_n[2] = {n_pick.nl,n_pick.nr};
    //         T select[2] = {x_pick, n_pick.t};
    //         csot_xs[i - 1]->online_1(block_x, select, 0, data_lens);
    //         csot_ns[i - 1]->online_1(block_n, select, 0, dt->node_lens);
    //         P2Pchannel::mychnl->flush_all();
            
    //         ki = csot_xs[i - 1]->online_2(block_x, select, 0, data_lens);
    //         idi = csot_ns[i - 1]->online_2(block_n, select, 0, dt->node_lens);
    //         P2Pchannel::mychnl->flush_all();
    //         P2Pchannel::mychnl->set_flush(true);
    //     }
    // }
}