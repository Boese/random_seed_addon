#pragma once

#include <node_api.h>
#include <random>
#include <queue>
#include <memory>
#include <iostream>
#include "NodeGlobalBuffer.h"
#include "napi_extensions.h"

namespace node_rand {

/// \class NodeRand
/// \brief c++ addon to generate reproducible random number sequences based off a seed
template<class GENERATOR>
class NodeRand : public napi_extensions::NapiObjectWrap<NodeRand<GENERATOR>> {

    // reference for NodeJS Readable ctor
    static napi_ref m_readableCtor;

    // signal seed reset
    bool m_seedReset;
    // Instance of global buffer for psuedo seeds
    NodeGlobalBuffer m_GlobalBuffer;
    // rng
    GENERATOR m_generator;

    /// \brief Used to set m_readableCtor. Must call before using class.
    /// \param arg0 - Node JS Readable Function
    /// \return null
    static napi_value SetReadable(napi_env env, napi_callback_info info);

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

public:
    static std::vector<napi_property_descriptor> GetClassProps() {
        std::vector<napi_property_descriptor> props{
            {"SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0},
            { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
            { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 },
            { "SetReadable", 0, SetReadable, 0, 0, 0, napi_static, 0 }
        };
        return props;
    }

    /// \brief ctor
    NodeRand();

    /// \brief dtor
    ~NodeRand() { std::cout << "~NodeRand" << std::endl; };
};

}