/** JavaScript VM: V8 isolate lifecycle, platform, and script execution (compile + run). */
module;
#include <v8.h>
#include <libplatform/libplatform.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting.VM;
#define VISERA_MODULE_NAME "Runtime.Scripting.VM"
export import Visera.Runtime.Scripting.Utils;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.Log;

export namespace Visera
{
    /** Wraps v8::Isolate: init/destroy, platform, array buffer allocator. */
    class VISERA_RUNTIME_API FJavaScriptVM
    {
    public:
        FJavaScriptVM();
        ~FJavaScriptVM();

        FJavaScriptVM(const FJavaScriptVM&)            = delete;
        FJavaScriptVM& operator=(const FJavaScriptVM&)  = delete;
        FJavaScriptVM(FJavaScriptVM&&)                  = delete;
        FJavaScriptVM& operator=(FJavaScriptVM&&)       = delete;

        [[nodiscard]] v8::Isolate*
        GetIsolate() const { return Isolate; }

    private:
        static void EnsureV8Platform();
        static std::unique_ptr<v8::Platform> Platform;
        static Int32 PlatformRefCount;

        v8::Isolate*               Isolate { nullptr };
        v8::ArrayBuffer::Allocator* ArrayBufferAllocator { nullptr };
    };

    /** Execute script in the given context. Returns NullOpt on success; on failure logs and returns an error message. */
    [[nodiscard]] TOptional<FString>
    ExecuteScript(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context, FStringView I_Source, FStringView I_FileName = "script");

    // --- Implementation ---

    std::unique_ptr<v8::Platform> FJavaScriptVM::Platform;
    Int32 FJavaScriptVM::PlatformRefCount = 0;

    void FJavaScriptVM::EnsureV8Platform()
    {
        if (PlatformRefCount++ == 0)
        {
            Platform = v8::platform::NewDefaultPlatform();
            v8::V8::InitializePlatform(Platform.get());
            v8::V8::Initialize();
        }
    }

    FJavaScriptVM::FJavaScriptVM()
    {
        EnsureV8Platform();
        ArrayBufferAllocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        v8::Isolate::CreateParams CreateParams;
        CreateParams.array_buffer_allocator = ArrayBufferAllocator;
        Isolate = v8::Isolate::New(CreateParams);
        if (!Isolate)
        {
            delete ArrayBufferAllocator;
            ArrayBufferAllocator = nullptr;
            if (--PlatformRefCount == 0)
            {
                v8::V8::Dispose();
                v8::V8::DisposePlatform();
                Platform.reset();
            }
            LOG_FATAL("(FJavaScriptVM) Isolate::New failed.");
        }
    }

    FJavaScriptVM::~FJavaScriptVM()
    {
        if (Isolate)
        {
            Isolate->Dispose();
            Isolate = nullptr;
        }
        if (ArrayBufferAllocator)
        {
            delete ArrayBufferAllocator;
            ArrayBufferAllocator = nullptr;
        }
        if (--PlatformRefCount == 0)
        {
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            Platform.reset();
        }
    }

    TOptional<FString> ExecuteScript(v8::Isolate* I_Isolate, v8::Local<v8::Context> I_Context, FStringView I_Source, FStringView I_FileName)
    {
        if (!I_Isolate || I_Context.IsEmpty())
            return TOptional<FString>(FString("(invalid isolate or context)"));
        if (I_Source.IsEmpty())
            return TOptional<FString>(FString("(empty script)"));
        v8::Isolate::Scope IsolateScope(I_Isolate);
        v8::HandleScope HandleScope(I_Isolate);
        v8::Context::Scope ContextScope(I_Context);

        v8::TryCatch TryCatch(I_Isolate);
        v8::MaybeLocal<v8::String> SourceResult = ToV8String(I_Isolate, I_Source);
        if (SourceResult.IsEmpty())
            return TOptional<FString>(FString("(invalid script encoding)"));
        v8::Local<v8::String> Source = SourceResult.ToLocalChecked();

        v8::MaybeLocal<v8::String> NameResult = ToV8String(I_Isolate, I_FileName);
        v8::Local<v8::String> Name = NameResult.IsEmpty() ? Source : NameResult.ToLocalChecked();

        v8::ScriptOrigin Origin(Name, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, false);
        v8::MaybeLocal<v8::Script> ScriptResult = v8::Script::Compile(I_Context, Source, &Origin);
        if (ScriptResult.IsEmpty())
        {
            FString Msg = FString("(compile error)");
            if (TryCatch.HasCaught() && !TryCatch.Message().IsEmpty())
            {
                v8::Local<v8::Message> Message = TryCatch.Message();
                Msg = FromV8String(I_Isolate, Message->Get()).Get(FString("(compile error)"));
                LOG_ERROR("(Script) Compile error: {}", Msg);
                v8::Local<v8::Value> Stack = TryCatch.StackTrace(I_Context).FromMaybe(v8::Local<v8::Value>());
                if (!Stack.IsEmpty() && Stack->IsString())
                {
                    TOptional<FString> StackStr = FromV8String(I_Isolate, Stack);
                    if (StackStr.HasValue())
                        LOG_ERROR("(Script) Stack: {}", StackStr.GetValue());
                }
            }
            return TOptional<FString>(std::move(Msg));
        }

        v8::Local<v8::Script> Script = ScriptResult.ToLocalChecked();
        v8::MaybeLocal<v8::Value> RunResult = Script->Run(I_Context);
        if (RunResult.IsEmpty())
        {
            FString Msg = FString("(runtime error)");
            if (TryCatch.HasCaught() && !TryCatch.Message().IsEmpty())
            {
                v8::Local<v8::Message> Message = TryCatch.Message();
                Msg = FromV8String(I_Isolate, Message->Get()).Get(FString("(runtime error)"));
                LOG_ERROR("(Script) Runtime error: {}", Msg);
                v8::Local<v8::Value> Stack = TryCatch.StackTrace(I_Context).FromMaybe(v8::Local<v8::Value>());
                if (!Stack.IsEmpty() && Stack->IsString())
                {
                    TOptional<FString> StackStr = FromV8String(I_Isolate, Stack);
                    if (StackStr.HasValue())
                        LOG_ERROR("(Script) Stack: {}", StackStr.GetValue());
                }
            }
            return TOptional<FString>(std::move(Msg));
        }

        (void)RunResult.ToLocalChecked();
        return NullOpt;
    }
}
