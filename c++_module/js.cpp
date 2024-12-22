#define GMMODULE
#define MAX_CONTEXTS 128
#define MAX_LISTENERS 10

#include "quickjs.h"
#include <stdio.h>
#include <cstring>
#include "GarrysMod/Lua/Interface.h"
#include "GarrysMod/Lua/LuaBase.h"
#include <math.h>

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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
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

JSValue inputs_change(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be an object");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, contextData->entIndex);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    JSPropertyEnum *properties;
    uint32_t propCount;

    if (JS_GetOwnPropertyNames(ctx, &properties, &propCount, argv[0], JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) < 0) {
        return JS_UNDEFINED;
    }

    LUA_CH->CreateTable();
    
    for (uint32_t i = 0; i < propCount; ++i) {
        const char *propName = JS_AtomToCString(ctx, properties[i].atom);
        if (propName) {
            JSValue value = JS_GetProperty(ctx, argv[0], properties[i].atom);

            const char *valueStr = JS_ToCString(ctx, value);

            if(strcmp(valueStr, "entity") == 0 || strcmp(valueStr, "vector") == 0 || strcmp(valueStr, "angle") == 0 || strcmp(valueStr, "string") == 0 || strcmp(valueStr, "color") == 0) {
                LUA_CH->PushString(valueStr);
                LUA_CH->SetField(-2, propName);
            } else {
                LUA_CH->PushString("normal");
                LUA_CH->SetField(-2, propName);
            }

            JS_FreeCString(ctx, propName);
            JS_FreeCString(ctx, valueStr);
            JS_FreeValue(ctx, value);
        }

        JS_FreeAtom(ctx, properties[i].atom);
    }

    js_free(ctx, properties);

    LUA_CH->GetField(-3, "SetInputs");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);
    LUA_CH->Pop(4);

    return JS_UNDEFINED;
}

JSValue outputs_change(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be an object");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, contextData->entIndex);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    JSPropertyEnum *properties;
    uint32_t propCount;

    if (JS_GetOwnPropertyNames(ctx, &properties, &propCount, argv[0], JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) < 0) {
        return JS_UNDEFINED;
    }

    LUA_CH->CreateTable();
    
    for (uint32_t i = 0; i < propCount; ++i) {
        const char *propName = JS_AtomToCString(ctx, properties[i].atom);
        if (propName) {
            JSValue value = JS_GetProperty(ctx, argv[0], properties[i].atom);

            const char *valueStr = JS_ToCString(ctx, value);

            if(strcmp(valueStr, "entity") == 0 || strcmp(valueStr, "vector") == 0 || strcmp(valueStr, "angle") == 0 || strcmp(valueStr, "string") == 0 || strcmp(valueStr, "color") == 0) {
                LUA_CH->PushString(valueStr);
                LUA_CH->SetField(-2, propName);
            } else {
                LUA_CH->PushString("normal");
                LUA_CH->SetField(-2, propName);
            }

            JS_FreeCString(ctx, propName);
            JS_FreeCString(ctx, valueStr);
            JS_FreeValue(ctx, value);
        }

        JS_FreeAtom(ctx, properties[i].atom);
    }

    js_free(ctx, properties);

    LUA_CH->GetField(-3, "SetOutputs");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);
    LUA_CH->Pop(4);

    return JS_UNDEFINED;
}

