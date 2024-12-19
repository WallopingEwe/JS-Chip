#define GMMODULE
#define MAX_CONTEXTS 128
#define MAX_LISTENERS 10

#include "quickjs.h"
#include <stdio.h>
#include <cstring>
#include "GarrysMod/Lua/Interface.h"
#include "GarrysMod/Lua/LuaBase.h"

typedef struct {
    char *eventName;
    JSValue callback;
} EventListener;

typedef struct {
    EventListener listeners[MAX_LISTENERS];
    int listenerCount;
    JSValue entIndex;
    JSValue ownerIndex;
} ContextData;

JSContext* contexts[MAX_CONTEXTS];
JSRuntime* runtimes[MAX_CONTEXTS];

GarrysMod::Lua::ILuaBase* LUA_CH;

void lua_printf(const char* str) {
    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "print");
    LUA_CH->PushString(str);
    LUA_CH->Call(1, 0);
 
    LUA_CH->Pop(); 
}

JSValue dev_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "print");

    for(int i = 0; i < argc; i++) {
        const char *str = JS_ToCString(ctx, argv[i]);
        if(str) {
            LUA_CH->PushString(str);
            JS_FreeCString(ctx, str);
        }
    }

    LUA_CH->Call(argc, 0);
    LUA_CH->Pop(); 
    return JS_UNDEFINED;
}

JSValue console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    double ownerIndex;
    JS_ToFloat64(ctx, &ownerIndex, contextData->ownerIndex);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(ownerIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "EntIndex");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(LUA_CH->GetNumber(-1) == 0) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-3, "JSChatPrint");
    LUA_CH->Push(-3);

    for(int i = 0; i < argc; i++) {
        const char *str = JS_ToCString(ctx, argv[i]);
        if(str) {
            LUA_CH->PushString(str);
            JS_FreeCString(ctx, str);
        }
    }

    LUA_CH->Call(argc+1, 0);
    LUA_CH->Pop(3);

    return JS_UNDEFINED;
}

JSValue entity_get_name(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "EntIndex");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(LUA_CH->GetNumber(-1) == 0) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetName");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    JSValue entityName = JS_NewString(ctx, LUA_CH->GetString(-1));

    LUA_CH->Pop(4);

    return entityName;
}

JSValue entity_get_class(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "EntIndex");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(LUA_CH->GetNumber(-1) == 0) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetClass");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    JSValue entityClass = JS_NewString(ctx, LUA_CH->GetString(-1));

    LUA_CH->Pop(4);

    return entityClass;
}

JSValue get_ent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);
    
    return contextData->entIndex;
}

JSValue get_owner(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);
    
    return contextData->ownerIndex;
}

JSValue event_on(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (eventName, callback)");
    }

    const char *eventName = JS_ToCString(ctx, argv[0]);
    if(!JS_IsFunction(ctx, argv[1])) {
        JS_FreeCString(ctx, eventName);
        return JS_ThrowTypeError(ctx, "Second argument must be a function");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    for(int i = 0; i < contextData->listenerCount; i++) {
        if(strcmp(contextData->listeners[i].eventName, eventName) == 0) {
            JS_FreeValue(ctx, contextData->listeners[i].callback);
            contextData->listeners[i].callback = JS_DupValue(ctx, argv[1]);
            JS_FreeCString(ctx, eventName);
            return JS_UNDEFINED;
        }
    }

    if(contextData->listenerCount >= MAX_LISTENERS) {
        JS_FreeCString(ctx, eventName);
        return JS_ThrowInternalError(ctx, "Maximum events reached");
    }

    EventListener *newListener = &contextData->listeners[contextData->listenerCount++];
    newListener->eventName = strdup(eventName);
    newListener->callback = JS_DupValue(ctx, argv[1]);

    JS_FreeCString(ctx, eventName);
    return JS_UNDEFINED;
}

