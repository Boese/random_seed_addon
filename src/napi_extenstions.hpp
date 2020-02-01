/*
    napi-api extensions/utility functions.
*/
#pragma once
#include "node_api.h"
#include "assert.h"

namespace napi_extensions
{

/*
@function - napi_inherts
@description - This function will prototype chain the child class to the parent class. 
    It will also call the parent class super constructor() with args.
@returns - napi_status indicated if it suceeded.

example to inherit from NodeJS Readable:

// Argument - expect Readable Function
napi_value super_ctor = args[0]; <<-- new Addon(Readable)

// ctor
napi_value ctor; <<-- get ctor of addon self: Addon.h: static napi_ref constructor
status = napi_get_reference_value(env, constructor, &ctor);

// "inherit" from NodeJS Readable
status = napi_extensions::napi_inherits(env, info, ctor, super_ctor, 0, nullptr);
assert(status == napi_ok);

*/
static napi_status napi_inherits(napi_env env,           // Node-api env
                        napi_callback_info info,  // Node-api callback info
                        napi_value ctor,          // NodeJS child Function
                        napi_value superCtor,     // NodeJS parent Function
                        size_t superArgc,         // Size of args to call parent super()
                        napi_value* superArgv)     // Args to call parent super()
{
    napi_status status;
    napi_value jsthis;

    // Get JsThis
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);
    if (!status == napi_ok) { return status; }

    // Grap the prototypes of child/parent
    napi_value global, global_object, set_prototype_of, ctor_proto_prop, super_instance;

    status = napi_get_global(env, &global);
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, global, "Object", &global_object); // Object
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, global_object, "setPrototypeOf", &set_prototype_of); // Object.setPrototypeOf
    if (status != napi_ok) { return status; }
    status = napi_get_named_property(env, ctor, "prototype", &ctor_proto_prop); // ctor.prototype
    if (status != napi_ok) { return status; }

    // New instance of superCtor
    status = napi_new_instance(env, superCtor, superArgc, superArgv, &super_instance);
    if (status != napi_ok) { return status; }

    napi_value argv[2];
    argv[0] = ctor_proto_prop;
    argv[1] = super_instance;

    // Object.setPrototypeOf(ctor.prototype, new superCtor());
    status = napi_call_function(env, global, set_prototype_of, 2, argv, NULL);
    if (status != napi_ok) { return status; }

    // Object.setPrototypeOf(ctor, superCtor);
    argv[0] = ctor;
    argv[1] = superCtor;
    return napi_call_function(env, global, set_prototype_of, 2, argv, NULL);
}

} // end napi_extensions namespace