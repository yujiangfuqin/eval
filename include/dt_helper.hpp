#ifndef _DT_HELPER_
#define _DT_HELPER_
#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include <queue>
#include <algorithm>
struct node{
    uint32_t weight,idx;
    uint32_t left;
    uint32_t right;
    node(uint32_t w, uint32_t idx, uint32_t l, uint32_t r):
        weight(w), idx(idx), left(l), right(r){}
};
void free_tree(std::map<int, node*> tmp){
    for(int i = 0; i < tmp.size(); i++)
        delete tmp[i];
}
void tokenize(const std::string& str, std::vector<std::string>& tokens) {
    tokens.clear();
    std::size_t prev = 0, pos;
    while ((pos = str.find_first_of(" [] \\ ", prev)) != std::string::npos)
    {
        if (pos > prev)
            tokens.push_back(str.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < str.length())
        tokens.push_back(str.substr(prev, std::string::npos));
}
std::map<int, node*> read_from_file(std::string string_file){
    std::map<int,node*> nodes;
    const char* filename = string_file.c_str();
    std::ifstream file;
    file.open(filename);
    

    std::string line;
    std::vector<std::string> tokens;
    while (getline(file, line)){
        
        tokenize(line, tokens);
        if(tokens.size() <= 1) continue;
        if(tokens[1] == "label=\"gini"){
            nodes[atoi(tokens[0].c_str())] = new node(atoi(tokens[6].c_str())* 1000, 0, 0, 0);
            //leaf
        }
        else if(tokens[1] == "label=\"X"){
            //node
            nodes[atoi(tokens[0].c_str())] = new node(atof(tokens[4].c_str())*1000, atoi(tokens[2].c_str()), 0, 0);
        }
        else if(tokens[1] == "->"){
            if(nodes[atoi(tokens[0].c_str())]->left == 0){
                nodes[atoi(tokens[0].c_str())]->left = atoi(tokens[2].c_str());
            }
            else{
                nodes[atoi(tokens[0].c_str())]->right = atoi(tokens[2].c_str());
            }
            //edge
        }
    }
    file.close();
    return nodes;
}
uint32_t find_deep(const std::map<int,node*>& tmp, uint32_t idx){
    uint32_t ldeep = 0, rdeep = 0;
    if(tmp.at(idx)->left != 0)
        ldeep = find_deep(tmp, tmp.at(idx)->left);
    if(tmp.at(idx)->right != 0)
        rdeep = find_deep(tmp, tmp.at(idx)->right);
    
    if(tmp.at(idx)->left == 0 && tmp.at(idx)->right == 0) return 1;
    return std::max(ldeep, rdeep) + 1;
}

uint32_t find_min_deep(const std::map<int, node*>& tmp, uint32_t idx) {
    uint32_t ldeep = UINT32_MAX, rdeep = UINT32_MAX;
    if (tmp.at(idx)->left != 0) 
        ldeep = find_min_deep(tmp, tmp.at(idx)->left);
    if (tmp.at(idx)->right != 0) 
        rdeep = find_min_deep(tmp, tmp.at(idx)->right);

    if(tmp.at(idx)->left == 0 && tmp.at(idx)->right == 0) return 1;
    return std::min(ldeep, rdeep) + 1;
}

template <typename T, typename D>
void cal_t(D* Ps, const std::map<int, node*>& tmp, uint32_t current_idx, T path_t_sum) 
{
    T current_t = tmp.at(current_idx)->weight;

    if (tmp.at(current_idx)->left == 0 && tmp.at(current_idx)->right == 0) {
        Ps[current_idx].t = current_t - path_t_sum;
    } else {
        Ps[current_idx].t = current_t;
        T next_path_t_sum = path_t_sum + current_t;
        if (tmp.at(current_idx)->left != 0) {
            cal_t(Ps, tmp, tmp.at(current_idx)->left, next_path_t_sum);
        }
        if (tmp.at(current_idx)->right != 0) {
            cal_t(Ps, tmp, tmp.at(current_idx)->right, next_path_t_sum); 
        }
    }
}

std::vector<uint32_t> cal_aux(const std::vector<uint32_t>& indices, uint32_t max_index_value) {
    if (max_index_value == 0) {
        return {};
    }
    std::vector<uint32_t> counts(max_index_value + 1, 0);
    for (uint32_t idx : indices) {
        if (idx <= max_index_value) {
            counts[idx]++;
        }
    }

    std::vector<uint32_t> aux(max_index_value + 1, 0);
    for (uint32_t i = 0; i <= max_index_value; ++i) {
        if (counts[i] > 1) {
            aux[i] = counts[i] - 1;
        }
    }
    return aux;


}
#endif