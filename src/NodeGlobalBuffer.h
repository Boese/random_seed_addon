#include <node_api.h>
#include <random>
#include <queue>
#include <memory>

#pragma once

namespace node_rand {
    
/// \brief Singleton queue of int64_t of random numbers for async sequence calls. 
/// \note Used for generating psuedo seeds based off a real seed.
class NodeGlobalBuffer {
    static const uint64_t BufferMax{1000};
    std::queue<int64_t> m_buffer;
    std::mt19937 m_generator;
    std::uniform_int_distribution<int64_t> m_distribution;

    /// \brief Fill buffer with pseudo seed
    void FillBuffer();

public:
    /// \brief ctor
    NodeGlobalBuffer();

    /// \brief Set the "real" seed
    void SetSeed(const int64_t seed);

    /// \brief Get the next pseudo seed
    int64_t Next();
};

}