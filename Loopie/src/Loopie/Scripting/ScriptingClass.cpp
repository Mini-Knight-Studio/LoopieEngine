#include "ScriptingClass.h"
#include "Loopie/Scripting/ScriptingManager.h"

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
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	const std::string ScriptingClass::GetFullName() const{
		if (m_classNamespace.empty())
			return m_className;
		return m_classNamespace + "." + m_className;
	}
}