JSValue outputs_trigger(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (outputName, output)");
    }

    if(!JS_IsString(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument must be a string");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, contextData->entIndex);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    const char* outputName = JS_ToCString(ctx, argv[0]);

    LUA_CH->GetField(-2, "OutputType");
    LUA_CH->Push(-3);
    LUA_CH->PushString(outputName);
    LUA_CH->Call(2, 1);

    const char* outputType = LUA_CH->GetString(-1);

    if(strcmp(outputType, "NORMAL") == 0) {
        if(!JS_IsNumber(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }

        double outputNumber;
        JS_ToFloat64(ctx, &outputNumber, argv[1]);

        LUA_CH->PushNumber(outputNumber);
    } else if(strcmp(outputType, "VECTOR") == 0) {
        if(!JS_IsObject(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        double x;
        JS_ToFloat64(ctx, &x, JSx);
        double y;
        JS_ToFloat64(ctx, &y, JSy);
        double z;
        JS_ToFloat64(ctx, &z, JSz);

        LUA_CH->GetField(-4, "Vector");
        LUA_CH->PushNumber(x);
        LUA_CH->PushNumber(y);
        LUA_CH->PushNumber(z);
        LUA_CH->Call(3, 1);
    } else if(strcmp(outputType, "ANGLE") == 0) {
        if(!JS_IsObject(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid angle");
        }

        JSValue JSpitch = JS_GetPropertyStr(ctx, argv[1], "pitch");
        JSValue JSyaw = JS_GetPropertyStr(ctx, argv[1], "yaw");
        JSValue JSroll = JS_GetPropertyStr(ctx, argv[1], "roll");

        if(!JS_IsNumber(JSpitch) || !JS_IsNumber(JSyaw) || !JS_IsNumber(JSroll)) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid angle");
        }

        double pitch;
        JS_ToFloat64(ctx, &pitch, JSpitch);
        double yaw;
        JS_ToFloat64(ctx, &yaw, JSyaw);
        double roll;
        JS_ToFloat64(ctx, &roll, JSroll);

        LUA_CH->GetField(-4, "Angle");
        LUA_CH->PushNumber(pitch);
        LUA_CH->PushNumber(yaw);
        LUA_CH->PushNumber(roll);
        LUA_CH->Call(3, 1);
    } else if(strcmp(outputType, "COLOR") == 0) {
        if(!JS_IsObject(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid color");
        }

        if(!JS_IsObject(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid angle");
        }

        JSValue JSr = JS_GetPropertyStr(ctx, argv[1], "r");
        JSValue JSg = JS_GetPropertyStr(ctx, argv[1], "g");
        JSValue JSb = JS_GetPropertyStr(ctx, argv[1], "b");
        JSValue JSa = JS_GetPropertyStr(ctx, argv[1], "a");

        if(!JS_IsNumber(JSr) || !JS_IsNumber(JSg) || !JS_IsNumber(JSb)) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid angle");
        }

        double r;
        JS_ToFloat64(ctx, &r, JSr);
        double g;
        JS_ToFloat64(ctx, &g, JSg);
        double b;
        JS_ToFloat64(ctx, &b, JSb);

        double a;
        if(JS_IsNumber(JSr)) {
            JS_ToFloat64(ctx, &a, JSa);
        } else {
            a = 255;
        }

        LUA_CH->GetField(-4, "Color");
        LUA_CH->PushNumber(r);
        LUA_CH->PushNumber(g);
        LUA_CH->PushNumber(b);
        LUA_CH->PushNumber(a);
        LUA_CH->Call(4, 1);
    } else if(strcmp(outputType, "ENTITY") == 0) {
        if(!JS_IsNumber(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }

        double entityIndex;
        JS_ToFloat64(ctx, &entityIndex, argv[1]);

        LUA_CH->GetField(-4, "Entity");
        LUA_CH->PushNumber(entityIndex);
        LUA_CH->Call(1, 1);

        LUA_CH->GetField(-1, "IsValid");
        LUA_CH->Push(-2);
        LUA_CH->Call(1, 1);

        if(!LUA_CH->GetBool(-1)) {
            LUA_CH->Pop(6);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid entity");
        }
        LUA_CH->Pop();
        
    } else if(strcmp(outputType, "STRING") == 0) {
        if(!JS_IsString(argv[1])) {
            LUA_CH->Pop(4);
            return JS_ThrowTypeError(ctx, "Second argument is not a valid string");
        }

        const char* outputString = JS_ToCString(ctx, argv[1]);

        LUA_CH->PushString(outputString);
        
        JS_FreeCString(ctx, outputString);

    } else {
        LUA_CH->Pop(4);
        return JS_ThrowTypeError(ctx, "First argument is not a valid output");
    }

    LUA_CH->GetField(-4, "TriggerOutput");
    LUA_CH->Push(-5);
    LUA_CH->PushString(outputName);
    LUA_CH->Push(-4);
    LUA_CH->Call(3, 0);
    LUA_CH->Pop(5);

    JS_FreeCString(ctx, outputName);

    return JS_UNDEFINED;
}

JSValue vector_add(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, x + x2));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, y + y2));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, z + z2));

    return JSvec;
}

