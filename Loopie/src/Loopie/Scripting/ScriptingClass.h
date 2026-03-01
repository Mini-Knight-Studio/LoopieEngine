#pragma once
#include <map>
#include <unordered_map>
#include <string> 
#include <memory> 

struct _MonoClass;
struct _MonoObject;
struct _MonoMethod;
struct _MonoClassField;
struct _MonoType;

namespace Loopie {


	enum class ScriptFieldType
	{
		None,
		Float,
		Double,
		Bool,
		Char,
		Byte,
		Short,
		Int,
		Long,
		UByte,
		UShort,
		UInt,
		ULong,
		String
	};

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "System.String",  ScriptFieldType::String }
	};

	

	struct ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::Byte;
		std::string Name;

		_MonoClassField* ClassField = nullptr;
	};

	class ScriptingClass {
	public:
		ScriptingClass() = default;
		ScriptingClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		_MonoObject* Instantiate();
		_MonoMethod* GetMethod(const std::string& name, int parameterCount);
		_MonoObject* InvokeMethod(_MonoObject* instance, _MonoMethod* method, void** params = nullptr);
		const std::map<std::string, ScriptField>& GetFields() const { return m_fields; }
		std::map<std::string, ScriptField>& GetFields() { return m_fields; }

		const std::string& GetClassName() const { return m_className; }
		const std::string& GetClassNamespace() const { return m_classNamespace; }
		const std::string GetFullName() const { return m_classNamespace + "." + m_className; }

		_MonoClass* GetMonoClass() const { return m_monoClass; }

		bool IsSameType(std::shared_ptr<ScriptingClass> scriptingClass) const {
			return GetFullName() == scriptingClass->GetFullName();
		}

		bool IsSameType(const std::string& type) const {
			return GetFullName() == type;
		}

	private:
		std::string m_classNamespace;
		std::string m_className;
		_MonoClass* m_monoClass = nullptr;

		std::map<std::string, ScriptField> m_fields;
	};
	
}