#include "ScriptingManager.h"

#include "Loopie/Scripting/ScriptGlue.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Events/Event.h"

#include <mono/metadata/mono-config.h>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>

namespace Loopie {
	ScriptingContext ScriptingManager::s_Data;
	bool ScriptingManager::s_IsRunning = false;
	bool ScriptingManager::s_Initialized = false;

	static ScriptFieldType MonoTypeToScriptFieldType(_MonoType* monoType)
	{
		std::string typeName = mono_type_get_name(monoType);

		auto it = s_ScriptFieldTypeMap.find(typeName);
		if (it == s_ScriptFieldTypeMap.end())
		{
			return ScriptFieldType::None;
		}

		return it->second;
	}

	MonoString* ScriptingManager::CreateString(const char* string)
	{
		return mono_string_new(s_Data.AppDomain, string);
	}
	void ScriptingManager::Init() {
		mono_set_assemblies_path("mono/lib/mono/4.5");
		mono_set_dirs("mono/lib", "mono/etc");
		mono_config_parse(NULL);

		s_Data.RootDomain = mono_jit_init("LoopieScriptingRuntime");
		if (!s_Data.RootDomain) {
			Log::Error("Failed to initialize Mono");
			return;
		}

		s_Data.AppDomain = mono_domain_create_appdomain("LoopieAppDomain", nullptr);
		mono_domain_set(s_Data.AppDomain, true);

		ScriptGlue::RegisterFunctions();

		LoadCoreAssembly();

		if (s_Data.EnableRecompile) {
			LoadCompilerAssembly();
			CompileGameAssembly();
		}

		LoadAppAssembly(); 

		if (s_Data.AppImage != nullptr)
			LoadScriptingClasses(s_Data.AppImage);
		else
			s_Data.ScriptingClasses.clear();

		ScriptGlue::RegisterComponents();

		s_Data.ComponentClass = std::make_shared<ScriptingClass>("Loopie", "Component", true);

		s_Initialized = true;

	}

	void ScriptingManager::Shutdown() {
		if (!s_Initialized) return;

		// Switch to root
		mono_domain_set(s_Data.RootDomain, false);

		if (s_Data.AppDomain) {
			mono_domain_unload(s_Data.AppDomain);
			s_Data.AppDomain = nullptr;
		}

		if (s_Data.RootDomain) {
			mono_jit_cleanup(s_Data.RootDomain);
			s_Data.RootDomain = nullptr;
		}
		s_Initialized = false;
	}

	void ScriptingManager::Reload() {
		if (!s_Initialized) return;

		Log::Info("Reloading scripting domain");
		if (s_IsRunning) RuntimeStop();

		if (!mono_thread_current())
			mono_thread_attach(s_Data.RootDomain);

		mono_domain_set(s_Data.RootDomain, false);

		if (s_Data.AppDomain) {
			mono_domain_unload(s_Data.AppDomain);
			s_Data.AppDomain = nullptr;
		}

		s_Data.AppDomain = mono_domain_create_appdomain("LoopieAppDomain", nullptr);
		mono_domain_set(s_Data.AppDomain, true);

		LoadCoreAssembly();

		if (s_Data.EnableRecompile) {
			LoadCompilerAssembly();

			if (!CompileGameAssembly()) {
				Log::Error("Script compilation failed. Aborting reload.");
				return;
			}
		}

		LoadAppAssembly();
		if(s_Data.AppImage!=nullptr)
			LoadScriptingClasses(s_Data.AppImage);
		else
			s_Data.ScriptingClasses.clear();

		ScriptGlue::RegisterComponents();

		s_Data.ComponentClass = std::make_shared<ScriptingClass>("Loopie", "Component", true);

		s_Data.Dirty = false;
	}


	void ScriptingManager::LoadCoreAssembly()
	{
		s_Data.CoreAssembly = LoadAssembly(s_Data.CoreAssemblyFilepath.c_str());
		if(s_Data.CoreAssembly)
			s_Data.CoreImage = mono_assembly_get_image(s_Data.CoreAssembly);
	}

	void ScriptingManager::LoadAppAssembly()
	{
		s_Data.AppAssembly = LoadAssembly(s_Data.AppAssemblyFilepath.c_str());
		if(s_Data.AppAssembly)
			s_Data.AppImage = mono_assembly_get_image(s_Data.AppAssembly);
	}

	void ScriptingManager::LoadCompilerAssembly()
	{
		s_Data.CompilerAssembly = LoadAssembly(s_Data.CompilerAssemblyFilepath.c_str());
		if (s_Data.CompilerAssembly)
			s_Data.CompilerImage = mono_assembly_get_image(s_Data.CompilerAssembly);
	}

	void ScriptingManager::RuntimeStart()
	{
		s_IsRunning = true;
		Application::GetInstance().m_notifier.Notify(EngineNotification::OnRuntimeStart);
	}