LUA_FUNCTION( lua_emit_event ) {
    LUA->CheckType(1, GarrysMod::Lua::Type::Number);
    LUA->CheckType(2, GarrysMod::Lua::Type::String);

    int lua_args = LUA->Top();
    size_t context = LUA->GetNumber(1);
    const char* eventName = LUA->GetString(2);
    JSContext* ctx = contexts[context];

    if(context > MAX_CONTEXTS || context < 0 || ctx == nullptr) {
        LUA->PushNumber(1);
        LUA->PushString("Couldn't find that context!");
        return 2;
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    for (int i = 0; i < contextData->listenerCount; i++) {
        if(strcmp(contextData->listeners[i].eventName, eventName) == 0) {
            JSValue callback = contextData->listeners[i].callback;
            JSValue *args = new JSValue[lua_args - 2];
            
            for(int k = 3; k <= lua_args; k++) {
                switch(LUA->GetType(k)) {
                    case GarrysMod::Lua::Type::Nil: {
                        args[k - 3] = JS_NULL;
                        break;
                    }
                    case GarrysMod::Lua::Type::Bool: {
                        args[k - 3] = JS_NewBool(ctx, LUA->GetBool(k));
                        break;
                    }
                    case GarrysMod::Lua::Type::Number: {
                        args[k - 3] = JS_NewFloat64(ctx, LUA->GetNumber(k));
                        break;
                    }
                    case GarrysMod::Lua::Type::String: {
                        args[k - 3] = JS_NewString(ctx, LUA->GetString(k));
                        break;
                    }
                    case GarrysMod::Lua::Type::Table: {
                        LUA->PushString("r");
                        LUA->GetTable(k);
                        bool hasR = (LUA->GetType(-1) == GarrysMod::Lua::Type::Number);
                        LUA->Pop();

                        LUA->PushString("g");
                        LUA->GetTable(k);
                        bool hasG = (LUA->GetType(-1) == GarrysMod::Lua::Type::Number);
                        LUA->Pop();

                        LUA->PushString("b");
                        LUA->GetTable(k);
                        bool hasB = (LUA->GetType(-1) == GarrysMod::Lua::Type::Number);
                        LUA->Pop();

                        if(hasR && hasG && hasB) {
                            args[k - 3] = JS_NewObject(ctx);
                            JS_SetPropertyStr(ctx, args[k - 3], "r", JS_NewFloat64(ctx, LUA->GetNumber(-3)));
                            JS_SetPropertyStr(ctx, args[k - 3], "g", JS_NewFloat64(ctx, LUA->GetNumber(-2)));
                            JS_SetPropertyStr(ctx, args[k - 3], "b", JS_NewFloat64(ctx, LUA->GetNumber(-1)));

                            LUA->PushString("a");
                            LUA->GetTable(k);
                            if (LUA->GetType(-1) == GarrysMod::Lua::Type::Number) {
                                JS_SetPropertyStr(ctx, args[k - 3], "a", JS_NewFloat64(ctx, LUA->GetNumber(-1)));
                            } else {
                                JS_SetPropertyStr(ctx, args[k - 3], "a", JS_NewFloat64(ctx, 255));

                            }
                            LUA->Pop();
                        } else {
                            args[k - 3] = JS_NULL;
                        }

                        break;
                    }
                    case GarrysMod::Lua::Type::Entity: {
                        LUA->GetMetaTable(k);
                        LUA->GetField(k, "EntIndex");
                        LUA->Push(k);
                        LUA->Call(1, 1);

                        double index = LUA->GetNumber(-1);
                        if(index == 0) {
                            args[k - 3] = JS_NULL;
                        } else {
                            args[k - 3] = JS_NewFloat64(ctx, LUA->GetNumber(-1));
                        }

                        LUA->Pop(1);

                        break;
                    }
                    case GarrysMod::Lua::Type::Vector: {
                        Vector vec = LUA->GetVector(k);
                        
                        args[k - 3] = JS_NewObject(ctx);
                        JS_SetPropertyStr(ctx, args[k - 3], "x", JS_NewFloat64(ctx, vec.x));
                        JS_SetPropertyStr(ctx, args[k - 3], "y", JS_NewFloat64(ctx, vec.y));
                        JS_SetPropertyStr(ctx, args[k - 3], "z", JS_NewFloat64(ctx, vec.z));
                        break;
                    }
                    case GarrysMod::Lua::Type::Angle: {
                        QAngle ang = LUA->GetAngle(k);

                        args[k - 3] = JS_NewObject(ctx);
                        JS_SetPropertyStr(ctx, args[k - 3], "pitch", JS_NewFloat64(ctx, ang.x));
                        JS_SetPropertyStr(ctx, args[k - 3], "yaw", JS_NewFloat64(ctx, ang.y));
                        JS_SetPropertyStr(ctx, args[k - 3], "roll", JS_NewFloat64(ctx, ang.z));
                        break;
                    }
                    default: {
                        args[k - 3] = JS_NULL;
                    }
                }
            }


            JSValue result = JS_Call(ctx, callback, JS_UNDEFINED, lua_args - 2, args);

            if(JS_IsException(result)) {
                JSValue exception = JS_GetException(ctx);
                const char *error_message = JS_ToCString(ctx, exception);
                LUA->PushNumber(1);
                LUA->PushString(error_message);
                JS_FreeCString(ctx, error_message);
                JS_FreeValue(ctx, exception);
            } else {
                const char *str = JS_ToCString(ctx, result);
                LUA->PushNumber(0);
                if(str) {
                    LUA->PushString(str);
                    JS_FreeCString(ctx, str);
                } else{
                    LUA->PushNil();
                }
            }

            for(int i = 0; i < lua_args - 2; ++i) {
                JS_FreeValue(ctx, args[i]);
            }

            delete[] args;

            return 2;

        }
    }

    LUA->PushNumber(0);
    LUA->PushNil();
    return 2;
}

LUA_FUNCTION( lua_create_context ) {
    LUA->CheckType(1, GarrysMod::Lua::Type::Entity);

    double ownerIndex;
    double entIndex;

    LUA->GetMetaTable(1);
    LUA->GetField(1, "EntIndex");
    LUA->Push(1);
    LUA->Call(1, 1);
    entIndex = LUA->GetNumber(-1);
    LUA->Pop(1);

    if(LUA->GetType(2) == GarrysMod::Lua::Type::Entity) {
        LUA->GetMetaTable(2);
        LUA->GetField(2, "EntIndex");
        LUA->Push(2);
        LUA->Call(1, 1);
        ownerIndex = LUA->GetNumber(-1);
        LUA->Pop(1);
    } else {
        ownerIndex = entIndex;
    }

    if(entIndex == 0 || ownerIndex == 0) {
        LUA->PushNumber(1);
        LUA->PushString("Invalid entity");
        return 2;
    }

    for(int i = 0; i < MAX_CONTEXTS; i++) {
        if(contexts[i] == nullptr) {
            JSRuntime *rt = JS_NewRuntime();
            JS_SetMemoryLimit(rt, 655359);
            JSContext *ctx = JS_NewContext(rt);
        
            ContextData *contextData = (ContextData *)malloc(sizeof(ContextData));

            contextData->entIndex = JS_NewFloat64(ctx, entIndex);
            contextData->ownerIndex = JS_NewFloat64(ctx, ownerIndex);
            
            contextData->listenerCount = 0;
            JS_SetContextOpaque(ctx, contextData);

            JSValue global_obj = JS_GetGlobalObject(ctx);

            JSValue entity_obj = JS_NewObject(ctx);
            JSValue entity = JS_NewCFunction(ctx, get_ent, "entity", 1);
            JSValue owner = JS_NewCFunction(ctx, get_owner, "owner", 1);
            JSValue get_name = JS_NewCFunction(ctx, entity_get_name, "GetName", 1);
            JSValue get_class = JS_NewCFunction(ctx, entity_get_class, "GetClass", 1);

            JS_SetPropertyStr(ctx, entity_obj, "chip", entity);
            JS_SetPropertyStr(ctx, entity_obj, "owner", owner);
            JS_SetPropertyStr(ctx, entity_obj, "GetName", get_name);
            JS_SetPropertyStr(ctx, entity_obj, "GetClass", get_class);
            JS_SetPropertyStr(ctx, global_obj, "Entity", entity_obj);

            JSValue new_obj = JS_NewObject(ctx);
            JSValue log_func = JS_NewCFunction(ctx, console_log, "log", 1);
            JS_SetPropertyStr(ctx, new_obj, "log", log_func);
            JS_SetPropertyStr(ctx, global_obj, "console", new_obj);

            JSValue events_obj = JS_NewObject(ctx);
            JSValue events_on = JS_NewCFunction(ctx, event_on, "on", 1);
            JS_SetPropertyStr(ctx, events_obj, "on", events_on);
            JS_SetPropertyStr(ctx, global_obj, "Events", events_obj);

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
    LUA->CheckType(1, GarrysMod::Lua::Type::Number);
    LUA->CheckType(2, GarrysMod::Lua::Type::Number);
    LUA->CheckType(3, GarrysMod::Lua::Type::String);

    size_t context = LUA->GetNumber(1);
    size_t length = LUA->GetNumber(2);
    const char* script = LUA->GetString(3);
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

    if(JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char *error_message = JS_ToCString(ctx, exception);
        LUA->PushNumber(1);
        LUA->PushString(error_message);
        JS_FreeCString(ctx, error_message);
        JS_FreeValue(ctx, exception);
    } else {
        const char *str = JS_ToCString(ctx, result);
        LUA->PushNumber(0);
        if(str) {
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
    LUA->CheckType(1, GarrysMod::Lua::Type::Number);
    LUA->CheckType(2, GarrysMod::Lua::Type::Number);
    LUA->CheckType(3, GarrysMod::Lua::Type::String);

    size_t context = LUA->GetNumber(1);
    size_t length = LUA->GetNumber(2);
    const char* script = LUA->GetString(3);

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
    if(JS_IsException(result)) {
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
    LUA->CheckType(1, GarrysMod::Lua::Type::Number);
    size_t context = LUA->GetNumber(1);
    
    if(context > MAX_CONTEXTS || context < 0 || contexts[context] == nullptr) {
        LUA->PushNumber(1);
        LUA->PushString("Couldn't find that context!");
        //LUA->Error();
        return 2;
    }

    ContextData *contextData = (ContextData*)JS_GetContextOpaque(contexts[context]);

    for(int i = 0; i < contextData->listenerCount; i++) {
        if(contextData->listeners[i].eventName) {
            free(contextData->listeners[i].eventName);
        }
        JS_FreeValue(contexts[context], contextData->listeners[i].callback);
    }

    JS_SetContextOpaque(contexts[context], NULL);
    free(contextData);
    
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

    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
          LUA->PushCFunction(lua_create_context);
          LUA->SetField(-2, "JS_CreateContext");

          LUA->PushCFunction(lua_free_context);
          LUA->SetField(-2, "JS_FreeContext");

          LUA->PushCFunction(lua_eval);
          LUA->SetField(-2, "JS_Eval");

          LUA->PushCFunction(lua_compile);
          LUA->SetField(-2, "JS_Compile");

          LUA->PushCFunction(lua_emit_event);
          LUA->SetField(-2, "JS_EmitEvent");
     LUA->Pop();

    lua_printf("Opened JS module...");
    return 0;
}

GMOD_MODULE_CLOSE() {
    for(int i = 0; i < MAX_CONTEXTS; i++) {
        if(contexts[i] != nullptr) {

            ContextData *contextData = (ContextData*)JS_GetContextOpaque(contexts[i]);

            for (int i = 0; i < contextData->listenerCount; i++) {
                if(contextData->listeners[i].eventName) {
                    free(contextData->listeners[i].eventName);
                }
                JS_FreeValue(contexts[i], contextData->listeners[i].callback);
            }

            JS_FreeValue(contexts[i], contextData->entIndex);
            JS_FreeValue(contexts[i], contextData->ownerIndex);

            JS_SetContextOpaque(contexts[i], NULL);
            free(contextData);

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
