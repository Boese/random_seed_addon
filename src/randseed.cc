#include "rand_seed.h"
#include <node_api.h>
#include <assert.h>
#include <random>
#include <iostream>
#include <string>

napi_ref RandSeed::constructor;

RandSeed::RandSeed() : m_generator(std::make_unique<std::mt19937>(std::random_device{}())) {}

RandSeed::~RandSeed() {
    m_generator.reset();
    napi_delete_reference(m_env, m_wrapper);
}

void RandSeed::Destructor(napi_env env,
                          void* nativeObject,
                          void* /*finalize_hint*/) {
  reinterpret_cast<RandSeed*>(nativeObject)->~RandSeed();
}

napi_value RandSeed::Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor properties[] = {
    { "SetSeed", 0, SetSeed, 0, 0, 0, napi_default, 0 },
    { "Generate", 0, Generate, 0, 0, 0, napi_default, 0 },
    { "GenerateSequenceStream", 0, GenerateSequenceStream, 0, 0, 0, napi_default, 0 }
  };
  size_t properties_count = sizeof(properties) / sizeof(properties[0]);

  napi_value cons;
  status = napi_define_class(
      env, "RandSeed", NAPI_AUTO_LENGTH, New, nullptr, properties_count, properties, &cons);
  assert(status == napi_ok);

  status = napi_create_reference(env, cons, 1, &constructor);
  assert(status == napi_ok);

  status = napi_set_named_property(env, exports, "RandSeed", cons);
  assert(status == napi_ok);
  return exports;
}

napi_value RandSeed::New(napi_env env, napi_callback_info info) {
  napi_status status;

  napi_value target;
  status = napi_get_new_target(env, info, &target);
  assert(status == napi_ok);
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    // Invoked as constructor: `new RandSeed()`
    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    RandSeed* rSeed = new RandSeed();

    rSeed->m_env = env;
    status = napi_wrap(env,
                       jsthis,
                       reinterpret_cast<void*>(rSeed),
                       RandSeed::Destructor,
                       nullptr,  // finalize_hint
                       &rSeed->m_wrapper);
    assert(status == napi_ok);

    return jsthis;
  } else {
    // Invoked as plain function `RandSeed()`, turn into construct call.
    status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    napi_value cons;
    status = napi_get_reference_value(env, constructor, &cons);
    assert(status == napi_ok);

    napi_value instance;
    status = napi_new_instance(env, cons, 0, nullptr, &instance);
    assert(status == napi_ok);

    return instance;
  }
}

// TODO: Put this in helper lib (for future projects). Other generic methods too like Init, New, Destructor
RandSeed* GetSelf(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    RandSeed* rSeed;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&rSeed));

    return rSeed;
}

napi_value RandSeed::SetSeed(napi_env env, napi_callback_info info) {

  napi_status status;

  RandSeed* rSeed = GetSelf(env, info);

  // Get seed if specified. If not use std::random_device()
  size_t argc = 1;
  napi_value args[1];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  assert(status == napi_ok);

  if (argc > 0) {

    napi_valuetype valuetype0;
    status = napi_typeof(env, args[0], &valuetype0);
    assert(status == napi_ok);

    if (valuetype0 != napi_number) {
        napi_throw_type_error(env, nullptr, "Wrong arguments. Expected number for seed");
        return nullptr;
    }

    int seed;
    status = napi_get_value_int32(env, args[0], &seed);
    assert(status == napi_ok);

    std::cout << "Seed generator with seed: " << seed << std::endl;
    rSeed->m_generator->seed(seed);
  } 
  else {
    std::cout << "Seed generator with random_device" << std::endl;
    rSeed->m_generator->seed(std::random_device{}());
  }

  return nullptr;
}

//TODO: Create arg type checker helper function
napi_value RandSeed::Generate(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf(env, info);

    // Get min/max
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    assert(status == napi_ok);

    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    napi_valuetype valuetype0;
    status = napi_typeof(env, args[0], &valuetype0);
    assert(status == napi_ok);

    napi_valuetype valuetype1;
    status = napi_typeof(env, args[1], &valuetype1);
    assert(status == napi_ok);

    if (valuetype0 != napi_number || valuetype1 != napi_number) {
        napi_throw_type_error(env, nullptr, "Wrong arguments");
        return nullptr;
    }

    int32_t min;
    status = napi_get_value_int32(env, args[0], &min);
    assert(status == napi_ok);

    int32_t max;
    status = napi_get_value_int32(env, args[1], &max);
    assert(status == napi_ok);

    if (max < min) {
        napi_throw_type_error(env, nullptr, "Max < Min. Min value must be less than Max");
        return nullptr;
    }

    // Generate random number
    std::uniform_int_distribution<int> distribution(min, max);

    napi_value result;
    assert(napi_create_int32(env,
                        distribution(*(rSeed->m_generator)),
                        &result) == napi_ok);

    // Return the JavaScript integer back to JavaScript.
    return result;
}

napi_value RandSeed::GenerateSequenceStream(napi_env env, napi_callback_info info) {
    RandSeed* rSeed = GetSelf(env, info);

    // Get min/max/size
    size_t argc = 3;
    napi_value args[3];
    napi_value jsthis;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    bool hasPush = false;
    napi_has_named_property(env, jsthis, "push", &hasPush);

    napi_value pushFunc;
    napi_get_named_property(env, jsthis, "push", &pushFunc);

    napi_value argv[1];
    status = napi_create_string_utf8(env, "start", NAPI_AUTO_LENGTH, argv);
    assert(status == napi_ok);

    // TODO: Before I can call push, I need to call Readable.call(this, {}: options);
    // How can I do this???

    napi_value result;
    napi_call_function(env, jsthis, pushFunc, 1, argv, &result);

    // napi_value result;
    // napi_get_property_names(env, jsthis, &result);

    // uint32_t size;
    // napi_get_array_length(env, result, &size);

    // std::cout << "size of props on this: " << size << std::endl;

    // for (size_t i = 0; i < size; i++) {
    //     napi_value r;
    //     napi_get_element(env, result, i, &r);

    //     char buf[100] = "";
    //     size_t bytesCopied;
    //     napi_get_value_string_utf8(env, r, buf, 100, &bytesCopied);
    //     std::string val(buf);
        
    //     std::cout << val << std::endl;
    // }

    return nullptr;
}


/* Register this as an ES Module */
napi_value Init(napi_env env, napi_value exports) {
  return RandSeed::Init(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)