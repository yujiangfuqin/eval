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
    uint32_t data_lens[7] = {4, 8, 3, 13, 48, 11, 14};

    uint64_t result;
    PRSS prss;
    prss.left = Config::myconfig->get_left_key();
    prss.right= Config::myconfig->get_right_key();
    
    Shuffler<uint64_t> shuffler(prss);

    for(int i = 0; i < total_set.size(); i++){
        std::cout<<"dataset "<<total_set[i]<<std::endl;

        uint64_t result;
        Dt<uint64_t, Node<uint64_t>>*dt = new Dt<uint64_t, Node<uint64_t>>("../dt/" + total_set[i], data_lens[i], "player0");
        DtEval<uint64_t> *dt_eval = new DtEval<uint64_t>(dt, shuffler,RepShare<std::vector<uint64_t>>{std::vector<uint64_t>(data_lens[i], 0), std::vector<uint64_t>(data_lens[i], 0)});
        dt_eval->offline();
        Timer::record(total_set[i]);
        result = dt_eval->online();
        Timer::stop(total_set[i]);
        delete dt;
        delete dt_eval;
    }
    Timer::test_print();
    return 0;
}