#ifndef _PROTOCOL_TYPES_H
#define _PROTOCOL_TYPES_H

#include <vector>
#include <cstdint>
#include <array>


template<typename T>
struct RepShare {
    std::array<T, 2> shares; 

    RepShare() = default;
    RepShare(const T& left, const T& right) : shares{left, right} {}

};

struct Perm{
    std::vector<uint32_t> zeta, sigma, zeta_f, sigma_f;

    void resize(size_t new_size) {
        zeta.resize(new_size);
        sigma.resize(new_size);
        zeta_f.resize(new_size);
        sigma_f.resize(new_size);
    }
};

template <typename T> 
struct Node_orig{
    uint32_t k; T t;
};
template <typename T>
struct Node{
    uint32_t ln, rn, lf, rf; 
    T t;

    uint32_t node_lens = 16;
    uint32_t data_lens = 16;

    Node& operator+=(const Node& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->ln = (this->ln + other.ln) % this->node_lens;
        this->rn = (this->rn + other.rn) % this->node_lens;
        this->lf = (this->lf + other.lf) % this->data_lens;
        this->rf = (this->rf + other.rf) % this->data_lens;
        this->t += other.t;
        return *this;
    }
    Node& operator-=(const Node& other){
        if(this->node_lens == 0) this->node_lens = other.node_lens;
        if(this->data_lens == 0) this->data_lens = other.data_lens;
        this->ln = (this->ln + this->node_lens - other.ln) % this->node_lens;
        this->rn = (this->rn + this->node_lens - other.rn) % this->node_lens;
        this->lf = (this->lf + this->data_lens - other.lf) % this->data_lens;
        this->rf = (this->rf + this->data_lens - other.rf) % this->data_lens;
        this->t -= other.t;
        return *this;
    }
    const Node operator*(const uint32_t &other) const{
        Node<T> newnode;
        newnode.ln = (this->ln * other) % this->node_lens;
        newnode.rn = (this->rn * other) % this->node_lens;
        newnode.lf = (this->lf * other) % this->data_lens;
        newnode.rf = (this->rf * other) % this->data_lens;
        newnode.t = this->t * other;
        newnode.data_lens = this->data_lens;
        newnode.node_lens = this->node_lens;
        return newnode;
    }

    bool operator==(const Node& other) const {
        return ln == other.ln && rn == other.rn && lf == other.lf && rf == other.rf && t == other.t;
    }
    friend std::ostream & operator<<(std::ostream & out, Node A){
        out << (uint32_t)A.ln <<" "<< (uint32_t)A.rn <<" "<<(uint32_t)A.lf <<" "<<(uint32_t)A.rf <<" "<<(uint32_t)A.t <<" "<<std::endl;
        return out;
    }
};

#endif // _PROTOCOL_TYPES_H
