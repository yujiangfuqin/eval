#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "timer.hpp"
#include "eval.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "config.hpp"
#include "net.hpp"
#include "cot.hpp"
#include "prss.hpp"

// Define static members
Config* Config::myconfig = nullptr;
P2Pchannel* P2Pchannel::mychnl = nullptr;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;

// Wrapper to help construct RepShare for testing
RepShare<uint64_t> make_share(uint64_t val, uint64_t r1, uint64_t r2, std::string player) {
    // val = r0 + r1 + r2
    // r0 = val - r1 - r2
    uint64_t r0 = val - r1 - r2;
    
    if (player == "player0") {
        return RepShare<uint64_t>(r0, r1);
    } else if (player == "player1") {
        return RepShare<uint64_t>(r1, r2);
    } else if (player == "player2") {
        return RepShare<uint64_t>(r2, r0);
    }
    return RepShare<uint64_t>(0, 0);
}

RepShare<uint32_t> make_share_u32(uint32_t val, uint32_t r1, uint32_t r2, std::string player) {
    // val = r0 + r1 + r2
    // r0 = val - r1 - r2
    uint32_t r0 = val - r1 - r2;
    
    if (player == "player0") {
        return RepShare<uint32_t>(r0, r1);
    } else if (player == "player1") {
        return RepShare<uint32_t>(r1, r2);
    } else if (player == "player2") {
        return RepShare<uint32_t>(r2, r0);
    }
    return RepShare<uint32_t>(0, 0);
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <player_name>" << std::endl;
        return 1;
    }
    std::string st = argv[1];
    std::string config_path = "./" + st + "_config.JSON";
    Config::myconfig = new Config(config_path);
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);

    PRSS prss;
    prss.left = Config::myconfig->get_left_key();
    prss.right= Config::myconfig->get_right_key();
    
    // Configured network and prss, now starting test

    // Test cases: (x, y, l_n, r_n, l_f, r_f)
    struct TestCase {
        uint64_t x, y;
        uint32_t l_n, r_n, l_f, r_f;
    };
    
    std::vector<TestCase> cases = {
        {10, 20, 100, 200, 300, 400}, // x < y, should choose right values (r_n=200, r_f=400)
        {30, 15, 150, 250, 350, 450}, // x >= y, should choose left values (l_n=150, l_f=350)  
        {50, 50, 111, 222, 333, 444}, // x == y, should choose left values (l_n=111, l_f=333)
    };
    
    // Random shares for secret sharing
    std::vector<std::array<uint64_t, 2>> x_shares = {
        {1, 2}, {3, 4}, {5, 6}
    };
    std::vector<std::array<uint64_t, 2>> y_shares = {
        {11, 12}, {13, 14}, {15, 16}
    };
    std::vector<std::array<uint32_t, 2>> ln_shares = {
        {21, 22}, {23, 24}, {25, 26}
    };
    std::vector<std::array<uint32_t, 2>> rn_shares = {
        {31, 32}, {33, 34}, {35, 36}
    };
    std::vector<std::array<uint32_t, 2>> lf_shares = {
        {41, 42}, {43, 44}, {45, 46}
    };
    std::vector<std::array<uint32_t, 2>> rf_shares = {
        {51, 52}, {53, 54}, {55, 56}
    };
    
    bool all_ok = true;

    for(size_t i = 0; i < cases.size(); ++i) {
        uint64_t X = cases[i].x;
        uint64_t Y = cases[i].y;
        uint32_t L_N = cases[i].l_n;
        uint32_t R_N = cases[i].r_n;
        uint32_t L_F = cases[i].l_f;
        uint32_t R_F = cases[i].r_f;
        
        // Create shares
        RepShare<uint64_t> shareX = make_share(X, x_shares[i][0], x_shares[i][1], st);
        RepShare<uint64_t> shareY = make_share(Y, y_shares[i][0], y_shares[i][1], st);
        RepShare<uint32_t> shareLN = make_share_u32(L_N, ln_shares[i][0], ln_shares[i][1], st);
        RepShare<uint32_t> shareRN = make_share_u32(R_N, rn_shares[i][0], rn_shares[i][1], st);
        RepShare<uint32_t> shareLF = make_share_u32(L_F, lf_shares[i][0], lf_shares[i][1], st);
        RepShare<uint32_t> shareRF = make_share_u32(R_F, rf_shares[i][0], rf_shares[i][1], st);
        
        std::cout << "Test " << i << ": COT(" << X << " >= " << Y << ")" << std::endl;
        std::cout << "  Options: left(n=" << L_N << ", f=" << L_F << ") vs right(n=" << R_N << ", f=" << R_F << ")" << std::endl;
        
        try {
            // Create COT instance and run protocol
            cotp<uint64_t> cot_instance;
            
            // Preprocess phase
            cot_instance.cot_preprocess(prss);
            std::cout << "  Preprocess completed" << std::endl;
            
            // Online phase
            uint32_t result_n, result_f;
            cot_instance.cot_online(shareX, shareY, shareLN, shareRN, shareLF, shareRF, 
                                   result_n, result_f, prss);
            
            // Determine expected results
            bool should_choose_left = (X >= Y);
            uint32_t expected_n = should_choose_left ? L_N : R_N;
            uint32_t expected_f = should_choose_left ? L_F : R_F;
            
            // Verify results
            bool test_passed = (result_n == expected_n) && (result_f == expected_f);
            
            if (test_passed) {
                std::cout << "  PASS: Got n=" << result_n << ", f=" << result_f << std::endl;
            } else {
                std::cout << "  FAIL: Got n=" << result_n << ", f=" << result_f 
                          << ", Expected n=" << expected_n << ", f=" << expected_f << std::endl;
                all_ok = false;
            }
            
        } catch (const std::exception& e) {
            std::cout << "  ERROR: " << e.what() << std::endl;
            all_ok = false;
        }
        
        std::cout << std::endl;
    }
    
    // Cleanup
    delete P2Pchannel::mychnl;
    delete Config::myconfig;

    if (all_ok) {
        std::cout << "All COT tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed." << std::endl;
        return 1;
    }
}