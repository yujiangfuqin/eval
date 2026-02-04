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
#include "cmp.hpp"

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

    // Test cases: pairs (A, B). Fcmp(A, B) -> (A >= B)
    struct TestCase {
        uint64_t a;
        uint64_t b;
    };
    
    std::vector<TestCase> cases = {
        {10, 20}, // 10 >= 20 is False (0)
        {30, 15}, // 30 >= 15 is True (1)
        {50, 50}, // 50 >= 50 is True (1)
        {0, 100}, // 0 >= 100 is False (0)
        {100, 0}  // 100 >= 0 is True (1)
    };
    
    // r components for A
    std::vector<std::pair<uint64_t, uint64_t>> r_A = {
        {1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}
    };
    // r components for B
    std::vector<std::pair<uint64_t, uint64_t>> r_B = {
        {11, 12}, {13, 14}, {15, 16}, {17, 18}, {19, 20}
    };
    
    bool all_ok = true;
    int party_id = Config::myconfig->get_idex();

    for(size_t i = 0; i < cases.size(); ++i) {
        uint64_t A = cases[i].a;
        uint64_t B = cases[i].b;
        
        uint64_t ra1 = r_A[i].first;
        uint64_t ra2 = r_A[i].second;
        
        uint64_t rb1 = r_B[i].first;
        uint64_t rb2 = r_B[i].second;
        
        RepShare<uint64_t> shareA = make_share(A, ra1, ra2, st);
        RepShare<uint64_t> shareB = make_share(B, rb1, rb2, st);
        
        std::cout << "Test " << i << ": Comparing " << A << " >= " << B << " ... " << std::flush;
        
        // Test with different spec_party values
        for(int spec_party = 0; spec_party < 3; ++spec_party) {
            bool result = Fcmp(spec_party, shareA, shareB);
            
            // Expected result
            bool expected_plaintext = (A >= B);
            bool expected_result = (party_id == spec_party) ? expected_plaintext : false;
            
            if (result == expected_result) {
                std::cout << "OK (spec_party=" << spec_party << ", Got " << result << ") ";
            } else {
                std::cout << "FAIL (spec_party=" << spec_party << ", Got " << result << ", Expected " << expected_result << ") ";
                all_ok = false;
            }
        }
        std::cout << std::endl;
    }
    
    // Test vector comparison functionality
    std::cout << "\n=== Testing Vector Comparison Functionality ===" << std::endl;
    
    // Test cases for vector comparison
    std::vector<uint64_t> A_vec = {10, 30, 50, 0, 100};
    std::vector<uint64_t> B_vec = {20, 15, 50, 100, 0};
    std::vector<int> spec_parties = {0, 1, 2, 0, 1}; // Different spec_party for each comparison
    
    // Create vector of RepShares
    std::vector<RepShare<uint64_t>> shareA_vec, shareB_vec;
    
    for(size_t i = 0; i < A_vec.size(); ++i) {
        uint64_t ra1 = r_A[i].first;
        uint64_t ra2 = r_A[i].second;
        uint64_t rb1 = r_B[i].first;
        uint64_t rb2 = r_B[i].second;
        
        shareA_vec.push_back(make_share(A_vec[i], ra1, ra2, st));
        shareB_vec.push_back(make_share(B_vec[i], rb1, rb2, st));
    }
    
    std::cout << "Vector comparison test: " << std::flush;
    
    std::vector<bool> results = vector_Fcmp(spec_parties, shareA_vec, shareB_vec);
    
    bool vector_test_ok = true;
    for(size_t i = 0; i < A_vec.size(); ++i) {
        bool expected_plaintext = (A_vec[i] >= B_vec[i]);
        bool expected_result = (party_id == spec_parties[i]) ? expected_plaintext : false;
        
        if (results[i] == expected_result) {
            std::cout << "OK[" << i << "] ";
        } else {
            std::cout << "FAIL[" << i << ":" << A_vec[i] << ">=" << B_vec[i] 
                      << ",spec=" << spec_parties[i] << ",got=" << results[i] 
                      << ",exp=" << expected_result << "] ";
            vector_test_ok = false;
            all_ok = false;
        }
    }
    std::cout << std::endl;
    
    if (vector_test_ok) {
        std::cout << "Vector comparison test passed!" << std::endl;
    } else {
        std::cout << "Vector comparison test failed!" << std::endl;
    }
    
    // Cleanup
    delete P2Pchannel::mychnl;
    delete Config::myconfig;

    if (all_ok) {
        std::cout << "All comparison tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed." << std::endl;
        return 1;
    }
}