JSValue vector_mul(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, x * x2));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, y * y2));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, z * z2));

    return JSvec;
}

JSValue vector_sub(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, x - x2));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, y - y2));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, z - z2));

    return JSvec;
}

JSValue vector_div(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, x / x2));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, y / y2));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, z / z2));

    return JSvec;
}

JSValue vector_mod(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vecto");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, fmod(x, x2)));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, fmod(y, y2)));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, fmod(z, z2)));

    return JSvec;
}

JSValue vector_pow(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector or number)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    double x;
    double y;
    double z;
    double x2;
    double y2;
    double z2;

    if(!JS_IsObject(argv[1])) {
        if(!JS_IsNumber(argv[1])) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid number");
        }
        
        JS_ToFloat64(ctx, &x2, argv[1]);
        y2 = x2;
        z2 = x2;
    } else {
        JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
        JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
        JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

        if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
            return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
        }

        JS_ToFloat64(ctx, &x2, JSx2);
        JS_ToFloat64(ctx, &y2, JSy2);
        JS_ToFloat64(ctx, &z2, JSz2);
    }

    JS_ToFloat64(ctx, &x, JSx);
    JS_ToFloat64(ctx, &y, JSy);
    JS_ToFloat64(ctx, &z, JSz);

    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, pow(x,  x2)));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, pow(y, y2)));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, pow(z, z2)));

    return JSvec;
}

JSValue vector_distance(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    double x2;
    JS_ToFloat64(ctx, &x2, JSx2);
    double y2;
    JS_ToFloat64(ctx, &y2, JSy2);
    double z2;
    JS_ToFloat64(ctx, &z2, JSz2);

    JSValue distance = JS_NewFloat64(ctx, sqrt((x2 - x) * (x2 - x) + (y2 - y) * (y2 - y) + (z2 - z) * (z2 - z)));

    return distance;
}

JSValue vector_distance2(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (vector, vector)");
    }

    if(!JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[0], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[0], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "First argument is not a valid vector");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx2 = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy2 = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz2 = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    double x2;
    JS_ToFloat64(ctx, &x2, JSx2);
    double y2;
    JS_ToFloat64(ctx, &y2, JSy2);
    double z2;
    JS_ToFloat64(ctx, &z2, JSz2);

    JSValue distance = JS_NewFloat64(ctx, (x2 - x) * (x2 - x) + (y2 - y) * (y2 - y) + (z2 - z) * (z2 - z));

    return distance;
}

JSValue vector_magnitude(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    JSValue magnitude = JS_NewFloat64(ctx, sqrt(x * x + y * y + z * z));

    return magnitude;
}

JSValue vector_normalize(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    double magnitude = sqrt(x * x + y * y + z * z);

    JSValue normalized = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, normalized, "x", JS_NewFloat64(ctx, x / magnitude));
    JS_SetPropertyStr(ctx, normalized, "y", JS_NewFloat64(ctx, y / magnitude));
    JS_SetPropertyStr(ctx, normalized, "z", JS_NewFloat64(ctx, z / magnitude));

    return normalized;
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
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

JSValue entity_set_position(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, vector)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-3, "Vector");
    LUA_CH->PushNumber(x);
    LUA_CH->PushNumber(y);
    LUA_CH->PushNumber(z);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-3, "SetPos");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);

    LUA_CH->Pop(4);

    return JS_UNDEFINED;
}

JSValue entity_set_color(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, color)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSr = JS_GetPropertyStr(ctx, argv[1], "r");
    JSValue JSg = JS_GetPropertyStr(ctx, argv[1], "g");
    JSValue JSb = JS_GetPropertyStr(ctx, argv[1], "b");
    JSValue JSa = JS_GetPropertyStr(ctx, argv[1], "a");

    if(!JS_IsNumber(JSr) || !JS_IsNumber(JSg) || !JS_IsNumber(JSb)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid color");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double r;
    JS_ToFloat64(ctx, &r, JSr);
    double g;
    JS_ToFloat64(ctx, &g, JSg);
    double b;
    JS_ToFloat64(ctx, &b, JSb);
    double a;

    if(!JS_IsNumber(JSa)) {
        a = 255;
    } else {
        JS_ToFloat64(ctx, &a, JSa);
    }

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-3, "Color");
    LUA_CH->PushNumber(r);
    LUA_CH->PushNumber(g);
    LUA_CH->PushNumber(b);
    LUA_CH->PushNumber(a);
    LUA_CH->Call(4, 1);

    LUA_CH->GetField(-3, "SetColor");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);

    LUA_CH->Pop(4);

    return JS_UNDEFINED;
}

