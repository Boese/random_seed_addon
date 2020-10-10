#include "NodeRand.h"
#include "NodeRandStream.h"
#include "NodeRNG.h"
#include "napi_extensions.h"

#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using namespace node_rand;
using namespace napi_extensions;

// template<class GENERATOR>
// napi_ref NodeRand<GENERATOR>::m_constructor;
template<class GENERATOR>
napi_ref NodeRand<GENERATOR>::m_readableCtor;

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::SetReadable(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value args[1];
  CheckStatus(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "Failed to get callback info in SetReadable");
  CheckStatus(napi_create_reference(env, args[0], 1, &m_readableCtor), env, "Failed to create NodeJS Readable reference");
  std::cout << "SetReadable" << __FUNCTION__ << std::endl;
  return nullptr;
}

template<class GENERATOR>
NodeRand<GENERATOR>::NodeRand() : m_seedReset(false), m_GlobalBuffer(), m_generator(std::random_device{}()) {
  std::cout << "new NodeRand" << std::endl;
}

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::SetSeed(napi_env env, napi_callback_info info) {
  NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

  // Get seed if specified. If not use std::random_device()
  size_t argc = 1;
  CheckStatus(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr), env, "SetSeed() get cb info");
  rSeed->m_seedReset = true;

  if (argc == 0) {
    std::cout << "Seed generator with random_device" << std::endl;
    rSeed->m_GlobalBuffer.SetSeed(std::random_device{}());
  } 
  else {
    NapiArgInt64 arg0;
    GetArgs(env, info, arg0);
    int64_t seed = arg0.GetVal();
    std::cout << "Seed generator with set seed: " << seed << std::endl;
    
    rSeed->m_GlobalBuffer.SetSeed(seed);
  }

  return nullptr;
}

template<class GENERATOR>
napi_value NodeRand<GENERATOR>::Generate(napi_env env, napi_callback_info info) {
    NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

    

    // I have generator from template
    // I need distribution from args

    // enum type Distribution - The distribution type to use
    // enum type T - uint8_t, int8_t, ... bigint64_t
    // int64_t arg0 min
    // int64_t arg1 max
    // validate arg0 and arg1 match type T bounds
    // return int64_t

    // If bigint64 or biguint64, same but return must match and no validation is needed

    if (rSeed->m_seedReset) {
      auto fakeSeed = rSeed->m_GlobalBuffer.Next();
      std::cout << "Setting fake seed: " << fakeSeed << std::endl;
      rSeed->m_generator.seed(fakeSeed);
      rSeed->m_seedReset = false;
    }

    NapiArgInt64 arg0, arg1;
    GetArgs(env, info, arg0, arg1);

    int64_t min =  arg0.GetVal();
    int64_t max =  arg1.GetVal();

    if (max < min) {
      // TODO: Replace all errors with this call
      std::stringstream ss;
      ss << "Max < Min. Min: " << min << ", Max: " << max << std::endl;
        napi_throw_type_error(env, nullptr, ss.str().c_str());
        return nullptr;
    }
    
    // TODO: Need to grab this from Args
    NodeRNGUniformDistribution distribution(min, max);

    napi_value result;
    CheckStatus(napi_create_int64(env, distribution(rSeed->m_generator), &result), 
      env, "Failed to create int64");

    return result;
}

// TODO: Change to async function so that push can be called after return
template<class GENERATOR>
napi_value NodeRand<GENERATOR>::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    NodeRand<GENERATOR>* rSeed = GetSelf<NodeRand<GENERATOR>>(env, info);

    NapiArgInt64 arg0, arg1;
    NapiArgUint32 arg2;
    GetArgs(env, info, arg0, arg1, arg2);

    int64_t min = arg0.GetVal();
    int64_t max = arg1.GetVal();
    uint32_t count = arg2.GetVal();

    // get thread-safe seed off global
    int64_t seed = rSeed->m_GlobalBuffer.Next();

    // Return new instance of NodeRandStream
    std::cout << "GenerateSequenceStream seed: " << seed << std::endl;
    GENERATOR g(seed);

    // TODO: Switch this to use NodeRNGUniformDistribution based off min/max in NewInstance
    std::uniform_int_distribution<int64_t> d(min, max);
    return NodeRandStream<int64_t, GENERATOR, std::uniform_int_distribution<int64_t>>::NewInstance(env, rSeed->m_readableCtor, g, d, count);
}

/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  NodeRand<std::mt19937>::Init("NodeRand_mt19937", env, exports);
  NodeRand<std::mt19937_64>::Init("NodeRand_mt19937_64", env, exports);
  std::cout << "done" << std::endl;
  return exports;
}

/* Register this as an ES Module */
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
