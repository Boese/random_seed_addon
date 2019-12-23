#include <node_api.h>

#include <assert.h>
#include <random>
#include <iostream>
#include <time.h>
#include <cstring>
#include <memory>

struct AddonData {
  std::mt19937* generator;
};

static void DeleteAddonData(napi_env env, void* data, void* hint) {
  // Avoid unused parameter warnings.
  (void) env;
  (void) hint;

  // Free the per-addon-instance data.
  AddonData* result = static_cast<AddonData*>(data);
  if (result) {
    if (result->generator) {
      delete result->generator;
      result->generator = nullptr;
    }
    delete result;
    result = nullptr;
  }
}

static AddonData* CreateAddonData(napi_env env, napi_value exports) {
  std::cout << "New AddonData instance" << std::endl;
  AddonData* result = new AddonData();
  result->generator = new std::mt19937(std::random_device{}());

  assert(napi_wrap(env,
                   exports,
                   result,
                   DeleteAddonData,
                   NULL,
                   NULL) == napi_ok);
  return result;
}

static AddonData* GetAddonData(napi_env env, napi_callback_info info)
{
  // Retrieve the per-addon-instance data.
  AddonData* addon_data = NULL;
  assert(napi_get_cb_info(env,
                          info,
                          NULL,
                          NULL,
                          NULL,
                          ((void**)&addon_data)) == napi_ok);

  return addon_data;
}

/// Specify seed for generator. Expect 1 arg for seed
static napi_value SetSeed(napi_env env, napi_callback_info info) {

  auto addon_data = GetAddonData(env, info);
  napi_status status;

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
    addon_data->generator->seed(seed);
  } 
  else {
    std::cout << "Seed generator with random_device" << std::endl;
    addon_data->generator->seed(std::random_device{}());
  }

  return nullptr;
}

/// Return next random number from generator between min/max
/// Expect 2 args: int32_t min, int32_t max
static napi_value GenerateRandom(napi_env env, napi_callback_info info) {
  AddonData* addon_data = GetAddonData(env, info);

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
                           distribution(*(addon_data->generator)),
                           &result) == napi_ok);

  // Return the JavaScript integer back to JavaScript.
  return result;
}

/// Generate a sequence of random numbers using rand().
/// Expects 3 arguments
///  > int32 min
///  > int32 max
///  > uint32 size
/// Returns javascript ArrayBuffer of Uint8_t. To get int32 values, convert to DataView.
static napi_value GenerateRandomSequence(napi_env env, napi_callback_info info) {
  AddonData* addon_data = GetAddonData(env, info);

  // Get min/max/size
  size_t argc = 3;
  napi_value args[3];
  napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  assert(status == napi_ok);

  if (argc < 3) {
    napi_throw_type_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  napi_valuetype valuetype0;
  status = napi_typeof(env, args[0], &valuetype0);
  assert(status == napi_ok);

  napi_valuetype valuetype1;
  status = napi_typeof(env, args[1], &valuetype1);
  assert(status == napi_ok);

  napi_valuetype valuetype2;
  status = napi_typeof(env, args[1], &valuetype2);
  assert(status == napi_ok);

  if (valuetype0 != napi_number || valuetype1 != napi_number || valuetype2 != napi_number) {
    napi_throw_type_error(env, nullptr, "Wrong arguments");
    return nullptr;
  }

  int32_t min;
  status = napi_get_value_int32(env, args[0], &min);
  assert(status == napi_ok);

  int32_t max;
  status = napi_get_value_int32(env, args[1], &max);
  assert(status == napi_ok);

  uint32_t size;
  status = napi_get_value_uint32(env, args[1], &size);
  assert(status == napi_ok);

  if (max < min) {
    napi_throw_type_error(env, nullptr, "Max < Min. Min value must be less than Max");
    return nullptr;
  }

  // Create new arraybuffer uint8_t[]
  const uint32_t buffsize = size*4;
  std::cout << "Buffer size: " << buffsize << std::endl;

  uint8_t* buf = nullptr;
  napi_value result;
  assert(napi_create_arraybuffer(env,
                           buffsize,
                           (void**)&buf,
                           &result) == napi_ok);

  std::uniform_int_distribution<int> distribution(min, max);

  for (uint32_t i = 0; i < size; i++) {
    const auto nextRand = distribution(*(addon_data->generator));
    buf[i*4] = nextRand;
    buf[(i*4) + 1] = nextRand >> 8;
    buf[(i*4) + 2] = nextRand >> 16;
    buf[(i*4) + 3] = nextRand >> 24;
  }
  
  // Return the JavaScript array buffer
  return result;
}

NAPI_MODULE_INIT(/*env, exports*/) {
  // Create a new instance of the per-instance-data that will be associated with
  // the instance of the addon being initialized here and that will be destroyed
  // along with the instance of the addon.
  AddonData* addon_data = CreateAddonData(env, exports);

  napi_property_descriptor bindings[] = {
    { "seed", 0, SetSeed, 0, 0, 0, napi_default, addon_data },
    { "generate", 0, GenerateRandom, 0, 0, 0, napi_default, addon_data },
    { "sequence", 0, GenerateRandomSequence, 0, 0, 0, napi_default, addon_data }
  };

  // Expose the two bindings declared above to JavaScript.
  assert(napi_define_properties(env,
                                exports,
                                sizeof(bindings) / sizeof(bindings[0]),
                                bindings) == napi_ok);

  // Return the `exports` object provided. It now has 3 new properties, which
  // are the functions we wish to expose to JavaScript.
  return exports;
}