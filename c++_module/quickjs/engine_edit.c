int handle_op_limit(JSContext* ctx,  int64_t cost) {
    ctx->op_count += cost;
    if(ctx->op_count > ctx->op_limit) {
        JS_ThrowInternalError(ctx, "OP limit reached");
        goto exception;
    }
}
