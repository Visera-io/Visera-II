module;
#include <Visera-Engine.hpp>
#include <Slang/slang.h>
#include <Slang/slang-com-ptr.h>
export module Visera.Engine.AssetHub.Shader.Slang;
#define VISERA_MODULE_NAME "Engine.AssetHub"
export import Visera.Runtime.RHI.SPIRV;
       import Visera.Core.Types.Path;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FSlangCompiler
    {
    public:
    	[[nodiscard]] static inline Bool
    	AddSearchPath(const FPath& I_Path);
        [[nodiscard]] inline TSharedPtr<RHI::FSPIRVShader>
    	Compile(const FPath& I_Path, FStringView I_EntryPoint);

    private:
    	static inline Slang::ComPtr<slang::IGlobalSession>
    	Context {nullptr}; //[Note] Currently, the global session type is not thread-safe. Applications that wish to compile on multiple threads will need to ensure that each concurrent thread compiles with a distinct global session.
    	static inline TSet<FString>
    	SearchPaths{};

    	struct FSession
        {
            Slang::ComPtr<slang::ISession> Handle;
            slang::TargetDesc              Description;
    		Slang::ComPtr<slang::IBlob>    CompiledCode;
        };
        TUniquePtr<FSession>               Session;

    public:
        FSlangCompiler();
    	~FSlangCompiler();

    private:
    	[[nodiscard]] Bool
    	CreateSession();
    	[[nodiscard]] TSharedPtr<RHI::FSPIRVShader>
    	Process(const FPath&  I_File, FStringView   I_EntryPoint);
    };

	Bool FSlangCompiler::
	AddSearchPath(const FPath& I_Path)
	{
		auto Path = I_Path.GetUTF8Path();
		if (!SearchPaths.contains(Path))
		{
			SearchPaths.emplace(std::move(Path));
			LOG_DEBUG("Added a new shader path: {}", I_Path);
			return True;
		}
		LOG_ERROR("Failed to add shader path: {}", I_Path);
		return False;
	}

    TSharedPtr<RHI::FSPIRVShader> FSlangCompiler::
    Compile(const FPath& I_Path, FStringView I_EntryPoint)
    {
		auto SPIRVShader = Process(I_Path.GetFileName(), I_EntryPoint);
    	if (!SPIRVShader)
    	{
    		LOG_ERROR("Failed to compile the shader \"{}\"!", I_Path);
    		return nullptr;
    	}
        return SPIRVShader;
    }

    FSlangCompiler::
    FSlangCompiler()
    {
    	if (Context == nullptr)
    	{
    		LOG_DEBUG("Creating a Slang thread global context.");

    		if (slang::createGlobalSession(Context.writeRef()) != SLANG_OK)
    		{ LOG_FATAL("Failed to create the Slang Context (a.k.a, Global Session)!"); }

    		if (!AddSearchPath(FPath{VISERA_ENGINE_DIR "/Assets/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add Engine shader path!"); }
    		if (!AddSearchPath(FPath{VISERA_APP_DIR "/Assets/Shader"}))
    		{ VISERA_ASSERT(False && "Failed to add App shader path!"); }
    	}

    	if (!CreateSession())
    	{ LOG_FATAL("Failed to create the Slang Session!"); }
    }

	FSlangCompiler::
	~FSlangCompiler()
    {
        Session->Handle.setNull();
    	Session.reset();
		slang::shutdown();
    }

	Bool FSlangCompiler::
	CreateSession()
    {
    	LOG_TRACE("Creating a new slang session.");
    	Session = MakeUnique<FSession>();

    	// Create Vulkan Compiler Session
    	Session->Description =
    	{
    		.format  = SLANG_SPIRV,
			.profile = Context->findProfile("glsl_450"),
		};

    	TArray<const char*> SlangSearchPaths;
		SlangSearchPaths.reserve(SearchPaths.size());
		for (const auto& Path : SearchPaths)
    	{ SlangSearchPaths.emplace_back(Path.data()); }

    	slang::SessionDesc SessionCreateInfo
		{
			.targets		 = &Session->Description,
			.targetCount	 = 1,
			.searchPaths	 = SlangSearchPaths.data(),
			.searchPathCount = static_cast<UInt32>(SearchPaths.size()),
		};

    	if (Context->createSession(SessionCreateInfo, Session->Handle.writeRef()) != SLANG_OK)
    	{
    		LOG_ERROR("Failed to create the Slang Session!");
    		return False;
    	}

    	return True;
    }

     TSharedPtr<RHI::FSPIRVShader> FSlangCompiler::
	 Process(const FPath& I_File, FStringView  I_EntryPoint)
	 {
    	VISERA_ASSERT(Context && Session);

    	LOG_TRACE("Compiling the {} (entry_point: {}).", I_File, I_EntryPoint);
	 	Slang::ComPtr<slang::IBlob>  Diagnostics;

	 	// Create Shader Module
	 	Slang::ComPtr<slang::IModule> ShaderModule
    	{
    		Session->Handle->loadModule(I_File.GetUTF8Path().data(),
    		Diagnostics.writeRef())
    	};
	 	if (Diagnostics)
	 	{
	 		LOG_ERROR("Failed to create the Shader Module: {}!",
	 			      static_cast<const char*>(Diagnostics->getBufferPointer()));
			return {};
	 	}

	 	// Create Shader Program
	 	Slang::ComPtr<slang::IEntryPoint> ShaderEntryPoint;
	 	if (ShaderModule->findEntryPointByName(
	 		I_EntryPoint.data(),
	 		ShaderEntryPoint.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to find the EntryPoint({}) from Shader({})!",
	 		          I_EntryPoint.data(), I_File);
			return {};
	 	}

	 	slang::IComponentType* const ShaderComponents[2]
    	{
	 		ShaderModule.get(),
	 		ShaderEntryPoint.get(),
	 	};

	 	Slang::ComPtr<slang::IComponentType> ShaderProgram {nullptr};
	 	if (Session->Handle->createCompositeComponentType(
	 		ShaderComponents, 2,
	 		ShaderProgram.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to create the Shader({}): {}!",
	 			      I_File, static_cast<const char*>(Diagnostics->getBufferPointer()));
	 		return {};
	 	}

	 	if (ShaderProgram->getEntryPointCode(
	 		0,
	 		0,
	 		Session->CompiledCode.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to obtain compiled code from {}: {}!",
	 		          I_File, static_cast<const char*>(Diagnostics->getBufferPointer()));
	 		return {};
	 	}

	 	// Reflect Shader
	 	slang::ProgramLayout* ShaderLayout = ShaderProgram->getLayout(0, Diagnostics.writeRef());
	 	if (Diagnostics)
		{
	 		LOG_ERROR("Failed to get reflection info from Shader({}): {}!",
	 			      I_File, static_cast<const char*>(Diagnostics->getBufferPointer()));
	 		return {};
		}

	 	auto* EntryPointRef = ShaderLayout->findEntryPointByName(I_EntryPoint.data());

		auto ShaderType { RHI::FSPIRVShader::EStage::Unknown };
	 	switch (EntryPointRef->getStage())
	 	{
	 		case SLANG_STAGE_VERTEX:	ShaderType = RHI::FSPIRVShader::EStage::Vertex;   break;
	 		case SLANG_STAGE_FRAGMENT:	ShaderType = RHI::FSPIRVShader::EStage::Fragment; break;
	 		default: LOG_ERROR("Unsupported Shader Stage!");
	 	}

		const FByte* Buffer = static_cast<const FByte*>(Session->CompiledCode->getBufferPointer());
		auto ShaderCode = TArray<FByte>(
			Buffer,
			Buffer + Session->CompiledCode->getBufferSize());
		Session->CompiledCode.setNull();

		return MakeShared<RHI::FSPIRVShader>(
			ShaderType,
			"main",
			ShaderCode);
	 }
}
