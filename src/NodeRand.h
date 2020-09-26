#pragma once

#include <node_api.h>
#include <random>
#include <queue>
#include <memory>
#include <iostream>
#include "NodeGlobalBuffer.h"

namespace node_rand {

/// \class NodeRand
/// \brief c++ addon to generate reproducible random number sequences based off a seed
template<class GENERATOR>
class NodeRand {
    // reference for 'this' class ctor
    static napi_ref m_constructor;
    // reference for NodeJS Readable ctor
    static napi_ref m_readableCtor;

    // napi_env
    napi_env m_env;
    // passed along to class to get reference to 'this'
    napi_ref m_wrapper;
    // signal seed reset
    bool m_seedReset;
    // Instance of global buffer for psuedo seeds
    NodeGlobalBuffer m_GlobalBuffer;
    // rng
    GENERATOR m_generator;

    /// \brief ctor
    NodeRand();

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

    /// \brief Calls ~Destructor()
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

public:
    /// \brief Module init function
    static napi_value Init(const std::string& generatorName, napi_env env, napi_value exports);
};

}