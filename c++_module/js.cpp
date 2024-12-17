#define GMMODULE
#define MAX_CONTEXTS 128

#include "quickjs.h"
#include <stdio.h>
#include <cstring>
#include "GarrysMod/Lua/Interface.h"
#include "GarrysMod/Lua/LuaBase.h"

JSContext* contexts[MAX_CONTEXTS];
JSRuntime* runtimes[MAX_CONTEXTS];

GarrysMod::Lua::ILuaBase* LUA_CH;

void lua_printf(const char* str) {
    LUA_CH->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
    LUA_CH->GetField( -1, "print" );
    LUA_CH->PushString(str);
    LUA_CH->Call( 1, 0 );
 
    LUA_CH->Pop(); 
}

JSValue console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    LUA_CH->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
    LUA_CH->GetField( -1, "print" );

    for (int i = 0; i < argc; i++) {
        const char *str = JS_ToCString(ctx, argv[i]);
        if (str) {
            LUA_CH->PushString(str);
            JS_FreeCString(ctx, str);
        }
    }

    LUA_CH->Call( argc, 0 );
    LUA_CH->Pop(); 
    return JS_UNDEFINED;
}

LUA_FUNCTION( lua_create_context ) {
    for(int i = 0; i < MAX_CONTEXTS; i++) {
        if(contexts[i] == nullptr) {
            JSRuntime *rt = JS_NewRuntime();
            JSContext *ctx = JS_NewContext(rt);

            JSValue global_obj = JS_GetGlobalObject(ctx);
            JSValue new_obj = JS_NewObject(ctx);
            JSValue log_func = JS_NewCFunction(ctx, console_log, "log", 1);
            JS_SetPropertyStr(ctx, new_obj, "log", log_func);
            JS_SetPropertyStr(ctx, global_obj, "console", new_obj);


            JS_FreeValue(ctx, global_obj);
            contexts[i] = ctx;
            runtimes[i] = rt;

            LUA->PushNumber(0);
            LUA->PushNumber(i);
            return 2;
        }
    }

    LUA->PushNumber(1);
    LUA->PushString("You've hit the max number of javascript contexts!");
    //LUA->Error();

    return 2;
}

LUA_FUNCTION( lua_eval ) {
    LUA->CheckType( 1, GarrysMod::Lua::Type::Number );
    LUA->CheckType( 2, GarrysMod::Lua::Type::Number );
    LUA->CheckType( 3, GarrysMod::Lua::Type::String );

    size_t context = LUA->GetNumber( 1 );
    size_t length = LUA->GetNumber( 2 );
    const char* script = LUA->GetString( 3 );
    JSContext* ctx = contexts[context];

    if(context > MAX_CONTEXTS || context < 0 || ctx == nullptr) {
        LUA->PushNumber(1);
        LUA->PushString("Couldn't find that context!");
        return 2;
    } else if(length < 1) {
        LUA->PushNumber(1);
        LUA->PushString("Your script has to have at least 1 character!");
        return 2;
    }

    //JSValue result = JS_Eval(contexts[context], script, length, "<stdin>", JS_EVAL_TYPE_GLOBAL);
    JSValue bytecode = JS_ReadObject(ctx, (uint8_t*)script, length, JS_READ_OBJ_BYTECODE);
    JSValue result = JS_EvalFunction(ctx, bytecode);

    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char *error_message = JS_ToCString(ctx, exception);
        LUA->PushNumber(1);
        LUA->PushString(error_message);
        JS_FreeCString(ctx, error_message);
        JS_FreeValue(ctx, exception);
    } else {
        const char *str = JS_ToCString(ctx, result);
        LUA->PushNumber(0);
        if (str) {
            LUA->PushString(str);
            JS_FreeCString(ctx, str);
        } else{
            LUA->PushNil();
        }
    }

    JS_FreeValue(ctx, result);
    return 2;
}

LUA_FUNCTION( lua_compile ) {
    LUA->CheckType( 1, GarrysMod::Lua::Type::Number );
    LUA->CheckType( 2, GarrysMod::Lua::Type::Number );
    LUA->CheckType( 3, GarrysMod::Lua::Type::String );

    size_t context = LUA->GetNumber( 1 );
    size_t length = LUA->GetNumber( 2 );
    const char* script = LUA->GetString( 3 );

    if(context > MAX_CONTEXTS || context < 0 || contexts[context] == nullptr) {
        LUA->PushNumber(1);
        LUA->PushString("Couldn't find that context!");
        return 2;
    } else if(length < 1) {
        LUA->PushNumber(1);
        LUA->PushString("Your script has to have at least 1 character!");
        return 2;
    }

    JSValue result = JS_Eval(contexts[context], script, length, "<stdin>", JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(contexts[context]);
        const char *error_message = JS_ToCString(contexts[context], exception);
        LUA->PushNumber(1);
        LUA->PushString(error_message);
        LUA->PushNil();
        JS_FreeCString(contexts[context], error_message);
        JS_FreeValue(contexts[context], exception);
    } else {
        size_t bytecode_len;
        uint8_t *bytecode = JS_WriteObject(contexts[context], &bytecode_len, result, JS_WRITE_OBJ_BYTECODE);

        LUA->PushNumber(0);
        LUA->PushString(reinterpret_cast<const char*>(bytecode), bytecode_len);
        LUA->PushNumber(bytecode_len);
        free(bytecode);
    }

    JS_FreeValue(contexts[context], result);
    return 3;
}

LUA_FUNCTION( lua_free_context ) {
    LUA->CheckType( 1, GarrysMod::Lua::Type::Number );
    size_t context = LUA->GetNumber( 1 );
    
    if(context > MAX_CONTEXTS || context < 0 || contexts[context] == nullptr) {
        LUA->PushNumber(1);
        LUA->PushString("Couldn't find that context!");
        //LUA->Error();
        return 2;
    }

    JS_FreeContext(contexts[context]);
    JS_FreeRuntime(runtimes[context]);

    contexts[context] = nullptr;
    runtimes[context] = nullptr;
    
    LUA->PushNumber(0);
    LUA->PushNil();
    return 2;
}

GMOD_MODULE_OPEN() {    
    LUA_CH = LUA;

    LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
          LUA->PushCFunction( lua_create_context );
          LUA->SetField( -2, "JS_CreateContext" );

          LUA->PushCFunction( lua_free_context );
          LUA->SetField( -2, "JS_FreeContext" );

          LUA->PushCFunction( lua_eval );
          LUA->SetField( -2, "JS_Eval" );

          LUA->PushCFunction( lua_compile );
          LUA->SetField( -2, "JS_Compile" );
     LUA->Pop();

    lua_printf("Opened JS module...");
    return 0;
}

GMOD_MODULE_CLOSE() {
    for(int i = 0; i < MAX_CONTEXTS; i++) {
        if(contexts[i] != nullptr) {
            JS_FreeContext(contexts[i]);
        }
    }

    for(int i = 0; i < MAX_CONTEXTS; i++) {
        if(runtimes[i] != nullptr) {
                JS_FreeRuntime(runtimes[i]);
        }
    }

    lua_printf("Closed JS module...");
    return 0;
}
