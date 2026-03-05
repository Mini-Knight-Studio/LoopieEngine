#pragma once
#include "Loopie/Scripting/ScriptingClass.h"
#include "Loopie/Core/UUID.h"

#include <string>
#include <memory>
#include <unordered_map>


struct _MonoDomain;
struct _MonoAssembly;
struct _MonoImage;
struct _MonoObject;
struct _MonoClass;
struct _MonoString;

namespace Loopie {
	struct ScriptingContext {
		_MonoDomain* RootDomain = nullptr;
		_MonoDomain* AppDomain = nullptr;

		_MonoAssembly* CoreAssembly = nullptr;
		_MonoImage* CoreImage = nullptr;

		_MonoAssembly* AppAssembly = nullptr;
		_MonoImage* AppImage = nullptr;

		_MonoAssembly* CompilerAssembly = nullptr;
		_MonoImage* CompilerImage = nullptr;

		std::string CoreAssemblyFilepath;
		std::string AppAssemblyFilepath;
		std::string CompilerAssemblyFilepath;

		std::shared_ptr<ScriptingClass> ComponentClass;

		std::unordered_map<std::string, std::shared_ptr<ScriptingClass>> ScriptingClasses;

		bool Dirty = false;

		bool EnableRecompile = true;
	};	


	class ScriptingManager {
	public:
		static void Init();
		static void Shutdown();

		static void Reload();

		static void LoadCoreAssembly();
		static void LoadAppAssembly();
		static void LoadCompilerAssembly();

		static void RuntimeStart();
		static void RuntimeStop();

		static void AttachCurrentThread();

		static void LoadScriptingClasses(_MonoImage* monoImage);
		static _MonoObject* InstantiateScriptingClass(_MonoClass* monoClass);
		static bool ExistsScriptingClass(const std::string& monoClassName);
		static std::shared_ptr<ScriptingClass> GetScriptingClass(const std::string& monoClassName);
		const std::unordered_map<std::string, std::shared_ptr<ScriptingClass>>& GetScriptingClasses() { return s_Data.ScriptingClasses; }
		static _MonoObject* CreateManagedEntity(const UUID& uuid);

		const static bool IsRunning() { return s_IsRunning; }
		static _MonoString* CreateString(const char* string);

		static ScriptingContext& GetContext() { return s_Data; }	

	public:
		static ScriptingContext s_Data;
	private:
		static _MonoAssembly* LoadAssembly(const char* path);
		static bool CompileGameAssembly();

	private:
		static bool s_IsRunning;
		static bool s_Initialized;
	};
	
}