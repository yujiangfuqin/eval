#include "dt.hpp"
#include "timer.hpp"

Config* Config::myconfig = nullptr;
P2Pchannel* P2Pchannel::mychnl = nullptr;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;

int main(int argc, const char** argv) {

    std::string st = argv[1];
    Config::myconfig = new Config("../config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);

    Timer::record("total_time");
    Dt<uint32_t, Node<uint32_t>> *dt = new Dt<uint32_t, Node<uint32_t>>("../dt/diabetes", 10, "player0", "player1", "player2");
    Timer::stop("total_time");

    // for(int i = 0; i< dt->node_lens; i++){
    //     std::cout << "share 1: " << dt->Ps[i];
    // }
    for(int i = 0; i< dt->node_lens*2+1; i++){
        std::cout << dt->per.zeta[i] << " ";
        if(i % 10 == 9) std::cout << std::endl;
    }
    std::cout << std::endl;
    for(int i = 0; i< dt->node_lens*2+1; i++){
        std::cout << dt->per.sigma[i] << " ";
        if(i % 10 == 9) std::cout << std::endl;
    }
    std::cout << "node lens: " << dt->node_lens << std::endl;

    Timer::test_print();
    return 0;
    
}