	void ScriptingManager::RuntimeStop()
	{
		s_IsRunning = false;
		Application::GetInstance().m_notifier.Notify(EngineNotification::OnRuntimeStop);
	}

	void ScriptingManager::AttachCurrentThread()
	{
		if (!mono_thread_current())
			mono_thread_attach(mono_domain_get());
	}

	void ScriptingManager::LoadScriptingClasses(_MonoImage* monoImage)
	{
		s_Data.ScriptingClasses.clear();

		//// Load Scripting Classes
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(monoImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* component = mono_class_from_name(s_Data.CoreImage, "Loopie", "Component");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(monoImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(monoImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			MonoClass* monoClass = mono_class_from_name(monoImage, nameSpace, className);

			if (!monoClass || monoClass == component)
				continue;

			bool isComponent = mono_class_is_subclass_of(monoClass, component, false);
			if (!isComponent)
				continue;

			std::shared_ptr<ScriptingClass> scriptClass = std::make_shared<ScriptingClass>(nameSpace, className, false);
			s_Data.ScriptingClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);
			Log::Warn("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_get_name(type);
					ScriptFieldType fieldType = MonoTypeToScriptFieldType(type);

					MonoClass* fieldClass = mono_class_from_mono_type(type);

					if (fieldClass)
					{
						isComponent = mono_class_is_subclass_of(fieldClass, component, false);
						if (isComponent)
							fieldType = ScriptFieldType::Component;
					}

					scriptClass->GetFields()[fieldName] = { fieldType, fieldName, field };
					Log::Warn("   {0} -> {1}", fieldName, typeName);
				}
			}
		}
	}

	_MonoObject* ScriptingManager::InstantiateScriptingClass(_MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data.AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	bool ScriptingManager::ExistsScriptingClass(const std::string& monoClassName)
	{
		return s_Data.ScriptingClasses.find(monoClassName) != s_Data.ScriptingClasses.end();
	}

	std::shared_ptr<ScriptingClass> ScriptingManager::GetScriptingClass(const std::string& monoClassName)
	{
		if (!ExistsScriptingClass(monoClassName))
			return nullptr;

		return s_Data.ScriptingClasses.at(monoClassName);
	}

	_MonoObject* ScriptingManager::CreateManagedEntity(const UUID& uuid)
	{
		MonoClass* entityClass =
			mono_class_from_name(s_Data.CoreImage, "Loopie", "Entity");

		if (!entityClass)
		{
			Log::Error("Failed to find Loopie.Entity class");
			return nullptr;
		}

		MonoObject* entityObject =
			mono_object_new(s_Data.AppDomain, entityClass);

		MonoMethod* ctor =
			mono_class_get_method_from_name(entityClass, ".ctor", 1);

		if (!ctor)
		{
			Log::Error("Failed to find Entity constructor");
			return nullptr;
		}

		std::string idStr = uuid.Get();
		MonoString* monoID =
			mono_string_new(s_Data.AppDomain, idStr.c_str());

		void* args[1] = { monoID };

		MonoObject* exception = nullptr;
		mono_runtime_invoke(ctor, entityObject, args, &exception);

		if (exception)
		{
			mono_print_unhandled_exception(exception);
			return nullptr;
		}

		return entityObject;
	}

	_MonoAssembly* ScriptingManager::LoadAssembly(const char* path)
	{
		_MonoAssembly* assembly = mono_domain_assembly_open(s_Data.AppDomain, path);
		if (!assembly)
			Log::Error("Failed to load assembly: {}", path);
		return assembly;
	}

	bool ScriptingManager::CompileGameAssembly()
	{
		MonoClass* compilerClass = mono_class_from_name(s_Data.CompilerImage, "Loopie", "ScriptCompiler"); 
		MonoMethod* compileMethod = mono_class_get_method_from_name(compilerClass, "Compile", 4); 
		MonoString* sourceDir = mono_string_new(s_Data.AppDomain, Application::GetInstance().m_activeProject.GetAssetsPath().string().c_str());
		MonoString* outputDll = mono_string_new(s_Data.AppDomain, s_Data.AppAssemblyFilepath.c_str());
		MonoString* coreDll = mono_string_new(s_Data.AppDomain, s_Data.CoreAssemblyFilepath.c_str());
		MonoString* errorString = nullptr; void* args[4] = { sourceDir, outputDll, coreDll, &errorString }; 
		MonoObject* exception = nullptr; MonoObject* result = mono_runtime_invoke(compileMethod, nullptr, args, &exception); 
		if (exception) 
		{ 
			mono_print_unhandled_exception(exception); 
			return false; 
		} 
		bool success = *(bool*)mono_object_unbox(result); 
		if (!success) 
		{ 
			char* err = mono_string_to_utf8(errorString); 
			std::string errorStr(err);
			Log::Error("Script compile error:\n{}", err); 
			if (errorStr == "No C# files found in source directory.") {
				s_Data.ScriptingClasses.clear();
			}
			mono_free(err); 
		} 
		return success;
	}
}