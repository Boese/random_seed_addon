#pragma once

#include <node_api.h>
#include <random>
#include <queue>
#include <memory>

namespace rand_addon {

/// \class RandSeed
/// \brief c++ addon to generate reproducible random number sequences based off a seed
class RandSeed {
private:
    /// \brief ctor
    explicit RandSeed();

    /// \brief dtor
    ~RandSeed();

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

    /// \brief Singleton queue of int64_t of random numbers for async sequence calls
    class GlobalBuffer {
        static const uint64_t BufferMax{1000};

        static std::queue<int64_t>& GetBuffer()
        {
            static std::queue<int64_t> globalBuffer{};
            return globalBuffer;
        }

        static std::mt19937& GetGenerator()
        {
            static std::mt19937 generator{std::random_device{}()};
            return generator;
        }

        static std::uniform_int_distribution<int64_t>& GetDistribution()
        {
            static std::uniform_int_distribution<int64_t> distribution(
                std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
            return distribution;
        }

        static void FillBuffer()
        {
            // Clear buffer
            static std::queue<int64_t> empty{};
            auto& buffer = GetBuffer();
            buffer.swap(empty);

            auto& distribution = GetDistribution();
            auto& generator = GetGenerator();

            // Fill Buffer
            for (size_t i = 0; i < BufferMax; i++) {
                buffer.push(distribution(generator));
            }
        }

    public:
        static void SetSeed(int64_t seed) {
            auto& buffer = GetBuffer();
            GetGenerator().seed(seed);
            FillBuffer();
        }

        static int64_t Next() {
            auto& buffer = GetBuffer();
            if (buffer.empty()) {
                FillBuffer();
            }
            return buffer.front();
        }
    };

public:
    /// \brief Module init function
    static napi_value Init(napi_env env, napi_value exports);

    /// \brief Calls ~Destructor()
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
};

}