#include "NodeGlobalBuffer.h"

using namespace node_rand;

NodeGlobalBuffer::NodeGlobalBuffer() : m_buffer(), m_generator(std::random_device{}()), m_distribution(0, ~(uint64_t)0) {}

void NodeGlobalBuffer::FillBuffer()
{
    // Clear buffer
    std::queue<int64_t> empty;
    m_buffer.swap(empty);

    // Fill Buffer
    for (size_t i = 0; i < BufferMax; i++) {
        auto num = m_distribution(m_generator);
        m_buffer.push(num);
    }
}

void NodeGlobalBuffer::SetSeed(const int64_t seed) {
    m_generator.seed(seed);
    FillBuffer();
}

int64_t NodeGlobalBuffer::Next()
{
    if (m_buffer.empty()) {
        FillBuffer();
    }
    auto next = m_buffer.front();
    m_buffer.pop();
    return next;
}