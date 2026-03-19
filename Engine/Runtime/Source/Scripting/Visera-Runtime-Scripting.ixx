/** Scripting service: VM + Context + bindings, entry script loading, and per-frame OnTick(dt) from JS. */
module;
#include <v8.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting;
#define VISERA_MODULE_NAME "Runtime.Scripting"
export import Visera.Runtime.Scripting.VM;
export import Visera.Runtime.Scripting.Context;
export import Visera.Runtime.Scripting.Binding;
       import Visera.Runtime.Graphics;
       import Visera.Runtime.AssetHub;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.Path;
       import Visera.Core.Containers.Array;
       import Visera.Platform;
       import Visera.Core.Log;

export namespace Visera
{
    struct VISERA_RUNTIME_API FScriptingCreateInfo
    {
        VPath EntryScriptPath{FStringView{"@assets://scripts/main.js"}};
    };

    /** Scripting service: owns VM and Context, loads entry script, runs OnTick(dt) from JS. */
    class VISERA_RUNTIME_API FScripting
    {
    public:
        /** I_Graphics and I_AssetHub must outlive FScripting. */
        FScripting(const FScriptingCreateInfo& I_CreateInfo, FGraphics* I_Graphics, FAssetHub* I_AssetHub);
        ~FScripting();

        FScripting(const FScripting&)            = delete;
        FScripting& operator=(const FScripting&)  = delete;
        FScripting(FScripting&&)                  = delete;
        FScripting& operator=(FScripting&&)        = delete;

        /** Run JS global OnTick(dt). No-op if OnTick is not defined or not a function. */
        void Tick(Double I_DeltaTime);

    private:
        /** Load and execute the entry script file; returns false on read or run failure. */
        Bool LoadEntryScript(const FPath& I_EntryPath);

        TUniquePtr<FJavaScriptVM>     VM;
        TUniquePtr<FJavaScriptContext> Context;
        FGraphics*                    Graphics { nullptr };
    };

    // --- Implementation ---

    FScripting::FScripting(const FScriptingCreateInfo& I_CreateInfo, FGraphics* I_Graphics, FAssetHub* I_AssetHub)
        : Graphics(I_Graphics)
    {
        VM = MakeUnique<FJavaScriptVM>();
        if (!VM)
            return;
        Context = MakeUnique<FJavaScriptContext>(*VM);
        if (!Context)
        {
            VM.Reset();
            return;
        }
        RegisterAllBindings(Graphics, *Context);

        const FPath EntryPath = I_AssetHub ? I_AssetHub->ResolvePath(I_CreateInfo.EntryScriptPath) : FPath{};
        if (EntryPath.IsEmpty() || !LoadEntryScript(EntryPath))
            LOG_WARN("(FScripting) Entry script load/run failed: {}", I_CreateInfo.EntryScriptPath);
    }

    FScripting::~FScripting()
    {
        Context.Reset();
        VM.Reset();
        Graphics = nullptr;
    }

    Bool FScripting::LoadEntryScript(const FPath& I_EntryPath)
    {
        if (!Context || !VM)
            return False;
        try
        {
            TOptional<TArray<FByte>> BytesOpt = FPlatform::ReadFile(I_EntryPath);
            if (!BytesOpt.HasValue() || BytesOpt.GetValue().IsEmpty())
            {
                LOG_WARN("(FScripting) ReadFile failed: {}", I_EntryPath);
                return False;
            }
            const TArray<FByte>& Bytes = BytesOpt.GetValue();
            FStringView Source(reinterpret_cast<const char*>(Bytes.Data()), Bytes.GetSize());
            FStringView FileNameDefault("main.js");
            FStringView FileName = I_EntryPath.GetFileName().HasValue() ? I_EntryPath.GetFileName().GetValue() : FileNameDefault;
            TOptional<FString> Err = Context->ExecuteScript(Source, FileName);
            return !Err.HasValue();
        }
        catch (...)
        {
            LOG_ERROR("(FScripting) LoadEntryScript threw.");
            return False;
        }
    }

    void FScripting::Tick(Double I_DeltaTime)
    {
        if (!Context || !VM)
            return;
        v8::Isolate* Isolate = VM->GetIsolate();
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        v8::Local<v8::Context> ContextHandle = Context->GetContext();
        if (ContextHandle.IsEmpty())
            return;
        v8::Context::Scope ContextScope(ContextHandle);

        v8::MaybeLocal<v8::String> OnTickName = ToV8String(Isolate, "OnTick");
        if (OnTickName.IsEmpty())
            return;
        v8::Local<v8::Value> OnTickVal;
        if (!ContextHandle->Global()->Get(ContextHandle, OnTickName.ToLocalChecked()).ToLocal(&OnTickVal) || !OnTickVal->IsFunction())
            return;

        v8::Local<v8::Function> OnTickFn = OnTickVal.As<v8::Function>();
        v8::Local<v8::Value> Arg = v8::Number::New(Isolate, I_DeltaTime);
        v8::Local<v8::Value> Receiver = ContextHandle->Global();
        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Result = OnTickFn->Call(ContextHandle, Receiver, 1, &Arg);
        if (Result.IsEmpty() && TryCatch.HasCaught())
            LOG_ERROR("(FScripting) OnTick(dt) threw.");
    }
}
