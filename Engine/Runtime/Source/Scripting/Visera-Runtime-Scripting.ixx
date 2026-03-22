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
       import Visera.Runtime.Audio;
       import Visera.Runtime.AssetHub;
       import Visera.Runtime.Input;
       import Visera.Runtime.Window;
       import Visera.Runtime.Scripting.Binding.Input;
       import Visera.Runtime.Scripting.Utils;
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
        VPath EntryScriptPath{"@assets://scripts/main.js"};
    };

    /** Scripting service: owns VM and Context, loads entry script, runs OnTick(dt) from JS. */
    class VISERA_RUNTIME_API FScripting
    {
    public:
        /**
         * I_Graphics, I_AssetHub, optional I_Audio, and optional I_Input must outlive FScripting.
         * When I_RegisterScriptMainWindow is true, visera.setMainWindow is exposed; call ResolveMainWindow() after construction to create the main window if FEngineCreateInfo.MainWindow was empty.
         */
        FScripting(const FScriptingCreateInfo& I_CreateInfo, FGraphics* I_Graphics, FAssetHub* I_AssetHub, FAudio* I_Audio, FInput* I_Input, Bool I_RegisterScriptMainWindow);
        ~FScripting();

        FScripting(const FScripting&)            = delete;
        FScripting& operator=(const FScripting&)  = delete;
        FScripting(FScripting&&)                  = delete;
        FScripting& operator=(FScripting&&)        = delete;

        /** Value written by visera.setMainWindow in OnInit, if any; empty => headless when FEngineCreateInfo.MainWindow was unset. */
        [[nodiscard]] TOptional<FWindowCreateInfo> ResolveMainWindow() const;

        /** Run JS global OnTick(dt). No-op if OnTick is not defined or not a function. */
        void Tick(Double I_DeltaTime);

    private:
        /** Load and execute the entry script file; returns false on read or run failure. */
        Bool LoadEntryScript(const FPath& I_EntryPath);
        /** Invoke global OnInit() once after entry script load if it exists. */
        void RunOnInit();

        TUniquePtr<FJavaScriptVM>                 VM;
        TUniquePtr<FJavaScriptContext>            Context;
        TUniquePtr<FJavaScriptInputBindingState>  InputBindingState;
        FGraphics*                                Graphics { nullptr };
        FInput*                                   Input { nullptr };
        TOptional<FWindowCreateInfo>              ScriptMainWindow;
    };
}

namespace Visera
{
    /** Log V8 message, optional source line, and stack (same detail pattern as ExecuteScript). */
    inline void
    LogV8TryCatch(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context, v8::TryCatch& IO_TryCatch, const char* I_Label)
    {
        if (!IO_TryCatch.HasCaught())
            return;

        if (!IO_TryCatch.Message().IsEmpty())
        {
            v8::Local<v8::Message> Message = IO_TryCatch.Message();
            const FString Text = FromV8String(I_Isolate, Message->Get()).Get(FString("(no message)"));
            LOG_ERROR("(FScripting) {}: {}", I_Label, Text);

            v8::Local<v8::Value> ResourceName = Message->GetScriptOrigin().ResourceName();
            if (!ResourceName.IsEmpty() && ResourceName->IsString())
            {
                if (TOptional<FString> Src = FromV8String(I_Isolate, ResourceName); Src.HasValue())
                {
                    v8::Maybe<int> LineMaybe = Message->GetLineNumber(I_Context);
                    if (LineMaybe.IsJust())
                        LOG_ERROR("(FScripting) {} @ {}:{}", I_Label, Src.GetValue(), LineMaybe.FromJust());
                    else
                        LOG_ERROR("(FScripting) {} @ {}", I_Label, Src.GetValue());
                }
            }
            else
            {
                v8::Maybe<int> LineMaybe = Message->GetLineNumber(I_Context);
                if (LineMaybe.IsJust())
                    LOG_ERROR("(FScripting) {} (line {})", I_Label, LineMaybe.FromJust());
            }
        }
        else
        {
            v8::Local<v8::Value> Ex = IO_TryCatch.Exception();
            const FString ExText = Ex.IsEmpty()
                ? FString("(empty exception)")
                : FromV8String(I_Isolate, Ex).Get(FString("(non-string exception)"));
            LOG_ERROR("(FScripting) {}: {}", I_Label, ExText);
        }

        v8::Local<v8::Value> Stack = IO_TryCatch.StackTrace(I_Context).FromMaybe(v8::Local<v8::Value>());
        if (!Stack.IsEmpty() && Stack->IsString())
        {
            if (TOptional<FString> StackStr = FromV8String(I_Isolate, Stack); StackStr.HasValue())
                LOG_ERROR("(FScripting) {} stack:\n{}", I_Label, StackStr.GetValue());
        }
    }
}

namespace Visera
{
    // --- Implementation ---

    FScripting::FScripting(const FScriptingCreateInfo& I_CreateInfo, FGraphics* I_Graphics, FAssetHub* I_AssetHub, FAudio* I_Audio, FInput* I_Input, Bool I_RegisterScriptMainWindow)
        : Graphics(I_Graphics)
        , Input(I_Input)
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
        if (Input)
        { InputBindingState = MakeUnique<FJavaScriptInputBindingState>(Input); }
        TOptional<FWindowCreateInfo>* MainWindowSink = I_RegisterScriptMainWindow ? &ScriptMainWindow : nullptr;
        RegisterAllBindings(Graphics, I_Audio, Input, InputBindingState.Get(), MainWindowSink, *Context);

        const FPath EntryPath = I_CreateInfo.EntryScriptPath.GetRealPath();
        if (EntryPath.IsEmpty() || !LoadEntryScript(EntryPath))
            LOG_WARN("(FScripting) Entry script load/run failed: {}", I_CreateInfo.EntryScriptPath);
        else
        { RunOnInit(); }
    }

    TOptional<FWindowCreateInfo> FScripting::ResolveMainWindow() const
    {
        return ScriptMainWindow;
    }

    FScripting::~FScripting()
    {
        InputBindingState.Reset();
        Context.Reset();
        VM.Reset();
        Graphics = nullptr;
        Input    = nullptr;
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
            LogV8TryCatch(Isolate, ContextHandle, TryCatch, "OnTick(dt)");
    }

    void FScripting::RunOnInit()
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

        v8::MaybeLocal<v8::String> OnInitName = ToV8String(Isolate, "OnInit");
        if (OnInitName.IsEmpty())
            return;
        v8::Local<v8::Value> OnInitVal;
        if (!ContextHandle->Global()->Get(ContextHandle, OnInitName.ToLocalChecked()).ToLocal(&OnInitVal) || !OnInitVal->IsFunction())
            return;

        v8::Local<v8::Function> OnInitFn = OnInitVal.As<v8::Function>();
        v8::Local<v8::Value>    Receiver = ContextHandle->Global();
        v8::TryCatch            TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Result = OnInitFn->Call(ContextHandle, Receiver, 0, nullptr);
        if (Result.IsEmpty() && TryCatch.HasCaught())
            LogV8TryCatch(Isolate, ContextHandle, TryCatch, "OnInit()");
    }
}
