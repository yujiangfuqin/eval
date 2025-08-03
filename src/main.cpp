#include <iostream>
#include <iomanip>
#include "timer.hpp"
#include "eval.hpp"

Config* Config::myconfig = nullptr;
P2Pchannel* P2Pchannel::mychnl = nullptr;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;

int main(int argc, const char** argv) {
    std::string st = argv[1];
    std::string config_path = "../" + st + "_config.JSON";
    Config::myconfig = new Config(config_path);
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);

    std::vector<std::string> total_set = {"iris", "wine", "linnerud", "breast", "digits", "diabetes", "boston"};
    uint32_t data_lens[7] = {5, 8, 4, 13, 48, 11, 14};

    // for(auto & str:total_set){
    //     std::cout<<"dataset "<<str<<std::endl;

    //     uint64_t result;
    //     Dt<uint64_t, Node<uint64_t>>*dt = new Dt<uint64_t, Node<uint64_t>>("../dt/" + str, data_len, "player0");
    //     DtEval<uint64_t> *dt_eval = new DtEval<uint64_t>(dt, shuffler);
    //     dt_eval->offline();
    //     Timer::record(str);
    //     result = dt_eval->online();
    //     Timer::stop(str);
    //     delete dt;
    //     delete dt_eval;
    // }
    uint64_t result;
    PRSS prss;
    prss.left = Config::myconfig->get_left_key();
    prss.right= Config::myconfig->get_right_key();
    
    Dt<uint64_t, Node<uint64_t>> *dt = new Dt<uint64_t, Node<uint64_t>>("../dt/boston", 14, "player0");
    Shuffler<uint64_t> shuffler(prss);

    RepShare<std::vector<uint32_t>> tmp_zeta;
    tmp_zeta.shares[0] = dt->perms.shares[0].zeta_f;
    tmp_zeta.shares[1] = dt->perms.shares[1].zeta_f;


    DtEval<uint64_t> *dt_eval = new DtEval<uint64_t>(dt, shuffler, RepShare<std::vector<uint64_t>>{std::vector<uint64_t>(4, 0), std::vector<uint64_t>(4, 0)});
    Timer::record("iris");
    dt_eval->offline();
    //result = dt_eval->online();
    Timer::stop("iris");
    Timer::test_print();
    delete dt;
    delete dt_eval;



    // //测试 reshare(RepShare<std::vector<Node<T>>> &a, uint32_t st)
    // {
    //     size_t n = 5;
    //     RepShare<std::vector<Node<uint64_t>>> a{
    //         std::vector<Node<uint64_t>>(n),
    //         std::vector<Node<uint64_t>>(n)
    //     };
    //     // 用 random_node 初始化
    //     for(size_t i = 0; i < n; ++i) {
    //         a.shares[0][i] = shuffler.random_node(0, n);
    //         a.shares[1][i] = shuffler.random_node(1, n);
    //     }

    //     //std::cout << "After reshare (Node):\n";

        // Node<uint64_t> node;
        // RepShare<Node<uint64_t>> r_node;
        // std::cout << "Before apply permutation:\n";
        // for(size_t i = 0; i < n; ++i) {
        //     r_node.shares[0] = a.shares[0][i];
        //     r_node.shares[1] = a.shares[1][i];
        //     node = shuffler.reveal(r_node);
        //     //打印node
            
        //     std::cout << node ;
        // }

        // RepShare<std::vector<uint32_t>> a{
        //     std::vector<uint32_t>(5, 0),
        //     std::vector<uint32_t>(5, 0)
        //     };
        // if(Config::myconfig->get_idex() == 0) { 
        //     a.shares[0] = {0, 1, 2, 3, 4};
        //     a.shares[1] = {0, 0, 0, 0, 0};
        // }
        // else if(Config::myconfig->get_idex() == 1) { 
        //     a.shares[0] = {0, 0, 0, 0, 0};
        //     a.shares[1] = {0, 0, 0, 0, 0};
        // }
        // else {
        //     a.shares[0] = {0, 0, 0, 0, 0};
        //     a.shares[1] = {0, 1, 2, 3, 4};
        // }

        // RepShare<std::vector<uint32_t>> perms{
        //     std::vector<uint32_t>(5, 0),
        //     std::vector<uint32_t>(5, 0)
        // };
        // if(Config::myconfig->get_idex() == 0) { 
        //     perms.shares[0] = {4, 2, 0, 1, 3};
        //     perms.shares[1] = {0, 0, 0, 0, 0};
        // }
        // else if(Config::myconfig->get_idex() == 1) { 
        //     perms.shares[0] = {0, 0, 0, 0, 0};
        //     perms.shares[1] = {0, 0, 0, 0, 0};
        // }
        // else {
        //     perms.shares[0] = {0, 0, 0, 0, 0};
        //     perms.shares[1] = {4, 2, 0, 1, 3};
        // }
        // std::vector<uint32_t> tmp = shuffler.reveal(perms);
        // std::cout << "Revealed permutation: ";
        // for(auto& v : tmp) std::cout << v << " ";
        // std::cout << std::endl;

        // tmp = shuffler.reveal(a);
        // std::cout << "Revealed a: ";
        // for(auto& v : tmp) std::cout << v << " ";
        // std::cout << std::endl;

        // shuffler.apply_permutation(a, perms);

        // tmp = shuffler.reveal(a);
        // std::cout << "Revealed a: ";
        // for(auto& v : tmp) std::cout << v << " ";
        // std::cout << std::endl;

        // std::cout << "After apply permutation:\n";
        // for(size_t i = 0; i < n; ++i) {
        //     r_node.shares[0] = a.shares[0][i];
        //     r_node.shares[1] = a.shares[1][i];
        //     node = shuffler.reveal(r_node);
        //     std::cout << "Node " << i << ": " << node;
        // }
    
    // }
    return 0;
}


//prss功能正常

//reveal(vector<uint32_t>)功能正常
//reveal(template<typename U>)功能正常

//apply_permutation_local功能正常

//generate功能正常

//reshare(RepShare<std::vector<T>> &a, uint32_t st)功能正常
//reshare(RepShare<std::vector<Node<T>>> &a, uint32_t st)功能正常

//shuffle(RepShare<std::vector<T>> &a, RepShare<std::vector<uint32_t>> perms)功能正常
//apply_permutation(RepShare<std::vector<T>> &a, RepShare<std::vector<uint32_t>> &rou)功能正常