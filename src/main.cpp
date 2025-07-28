#include <iostream>
#include <iomanip>
#include "dt.hpp"
#include "timer.hpp"
#include "prss.hpp"
#include "shuffle.hpp"


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

    Timer::record("total_time");
    Dt<uint32_t, Node<uint32_t>> *dt = new Dt<uint32_t, Node<uint32_t>>("../dt/iris", 10, "player0", "player1", "player2");
    

    // for(int i = 0; i< dt->node_lens; i++){
    //     std::cout << "share 1: " << dt->Ps[i];
    // }
    for(int i = 0; i< dt->node_lens*2+1; i++){
        std::cout << dt->perm.shares[1].zeta[i] << " ";
        if(i % 10 == 9) std::cout << std::endl;
    }
    std::cout << std::endl;
    for(int i = 0; i< dt->node_lens*2+1; i++){
        std::cout << dt->perm.shares[1].sigma[i] << " ";
        if(i % 10 == 9) std::cout << std::endl;
    }
    std::cout << "node lens: " << dt->node_lens << std::endl;

    PRSS prss;
    prss.left = Config::myconfig->get_left_key();
    prss.right= Config::myconfig->get_right_key();


    Shuffle<uint64_t> shuffle(prss);
    auto perms = shuffle.generate(dt->node_lens);
    std::cout << "Generated random permutations (left/right):" << std::endl;
    std::cout << "left:  ";
    for (const auto& p : perms.shares[0]) {
        std::cout << p << " ";
    }
    std::cout << std::endl;
    std::cout << "right: ";
    for (const auto& p : perms.shares[1]) {
        std::cout << p << " ";
    }
    std::cout << std::endl;

    Timer::stop("total_time");
      Timer::test_print();

    return 0;
}

// std::cout << "=== PRSS 核心功能测试 ===" << std::endl;
    
    // // 测试4: 单密钥生成多个相同随机值
    // std::cout << "\n--- 测试1: 单密钥生成多个随机值 ---" << std::endl;
    // std::cout << "演示：相同密钥 → 相同随机序列" << std::endl;
    
    // // 定义一个共享密钥
    // oc::block shared_key = oc::block(0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL);
    
    // // 参与方A使用该密钥
    // oc::PRNG prng_A;
    // prng_A.SetSeed(shared_key);
    
    // // 参与方B使用相同密钥
    // oc::PRNG prng_B;
    // prng_B.SetSeed(shared_key);
    
    // std::cout << "\n两方使用相同密钥生成随机序列：" << std::endl;
    // std::cout << std::hex << std::setfill('0');
    
    // for(int i = 0; i < 6; i++) {
    //     u64 rand_A = prng_A.get<u64>();
    //     u64 rand_B = prng_B.get<u64>();
        
    //     std::cout << "第" << std::dec << i+1 << "个随机值: ";
    //     std::cout << "A=0x" << std::hex << std::setw(16) << rand_A;
    //     std::cout << ", B=0x" << std::hex << std::setw(16) << rand_B;
    //     std::cout << " " << (rand_A == rand_B ? "✓相同" : "✗不同") << std::endl;
    // }
    
    // // 测试6: 密钥重置后重新生成相同序列
    // std::cout << std::dec << "\n--- 测试2: 密钥重置后的一致性 ---" << std::endl;
    
    // oc::PRNG prng_reset;
    // oc::block test_key = oc::block(0xABCDEF0123456789ULL, 0x9876543210FEDCBAULL);
    
    // // 第一次使用密钥
    // prng_reset.SetSeed(test_key);
    // u64 first_seq[3];
    // for(int i = 0; i < 3; i++) {
    //     first_seq[i] = prng_reset.get<u64>();
    // }
    
    // // 重置相同密钥
    // prng_reset.SetSeed(test_key);
    // u64 second_seq[3];
    // for(int i = 0; i < 3; i++) {
    //     second_seq[i] = prng_reset.get<u64>();
    // }
    
    // std::cout << "第一次序列: ";
    // for(int i = 0; i < 3; i++) {
    //     std::cout << "0x" << std::hex << std::setw(8) << (first_seq[i] & 0xFFFFFFFF) << " ";
    // }
    // std::cout << std::endl;
    
    // std::cout << "重置后序列: ";
    // for(int i = 0; i < 3; i++) {
    //     std::cout << "0x" << std::hex << std::setw(8) << (second_seq[i] & 0xFFFFFFFF) << " ";
    // }
    // std::cout << std::endl;
    
    // bool sequences_match = true;
    // for(int i = 0; i < 3; i++) {
    //     if(first_seq[i] != second_seq[i]) {
    //         sequences_match = false;
    //         break;
    //     }
    // }
    
    // std::cout << std::dec << "序列一致性: " << (sequences_match ? "✓完全相同" : "✗不同") << std::endl;
    
    // std::cout << "\n=== 测试完成 ===" << std::endl;
    // return 0;