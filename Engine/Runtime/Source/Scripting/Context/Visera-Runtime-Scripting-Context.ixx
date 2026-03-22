/** JavaScript context: one v8::Context per environment, API registration, and script execution entry. */
module;
#include <v8.h>

#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.Context;
#define VISERA_MODULE_NAME "Runtime.Scripting.Context"
import Visera.Runtime.Scripting.VM;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Log;

export namespace Visera
{
    /** Callback signature for C++ functions exposed to JS. */
    using FV8FunctionCallback = void (*)(const v8::FunctionCallbackInfo<v8::Value>&);

    /** Wraps v8::Context. One per window/environment (1:N). Holds API registry (RegisterFunction). */
    class VISERA_RUNTIME_API FJavaScriptContext
    {
    public:
        explicit FJavaScriptContext(FJavaScriptVM& I_VM);
        ~FJavaScriptContext();

        FJavaScriptContext(const FJavaScriptContext&)            = delete;
        FJavaScriptContext& operator=(const FJavaScriptContext&) = delete;
        FJavaScriptContext(FJavaScriptContext&&)                = delete;
        FJavaScriptContext& operator=(FJavaScriptContext&&)      = delete;

        /** Enter this context for script execution. Call from FScripting when running script or Tick. */
        [[nodiscard]] v8::Local<v8::Context>
        GetContext() const;

        /** Isolate that owns this context. */
        [[nodiscard]] v8::Isolate*
        GetIsolate() const { return VM ? VM->GetIsolate() : nullptr; }

        /** Register a C++ function on the global object so JS can call it (e.g. "Graphics.draw", callback). */
        void
        RegisterFunction(FStringView I_Name, FV8FunctionCallback I_Callback);

        /** Execute script source in this context. NullOpt on success; otherwise an error message. */
        [[nodiscard]] TOptional<FString>
        ExecuteScript(FStringView I_Source, FStringView I_FileName = "script");

    private:
        FJavaScriptVM* VM { nullptr };
        v8::Persistent<v8::Context> PersistentContext;
    };

    // --- Implementation ---

    FJavaScriptContext::FJavaScriptContext(FJavaScriptVM& I_VM)
        : VM(&I_VM)
    {
        v8::Isolate* Isolate = VM->GetIsolate();
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        v8::Local<v8::Context> Context = v8::Context::New(Isolate);
        PersistentContext.Reset(Isolate, Context);
    }

    FJavaScriptContext::~FJavaScriptContext()
    {
        if (VM && VM->GetIsolate() && !PersistentContext.IsEmpty())
        {
            PersistentContext.Reset();
        }
    }

    v8::Local<v8::Context> FJavaScriptContext::GetContext() const
    {
        if (PersistentContext.IsEmpty())
            return v8::Local<v8::Context>();
        return PersistentContext.Get(VM->GetIsolate());
    }

    void FJavaScriptContext::RegisterFunction(FStringView I_Name, FV8FunctionCallback I_Callback)
    {
        if (!VM || PersistentContext.IsEmpty() || !I_Callback)
            return;
        v8::Isolate* Isolate = VM->GetIsolate();
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        v8::Local<v8::Context> Context = PersistentContext.Get(Isolate);
        v8::Context::Scope ContextScope(Context);

        v8::Local<v8::FunctionTemplate> Template = v8::FunctionTemplate::New(Isolate, I_Callback);
        v8::Local<v8::Function> Function = Template->GetFunction(Context).ToLocalChecked();
        v8::MaybeLocal<v8::String> NameResult = ToV8String(Isolate, I_Name);
        if (NameResult.IsEmpty())
            return;

        v8::Local<v8::String> Name = NameResult.ToLocalChecked();
        v8::Local<v8::Object> Global = Context->Global();
        if (Global->Set(Context, Name, Function).IsNothing())
            LOG_WARN("(FJavaScriptContext) RegisterFunction failed for '{}'", I_Name);
    }

    TOptional<FString> FJavaScriptContext::ExecuteScript(FStringView I_Source, FStringView I_FileName)
    {
        if (!VM || PersistentContext.IsEmpty())
            return TOptional<FString>(FString("(no context)"));
        v8::Isolate* Isolate = VM->GetIsolate();
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        v8::Local<v8::Context> Context = GetContext();
        if (Context.IsEmpty())
            return TOptional<FString>(FString("(no context)"));
        return Visera::ExecuteScript(Isolate, Context, I_Source, I_FileName);
    }
}