JSValue entity_set_angles(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, angle)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSpitch = JS_GetPropertyStr(ctx, argv[1], "pitch");
    JSValue JSyaw = JS_GetPropertyStr(ctx, argv[1], "yaw");
    JSValue JSroll = JS_GetPropertyStr(ctx, argv[1], "roll");

    if(!JS_IsNumber(JSpitch) || !JS_IsNumber(JSyaw) || !JS_IsNumber(JSroll)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid angle");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double pitch;
    JS_ToFloat64(ctx, &pitch, JSpitch);
    double yaw;
    JS_ToFloat64(ctx, &yaw, JSyaw);
    double roll;
    JS_ToFloat64(ctx, &roll, JSroll);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-3, "Angle");
    LUA_CH->PushNumber(pitch);
    LUA_CH->PushNumber(yaw);
    LUA_CH->PushNumber(roll);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-3, "SetAngles");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);

    LUA_CH->Pop(4);

    return JS_UNDEFINED;
}

JSValue entity_use(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected at least one argument");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    ContextData *contextData = (ContextData *)JS_GetContextOpaque(ctx);

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);

    double ownerIndex;
    JS_ToFloat64(ctx, &ownerIndex, contextData->ownerIndex);
    
    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-3, "Entity");
    LUA_CH->PushNumber(ownerIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(5);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-4, "Use");
    LUA_CH->Push(-5);
    LUA_CH->Push(-4);
    LUA_CH->Call(2, 0);
    LUA_CH->Pop(5);

    return JS_UNDEFINED;
}

JSValue entity_crouching(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid playerr");
    }

    LUA_CH->GetField(-3, "Crouching");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue IsCrouching = JS_NewBool(ctx, LUA_CH->GetBool(-1));

    LUA_CH->Pop(5);

    return IsCrouching;
}

JSValue entity_team(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid player");
    }

    LUA_CH->GetField(-3, "Team");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue team = JS_NewFloat64(ctx, LUA_CH->GetNumber(-1));

    LUA_CH->Pop(5);

    return team;
}

JSValue entity_frags(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid player");
    }

    LUA_CH->GetField(-3, "Frags");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue frags = JS_NewFloat64(ctx, LUA_CH->GetNumber(-1));

    LUA_CH->Pop(5);

    return frags;
}

JSValue entity_steamid(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid player");
    }

    LUA_CH->GetField(-3, "SteamID");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue steamID = JS_NewString(ctx, LUA_CH->GetString(-1));

    LUA_CH->Pop(5);

    return steamID;
}

JSValue entity_steamid64(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid player");
    }

    LUA_CH->GetField(-3, "SteamID64");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue steamID = JS_NewString(ctx, LUA_CH->GetString(-1));

    LUA_CH->Pop(5);

    return steamID;
}

JSValue entity_accountid(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "IsPlayer");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid player");
    }

    LUA_CH->GetField(-3, "AccountID");
    LUA_CH->Push(-4);
    LUA_CH->Call(1, 1);

    JSValue accountID = JS_NewString(ctx, LUA_CH->GetString(-1));

    LUA_CH->Pop(5);

    return accountID;
}

JSValue entity_get_position(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetPos");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    Vector vec = LUA_CH->GetVector(-1);
                        
    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, vec.y));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, vec.z));

    LUA_CH->Pop(4);

    return JSvec;
}

