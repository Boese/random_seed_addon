#pragma once

#include <node_api.h>
#include <random>
#include <queue>
#include <memory>
#include <iostream>

namespace node_rand {

/// \class NodeRand
/// \brief c++ addon to generate reproducible random number sequences based off a seed
class NodeRand {
private:
    /// \brief ctor
    explicit NodeRand();

    /// \brief dtor
    ~NodeRand();

    /// \brief Used to set m_readableCtor. Must call before using class.
    /// \param arg0 - Node JS Readable Function
    /// \return null
    static napi_value SetReadable(napi_env env, napi_callback_info info);

    /// \brief Instantiate class either using new or function() syntax
    /// \return this
    static napi_value New(napi_env env, napi_callback_info info);

    /// \brief Set the seed of the RNG
    /// \param (Optional) arg0 int64_t seed. Default is random seed
    /// \return null
    static napi_value SetSeed(napi_env env, napi_callback_info info);

    /// \brief Synchronous function to generate a single random number between a min <-> max
    /// \param arg0 int64_t min
    /// \param arg1 int64_t max
    /// \return int64_t random number
    static napi_value Generate(napi_env env, napi_callback_info info);

    /// \brief Asynchronous function to generate a stream of random numbers between a min <-> max
    /// \param arg0 int64_t min
    /// \param arg1 int64_t max
    /// \param arg2 int64_t count - how many to generate
    /// \return Readable instance that will write random numbers to buffer. See class rand_seed_stream
    static napi_value GenerateSequenceStream(napi_env env, napi_callback_info info);
    
    // reference for 'this' class ctor
    static napi_ref m_constructor;
    // reference for NodeJS Readable ctor
    static napi_ref m_readableCtor;

    // napi_env
    napi_env m_env;
    // passed along to class to get reference to 'this'
    napi_ref m_wrapper;
    // the RNG
    std::unique_ptr<std::mt19937> m_generator;
    // signal seed reset
    bool m_seedReset{false};

    // TODO: Move this to a new file!!
    /// \brief Singleton queue of int64_t of random numbers for async sequence calls
    class GlobalBuffer {
        const uint64_t BufferMax{1000};
        std::queue<int64_t> m_buffer;
        std::mt19937 m_generator;
        const std::uniform_int_distribution<int64_t> m_distribution;

        void FillBuffer()
        {
            static std::queue<int64_t> empty{};
            // Clear buffer
            m_buffer.swap(empty);

            // Fill Buffer
            for (size_t i = 0; i < BufferMax; i++) {
                auto num = m_distribution(m_generator);
                m_buffer.push(num);
            }
        }

    public:
        GlobalBuffer() : m_buffer(), m_generator(std::random_device{}()), 
            m_distribution(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()) {}

        void SetSeed(int64_t seed) {
            m_generator.seed(seed);
            FillBuffer();
        }

        int64_t Next() {
            if (m_buffer.empty()) {
                FillBuffer();
            }
            auto next = m_buffer.front();
            m_buffer.pop();
            return next;
        }
    };
    std::unique_ptr<GlobalBuffer> m_GlobalBuffer;

public:
    /// \brief Module init function
    static napi_value Init(napi_env env, napi_value exports);

    /// \brief Calls ~Destructor()
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
};

}