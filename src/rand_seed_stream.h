#pragma once

#include <node_api.h>
#include <random>
#include <memory>

namespace rand_addon {

class RandSeedStream /* extends Node JS Readable */ {
private:
    std::unique_ptr<std::mt19937> m_generator;
    int64_t m_min;
    int64_t m_max;
    uint32_t m_count;

    explicit RandSeedStream(int64_t seed, int64_t min, int64_t max, uint32_t count)
        : m_generator(std::make_unique<std::mt19937>(seed)), m_min(min), m_max(max), m_count(count) {}

public:
    /// \brief Instantiate class either using new or function() syntax
    /// \return this
    static napi_value NewInstance(napi_env env, napi_ref readableCtorRef, int64_t seed, int64_t min, int64_t max, uint32_t count);
};

}