JSValue entity_get_color(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    JSValue JSang = JS_NewObject(ctx);

    LUA_CH->GetField(-2, "GetColor");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->PushString("r");
    LUA_CH->GetTable(-2);
    JS_SetPropertyStr(ctx, JSang, "r", JS_NewFloat64(ctx, LUA_CH->GetNumber(-1)));

    LUA_CH->PushString("g");
    LUA_CH->GetTable(-3);
    JS_SetPropertyStr(ctx, JSang, "g", JS_NewFloat64(ctx, LUA_CH->GetNumber(-1)));

    LUA_CH->PushString("b");
    LUA_CH->GetTable(-4);
    JS_SetPropertyStr(ctx, JSang, "b", JS_NewFloat64(ctx, LUA_CH->GetNumber(-1)));

    LUA_CH->PushString("a");
    LUA_CH->GetTable(-5);
    JS_SetPropertyStr(ctx, JSang, "a", JS_NewFloat64(ctx, LUA_CH->GetNumber(-1)));

    LUA_CH->Pop(8);

    return JSang;
}

JSValue entity_get_angles(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetAngles");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    QAngle ang = LUA_CH->GetAngle(-1);
                        
    JSValue JSang = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSang, "pitch", JS_NewFloat64(ctx, ang.x));
    JS_SetPropertyStr(ctx, JSang, "yaw", JS_NewFloat64(ctx, ang.y));
    JS_SetPropertyStr(ctx, JSang, "roll", JS_NewFloat64(ctx, ang.z));

    LUA_CH->Pop(4);

    return JSang;
}

JSValue entity_eye_angles(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "EyeAngles");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    QAngle ang = LUA_CH->GetAngle(-1);
                        
    JSValue JSang = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSang, "pitch", JS_NewFloat64(ctx, ang.x));
    JS_SetPropertyStr(ctx, JSang, "yaw", JS_NewFloat64(ctx, ang.y));
    JS_SetPropertyStr(ctx, JSang, "roll", JS_NewFloat64(ctx, ang.z));

    LUA_CH->Pop(4);

    return JSang;
}

JSValue entity_isvalid_physics(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetPhysicsObject");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    JSValue IsValid = JS_NewBool(ctx, LUA_CH->GetBool(-1));
    LUA_CH->Pop(3);

    return IsValid;
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
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

JSValue entity_isvalid(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    JSValue IsValid = JS_NewBool(ctx, LUA_CH->GetBool(-1));
    LUA_CH->Pop(3);

    return IsValid;
}

JSValue entity_isplayer(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsPlayer");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    JSValue IsPlayer = JS_NewBool(ctx, LUA_CH->GetBool(-1));
    LUA_CH->Pop(3);

    return IsPlayer;
}

JSValue entity_isnpc(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsNPC");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    JSValue IsNPC = JS_NewBool(ctx, LUA_CH->GetBool(-1));
    LUA_CH->Pop(3);

    return IsNPC;
}

JSValue entity_isworld(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsWorld");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    JSValue IsWorld = JS_NewBool(ctx, LUA_CH->GetBool(-1));
    LUA_CH->Pop(3);

    return IsWorld;
}

JSValue entity_get_masscenter(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-2, "GetPhysicsObject");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(5);
        return JS_ThrowTypeError(ctx, "Invalid physics object");
    }

    LUA_CH->GetField(-2, "GetMassCenter");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    Vector vec = LUA_CH->GetVector(-1);
                        
    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, vec.y));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, vec.z));

    LUA_CH->Pop(6);

    return JSvec;
}


JSValue entity_get_mass(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
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

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-2, "GetPhysicsObject");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);
    
    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(5);
        return JS_ThrowTypeError(ctx, "Invalid physics object");
    }

    LUA_CH->GetField(-2, "GetMass");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);
    
    JSValue mass = JS_NewFloat64(ctx, LUA_CH->GetNumber(-1));

    LUA_CH->Pop(6);

    return mass;
}

JSValue entity_local_to_world(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, vector)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-5, "Vector");
    LUA_CH->PushNumber(x);
    LUA_CH->PushNumber(y);
    LUA_CH->PushNumber(z);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-3, "LocalToWorld");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 1);

    Vector vec = LUA_CH->GetVector(-1);
                        
    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, vec.y));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, vec.z));

    LUA_CH->Pop(6);

    return JSvec;
}

JSValue entity_world_to_local(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, vector)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "Argument must be a number");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_NewBool(ctx, false);
    }

    LUA_CH->GetField(-5, "Vector");
    LUA_CH->PushNumber(x);
    LUA_CH->PushNumber(y);
    LUA_CH->PushNumber(z);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-3, "WorldToLocal");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 1);

    Vector vec = LUA_CH->GetVector(-1);
                        
    JSValue JSvec = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, JSvec, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, JSvec, "y", JS_NewFloat64(ctx, vec.y));
    JS_SetPropertyStr(ctx, JSvec, "z", JS_NewFloat64(ctx, vec.z));

    LUA_CH->Pop(6);

    return JSvec;
}

