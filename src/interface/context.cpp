#include "../../include/op/context.hpp"
#include "../data/context_data.hpp"
#include "../common/common.hpp"
#include "context_impl.hpp"
#include "common_impl.hpp"


// -- Context Management -- //


opContext*
opContextCreate()
{
  opContext *ctx = new opContext;

  ctx->data.alloc_cb    = getCallbackAlloc();
  ctx->data.realloc_cb  = getCallbackRealloc();
  ctx->data.destroy_cb  = getCallbackFree();
  ctx->data.user_data   = getCallbackUserData();;

  ctx->data.initial_reserve();

  return ctx;
}


// -- Context Support -- //
