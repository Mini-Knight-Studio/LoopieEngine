#include "ScriptingClass.h"
#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Core/Log.h"

#include "mono/metadata/object.h"

namespace Loopie {
	ScriptingClass::ScriptingClass(const std::string& classNamespace, const std::string& className, bool isCore) : m_classNamespace(classNamespace), m_className(className)
	{
		m_monoClass = mono_class_from_name(isCore ? ScriptingManager::s_Data.CoreImage : ScriptingManager::s_Data.AppImage, classNamespace.c_str(), className.c_str());
	}

	_MonoObject* ScriptingClass::Instantiate()
	{
		return ScriptingManager::InstantiateScriptingClass(m_monoClass);
	}

	_MonoMethod* ScriptingClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_monoClass, name.c_str(), parameterCount);
	}

	_MonoObject* ScriptingClass::InvokeMethod(_MonoObject* instance, _MonoMethod* method, void** params)
	{
		if (!instance)
		{
			Log::Error("InvokeMethod: instance is null");
			return nullptr;
		}

		if (!method)
		{
			Log::Error("InvokeMethod: method is null");
			return nullptr;
		}

		ScriptingManager::AttachCurrentThread();

		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);

		if (exception)
		{
			mono_print_unhandled_exception(exception);
			Log::Error("Exception thrown in script method invocation");
		}

		return result;
	}

	const std::string ScriptingClass::GetFullName() const{
		if (m_classNamespace.empty())
			return m_className;
		return m_classNamespace + "." + m_className;
	}
	const ScriptField* ScriptingClass::FindField(const std::string& name) const
	{
		auto it = m_index.find(name);
		if (it == m_index.end())
			return nullptr;
		return &m_fields[it->second];
	}
}