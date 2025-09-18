module;
#include <Visera-Runtime.hpp>
#include <slang/slang.h>
#include <slang-com-ptr.h>
export module Visera.Runtime.AssetHub.Shader.Slang;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Shader.Importer;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_RUNTIME_API FSlangShaderImporter : public IShaderImporter
    {
    public:
        [[nodiscard]] auto
        Import(const FPath& I_Path) -> TArray<FByte> override;

    private:
        thread_local static inline Slang::ComPtr<slang::IGlobalSession>
    	Context {nullptr}; //[Note] Currently, the global session type is not thread-safe. Applications that wish to compile on multiple threads will need to ensure that each concurrent thread compiles with a distinct global session.

    	struct FSession
        {
            Slang::ComPtr<slang::ISession> Handle;
            slang::TargetDesc              Description;
            TArray<FString>                ShaderPaths;
        	Slang::ComPtr<slang::IBlob>    CompiledCode;
        };
        TUniquePtr<FSession>               Session;

    public:
        FSlangShaderImporter();

    private:
    	[[nodiscard]] Bool
    	CreateSession(const FPath& I_WorkingDirectory);
    	[[nodiscard]] Bool
    	Compile(const FPath& I_File, FStringView I_EntryPoint);
    };

    TArray<FByte> FSlangShaderImporter::
    Import(const FPath& I_Path)
    {
    	TArray<FByte> ShaderData;

    	FPath WorkingDirectory = I_Path.GetParent();

    	if (CreateSession(WorkingDirectory) &&
    		Compile(I_Path.GetFileName(), "VertexMain") &&
    		Compile(I_Path.GetFileName(), "FragmentMain"))
    	{

    	}
		else { LOG_ERROR("Failed to import the shader \"{}\"!", I_Path); }

        return ShaderData;
    }

    FSlangShaderImporter::
    FSlangShaderImporter()
    {
    	if (Context != nullptr) { return; }

		LOG_DEBUG("Creating a Slang thread global context.");

        if (slang::createGlobalSession(Context.writeRef()) != SLANG_OK)
        { LOG_FATAL("Failed to create the Slang Context (a.k.a, Global Session)!"); }
    }


	Bool FSlangShaderImporter::
	CreateSession(const FPath& I_WorkingDirectory)
    {
    	VISERA_ASSERT(!Session);
    	LOG_TRACE("Creating a new slange session.");
    	Session = MakeUnique<FSession>();

    	// Create Vulkan Compiler Session
    	Session->Description =
    	{
    		.format  = SLANG_SPIRV,
			.profile = Context->findProfile("glsl_450"),
		};
    	Session->ShaderPaths.emplace_back(I_WorkingDirectory.GetUTF8Path());

    	TArray<const char*> ShaderPaths(Session->ShaderPaths.size());
    	for (UInt32 Idx = 0; Idx < ShaderPaths.size(); ++Idx)
    	{
    		ShaderPaths[Idx] = Session->ShaderPaths[Idx].data();
    	}

    	slang::SessionDesc SessionCreateInfo
		{
			.targets		 = &Session->Description,
			.targetCount	 = 1,
			.searchPaths	 = ShaderPaths.data(),
			.searchPathCount = static_cast<UInt32>(Session->ShaderPaths.size()),
		};

    	if (Context->createSession(SessionCreateInfo, Session->Handle.writeRef()) != SLANG_OK)
    	{
    		LOG_ERROR("Failed to create the Slang Session!");
    		return False;
    	}

    	return True;
    }

     Bool FSlangShaderImporter::
	 Compile(const FPath& I_File, FStringView I_EntryPoint)
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
	 		LOG_ERROR("Failed to create the Shader Module: {}!", Diagnostics->getBufferPointer());
			return False;
	 	}

	 	// Create Shader Program
	 	Slang::ComPtr<slang::IEntryPoint> ShaderEntryPoint;
	 	if (ShaderModule->findEntryPointByName(
	 		I_EntryPoint.data(),
	 		ShaderEntryPoint.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to find the EntryPoint({}) from Shader({})!",
	 		          I_EntryPoint.data(), I_File);
			return False;
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
	 		return False;
	 	}

	 	if (ShaderProgram->getEntryPointCode(
	 		0,
	 		0,
	 		Session->CompiledCode.writeRef(),
	 		Diagnostics.writeRef()) != SLANG_OK)
	 	{
	 		LOG_ERROR("Failed to obtain compiled shader code from {}: {}!",
	 		          I_File, static_cast<const char*>(Diagnostics->getBufferPointer()));
	 		return False;
	 	}
		//
	 // 	// Reflect Shader
	 // 	slang::ProgramLayout* ShaderLayout = ShaderProgram->getLayout(0, Diagnostics.writeRef());
	 // 	if (Diagnostics)
		// {
	 // 		String Info = Text("Failed to get reflection info from Shader({}): {} -- throw(SRuntimeError)",
		// 					   _FileName.data(), RawString(Diagnostics->getBufferPointer()));
	 // 		VE_LOG_ERROR("{}", Info);
	 // 		throw SRuntimeError(Info);
		// }
		//
	 // 	auto* EntryPointRef = ShaderLayout->findEntryPointByName(_EntryPoint.data());
		//
	 // 	EShaderStage ShaderStage{};
	 // 	switch (EntryPointRef->getStage())
	 // 	{
	 // 	case SLANG_STAGE_VERTEX:	ShaderStage = EShaderStage::Vertex;   break;
	 // 	case SLANG_STAGE_FRAGMENT:	ShaderStage = EShaderStage::Fragment; break;
	 // 	default: VE_LOG_FATAL("Unsupported Shader PoolType!");
	 // 	}
		//
	 // 	auto RHIShader = RHI::CreateShader(ShaderStage,
	 // 					                   CompiledCode->getBufferPointer(),
	 // 					                   CompiledCode->getBufferSize());
		//
		return True;
	 }
}