JSValue entity_applyforce_center(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected (entity, vector)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument must be a number");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetPhysicsObject");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(5);
        return JS_ThrowTypeError(ctx, "Invalid physics object");
    }

    LUA_CH->GetField(-5, "Vector");
    LUA_CH->PushNumber(x);
    LUA_CH->PushNumber(y);
    LUA_CH->PushNumber(z);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-3, "ApplyForceCenter");
    LUA_CH->Push(-4);
    LUA_CH->Push(-3);
    LUA_CH->Call(2, 0);
    LUA_CH->Pop(6);

    return JS_UNDEFINED;
}

JSValue entity_applyforce_offset(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    if(argc < 3) {
        return JS_ThrowTypeError(ctx, "Expected (entity, vector, vector)");
    }

    if(!JS_IsNumber(argv[0])) {
        return JS_ThrowTypeError(ctx, "First argument must be a number");
    }

    if(!JS_IsObject(argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    if(!JS_IsObject(argv[2])) {
        return JS_ThrowTypeError(ctx, "Third argument is not a valid vector");
    }

    JSValue JSx = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue JSy = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue JSz = JS_GetPropertyStr(ctx, argv[1], "z");

    JSValue JSx2 = JS_GetPropertyStr(ctx, argv[2], "x");
    JSValue JSy2 = JS_GetPropertyStr(ctx, argv[2], "y");
    JSValue JSz2 = JS_GetPropertyStr(ctx, argv[2], "z");

    if(!JS_IsNumber(JSx) || !JS_IsNumber(JSy) || !JS_IsNumber(JSz)) {
        return JS_ThrowTypeError(ctx, "Second argument is not a valid vector");
    }

    if(!JS_IsNumber(JSx2) || !JS_IsNumber(JSy2) || !JS_IsNumber(JSz2)) {
        return JS_ThrowTypeError(ctx, "Third argument is not a valid vector");
    }

    double entityIndex;
    JS_ToFloat64(ctx, &entityIndex, argv[0]);
    double x;
    JS_ToFloat64(ctx, &x, JSx);
    double y;
    JS_ToFloat64(ctx, &y, JSy);
    double z;
    JS_ToFloat64(ctx, &z, JSz);

    double x2;
    JS_ToFloat64(ctx, &x, JSx2);
    double y2;
    JS_ToFloat64(ctx, &y, JSy2);
    double z2;
    JS_ToFloat64(ctx, &z, JSz2);

    LUA_CH->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA_CH->GetField(-1, "Entity");
    LUA_CH->PushNumber(entityIndex);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(3);
        return JS_ThrowTypeError(ctx, "Invalid entity");
    }

    LUA_CH->GetField(-2, "GetPhysicsObject");
    LUA_CH->Push(-3);
    LUA_CH->Call(1, 1);

    LUA_CH->GetField(-1, "IsValid");
    LUA_CH->Push(-2);
    LUA_CH->Call(1, 1);

    if(!LUA_CH->GetBool(-1)) {
        LUA_CH->Pop(5);
        return JS_ThrowTypeError(ctx, "Invalid physics object");
    }

    LUA_CH->GetField(-5, "Vector");
    LUA_CH->PushNumber(x);
    LUA_CH->PushNumber(y);
    LUA_CH->PushNumber(z);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-6, "Vector");
    LUA_CH->PushNumber(x2);
    LUA_CH->PushNumber(y2);
    LUA_CH->PushNumber(z2);
    LUA_CH->Call(3, 1);

    LUA_CH->GetField(-4, "ApplyForceOffset");
    LUA_CH->Push(-5);
    LUA_CH->Push(-4);
    LUA_CH->Push(-4);
    LUA_CH->Call(3, 0);
    LUA_CH->Pop(7);

    return JS_UNDEFINED;
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
                            args[k - 3] = JS_NewFloat64(ctx, index);
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
            JSValue apply_force = JS_NewCFunction(ctx, entity_applyforce_center, "ApplyForce", 1);
            JSValue apply_force_offset = JS_NewCFunction(ctx, entity_applyforce_offset, "ApplyForceOffset", 1);
            JSValue get_pos = JS_NewCFunction(ctx, entity_get_position, "GetPos", 1);
            JSValue get_mass_center = JS_NewCFunction(ctx, entity_get_masscenter, "GetMassCenter", 1);
            JSValue is_valid = JS_NewCFunction(ctx, entity_isvalid, "IsValid", 1);
            JSValue is_valid_physics = JS_NewCFunction(ctx, entity_isvalid_physics, "IsValidPhysics", 1);
            JSValue local_to_world = JS_NewCFunction(ctx, entity_local_to_world, "LocalToWorld", 1);
            JSValue world_to_local = JS_NewCFunction(ctx, entity_local_to_world, "WorldToLocal", 1);
            JSValue is_world = JS_NewCFunction(ctx, entity_isworld, "IsWorld", 1);
            JSValue get_color = JS_NewCFunction(ctx, entity_get_color, "GetColor", 1);
            JSValue get_angles = JS_NewCFunction(ctx, entity_get_angles, "GetAngles", 1);
            JSValue eye_angles = JS_NewCFunction(ctx, entity_eye_angles, "EyeAngles", 1);
            JSValue is_player = JS_NewCFunction(ctx, entity_isplayer, "IsPlayer", 1);
            JSValue is_npc = JS_NewCFunction(ctx, entity_isnpc, "IsNPC", 1);
            JSValue crouching = JS_NewCFunction(ctx, entity_crouching, "Crouching", 1);
            JSValue steam_id = JS_NewCFunction(ctx, entity_steamid, "SteamID", 1);
            JSValue steam_id64 = JS_NewCFunction(ctx, entity_steamid64, "SteamID64", 1);
            JSValue account_id = JS_NewCFunction(ctx, entity_accountid, "AccountID", 1);
            JSValue team = JS_NewCFunction(ctx, entity_team, "Team", 1);
            JSValue frags = JS_NewCFunction(ctx, entity_frags, "Frags", 1);
            JSValue use = JS_NewCFunction(ctx, entity_use, "Use", 1);
            JSValue set_color = JS_NewCFunction(ctx, entity_set_color, "SetColor", 1);
            JSValue set_pos = JS_NewCFunction(ctx, entity_set_position, "SetPos", 1);
            JSValue set_angles = JS_NewCFunction(ctx, entity_set_angles, "SetAngles", 1);
            JSValue get_mass = JS_NewCFunction(ctx, entity_get_mass, "GetMass", 1);

            JS_SetPropertyStr(ctx, entity_obj, "Chip", entity);
            JS_SetPropertyStr(ctx, entity_obj, "Owner", owner);
            JS_SetPropertyStr(ctx, entity_obj, "GetName", get_name);
            JS_SetPropertyStr(ctx, entity_obj, "GetClass", get_class);
            JS_SetPropertyStr(ctx, entity_obj, "ApplyForce", apply_force);
            JS_SetPropertyStr(ctx, entity_obj, "ApplyForceOffset", apply_force_offset);
            JS_SetPropertyStr(ctx, entity_obj, "IsValid", is_valid);
            JS_SetPropertyStr(ctx, entity_obj, "IsValidPhysics", is_valid_physics);
            JS_SetPropertyStr(ctx, entity_obj, "GetPos", get_pos);
            JS_SetPropertyStr(ctx, entity_obj, "GetMassCenter", get_mass_center);
            JS_SetPropertyStr(ctx, entity_obj, "ToWorld", local_to_world);
            JS_SetPropertyStr(ctx, entity_obj, "ToLocal", world_to_local);
            JS_SetPropertyStr(ctx, entity_obj, "IsWorld", is_world);
            JS_SetPropertyStr(ctx, entity_obj, "GetColor", get_color);
            JS_SetPropertyStr(ctx, entity_obj, "GetAngles", get_angles);
            JS_SetPropertyStr(ctx, entity_obj, "IsPlayer", is_player);
            JS_SetPropertyStr(ctx, entity_obj, "IsNPC", is_npc);
            JS_SetPropertyStr(ctx, entity_obj, "Crouching", crouching);
            JS_SetPropertyStr(ctx, entity_obj, "EyeAngles", eye_angles);
            JS_SetPropertyStr(ctx, entity_obj, "SteamID", steam_id);
            JS_SetPropertyStr(ctx, entity_obj, "Team", team);
            JS_SetPropertyStr(ctx, entity_obj, "Frags", frags);
            JS_SetPropertyStr(ctx, entity_obj, "SteamID64", steam_id64);
            JS_SetPropertyStr(ctx, entity_obj, "AccountID", account_id);
            JS_SetPropertyStr(ctx, entity_obj, "Use", use);
            JS_SetPropertyStr(ctx, entity_obj, "SetColor", set_color);
            JS_SetPropertyStr(ctx, entity_obj, "SetAngles", set_angles);
            JS_SetPropertyStr(ctx, entity_obj, "SetPos", set_pos);
            JS_SetPropertyStr(ctx, entity_obj, "GetMass", get_mass);
            JS_SetPropertyStr(ctx, global_obj, "Entity", entity_obj);

            JSValue inputs_obj = JS_NewObject(ctx);
            JSValue input_change = JS_NewCFunction(ctx, inputs_change, "Change", 1);
            
            JS_SetPropertyStr(ctx, inputs_obj, "Change", input_change);
            JS_SetPropertyStr(ctx, global_obj, "Inputs", inputs_obj);

            JSValue outputs_obj = JS_NewObject(ctx);
            JSValue output_change = JS_NewCFunction(ctx, outputs_change, "Change", 1);
            JSValue output_trigger = JS_NewCFunction(ctx, outputs_trigger, "Trigger", 1);
            
            JS_SetPropertyStr(ctx, outputs_obj, "Change", output_change);
            JS_SetPropertyStr(ctx, outputs_obj, "Trigger", output_trigger);
            JS_SetPropertyStr(ctx, global_obj, "Outputs", outputs_obj);

            JSValue vector_obj = JS_NewObject(ctx);
            JSValue vec_add = JS_NewCFunction(ctx, vector_add, "Add", 1);
            JSValue vec_sub = JS_NewCFunction(ctx, vector_sub, "Sub", 1);
            JSValue vec_mul = JS_NewCFunction(ctx, vector_mul, "Mul", 1);
            JSValue vec_div = JS_NewCFunction(ctx, vector_div, "Div", 1);
            JSValue vec_pow = JS_NewCFunction(ctx, vector_pow, "Pow", 1);
            JSValue vec_mod = JS_NewCFunction(ctx, vector_mod, "Mod", 1);
            JSValue vec_magnitude = JS_NewCFunction(ctx, vector_magnitude, "Magnitude", 1);
            JSValue vec_distance = JS_NewCFunction(ctx, vector_distance, "Distance", 1);
            JSValue vec_distance2 = JS_NewCFunction(ctx, vector_distance, "Distance2", 1);
            JSValue vec_normalize = JS_NewCFunction(ctx, vector_distance2, "Normalize", 1);

            JS_SetPropertyStr(ctx, vector_obj, "Add", vec_add);
            JS_SetPropertyStr(ctx, vector_obj, "Sub", vec_sub);
            JS_SetPropertyStr(ctx, vector_obj, "Mul", vec_mul);
            JS_SetPropertyStr(ctx, vector_obj, "Div", vec_div);
            JS_SetPropertyStr(ctx, vector_obj, "Pow", vec_pow);
            JS_SetPropertyStr(ctx, vector_obj, "Mod", vec_mod);
            JS_SetPropertyStr(ctx, vector_obj, "Magnitude", vec_magnitude);
            JS_SetPropertyStr(ctx, vector_obj, "Distance", vec_distance);
            JS_SetPropertyStr(ctx, vector_obj, "Distance2", vec_distance2);
            JS_SetPropertyStr(ctx, vector_obj, "Normalize", vec_normalize);
            JS_SetPropertyStr(ctx, global_obj, "Vector", vector_obj);

            JSValue console_obj = JS_NewObject(ctx);
            JSValue log_func = JS_NewCFunction(ctx, console_log, "log", 1);
            JS_SetPropertyStr(ctx, console_obj, "log", log_func);
            JS_SetPropertyStr(ctx, global_obj, "console", console_obj);

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
