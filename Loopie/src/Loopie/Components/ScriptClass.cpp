#include "ScriptClass.h"

#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Log.h"

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <mono/jit/jit.h>

namespace Loopie
{

	ScriptClass::ScriptClass(const std::string& className) : m_className(className)
	{
		m_scriptingClass = ScriptingManager::GetScriptingClass(m_className);
	}

	ScriptClass::~ScriptClass()
	{
		DestroyInstance();
	}

	void ScriptClass::SetUp()
	{
		m_scriptingClass = ScriptingManager::GetScriptingClass(m_className);
		m_instance = m_scriptingClass->Instantiate();

		m_OnCreate = m_scriptingClass->GetMethod("OnCreate", 0);
		m_OnUpdate = m_scriptingClass->GetMethod("OnUpdate", 0);

		MonoProperty* entityProperty =
			mono_class_get_property_from_name(ScriptingManager::s_Data.ComponentClass->GetMonoClass() , "entity");

		MonoProperty* idProperty =
			mono_class_get_property_from_name(ScriptingManager::s_Data.ComponentClass->GetMonoClass(), "ID");

		MonoObject* entityInstance = ScriptingManager::CreateManagedEntity(GetOwner()->GetUUID());
		MonoObject* componentInstance = ScriptingManager::CreateManagedEntity(GetUUID());

		void* args[1] = { nullptr };
		args[0] = entityInstance;
		mono_property_set_value(entityProperty, m_instance, args, nullptr);

		args[0] = componentInstance;
		mono_property_set_value(idProperty, m_instance, args, nullptr);

		// Restore fields
		for (const auto& [name, field] : m_scriptingClass->GetFields())
		{
			const ScriptFieldData& fieldData = m_scriptFields[name];

			if (field.Type == ScriptFieldType::String)
				SetRuntimeFieldString(name, fieldData.GetString());
			else
				SetFieldValueInternal(name, fieldData.GetBuffer());
		}
	}

	void ScriptClass::DestroyInstance()
	{
		m_instance = nullptr;
		m_OnCreate = nullptr;
		m_OnUpdate = nullptr;
	}

	void ScriptClass::InvokeOnCreate()
	{
		if (m_OnCreate)
			m_scriptingClass->InvokeMethod(m_instance, m_OnCreate);
	}

	void ScriptClass::InvokeOnUpdate()
	{
		if (m_OnUpdate)
			m_scriptingClass->InvokeMethod(m_instance, m_OnUpdate);
	}

	void ScriptClass::SetClass(const std::string& fullName)
	{
		m_className = fullName;
	}

	std::string ScriptClass::GetRuntimeFieldString(const std::string& name)
	{
		const auto& fields = m_scriptingClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return {};

		const ScriptField& field = it->second;

		MonoString* monoStr = nullptr;
		mono_field_get_value(m_instance, field.ClassField, &monoStr);

		if (!monoStr)
			return {};

		char* utf8 = mono_string_to_utf8(monoStr);
		std::string result = utf8;
		mono_free(utf8);

		return result;
	}

	void ScriptClass::SetRuntimeFieldString(const std::string& name, const std::string& value)
	{
		const auto& fields = m_scriptingClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return;

		const ScriptField& field = it->second;

		MonoString* monoStr = mono_string_new(mono_domain_get(), value.c_str());
		mono_field_set_value(m_instance, field.ClassField, monoStr);
	}

	std::string ScriptClass::GetFieldString(const std::string& name) const
	{
		auto it = m_scriptFields.find(name);
		if (it == m_scriptFields.end())
			return "";

		return it->second.GetString();
	}

	void ScriptClass::SetFieldString(const std::string& name, const std::string& value)
	{
		m_scriptFields[name].SetString(value);
	}

	bool ScriptClass::GetFieldValueInternal(const std::string& fieldName, void* buffer)
	{
		const auto& fields = m_scriptingClass->GetFields();
		auto it = fields.find(fieldName);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptClass::SetFieldValueInternal(const std::string& fieldName, const void* value)
	{
		const auto& fields = m_scriptingClass->GetFields();
		auto it = fields.find(fieldName);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_instance, field.ClassField, (void*)value);
		return true;
	}

	JsonNode ScriptClass::Serialize(JsonNode& parent) const
	{
		JsonNode scriptObj = parent.CreateObjectField("script");

		scriptObj.CreateField("class_id", GetClassName());

		const auto& fields = m_scriptingClass->GetFields();

		JsonNode node = scriptObj.CreateObjectField("fields");

		for (const auto& [name, field] : fields)
		{
			switch (field.Type)
			{
			case ScriptFieldType::Float:
				node.CreateField(name, GetFieldValue<float>(name));
				break;
			case ScriptFieldType::Double:
				node.CreateField(name, GetFieldValue<double>(name));
				break;
			case ScriptFieldType::Bool:
				node.CreateField(name, GetFieldValue<bool>(name));
				break;
			case ScriptFieldType::Char:
				node.CreateField(name, (int)GetFieldValue<char>(name));
				break;

			case ScriptFieldType::Byte:
				node.CreateField(name, (int)GetFieldValue<uint8_t>(name));
				break;

			case ScriptFieldType::Short:
				node.CreateField(name, GetFieldValue<int16_t>(name));
				break;
			case ScriptFieldType::Int:
				node.CreateField(name, GetFieldValue<int32_t>(name));
				break;
			case ScriptFieldType::Long:
				node.CreateField(name, (int64_t)GetFieldValue<int64_t>(name));
				break;

			case ScriptFieldType::UShort:
				node.CreateField(name, GetFieldValue<uint16_t>(name));
				break;
			case ScriptFieldType::UInt:
				node.CreateField(name, GetFieldValue<uint32_t>(name));
				break;
			case ScriptFieldType::ULong:
				node.CreateField(name, (uint64_t)GetFieldValue<uint64_t>(name));
				break;
			case ScriptFieldType::String:
				node.CreateField(name, GetFieldString(name));
				break;
				// more types ...

			default:
				break;
			}
		}

		return scriptObj;
	}

	void ScriptClass::Deserialize(const JsonNode& data)
	{

		std::string classID = data.GetValue<std::string>("class_id", "").Result;
		m_className = classID;
		m_scriptingClass = ScriptingManager::GetScriptingClass(m_className);


		JsonNode node = data.Child("fields");
		const auto& fields = m_scriptingClass->GetFields();
		for (const auto& [name, scriptField] : fields)
		{
			if (!node.HasKey(name))
				continue;

			ScriptFieldData& fieldData = m_scriptFields[name];

			switch (scriptField.Type)
			{
			case ScriptFieldType::Float:
				fieldData.SetValue<float>(node.GetValue<float>(name, 0.f).Result);
				break;
			case ScriptFieldType::Double:
				fieldData.SetValue<double>(node.GetValue<double>(name, 0.0).Result);
				break;
			case ScriptFieldType::Bool:
				fieldData.SetValue<bool>(node.GetValue<bool>(name, false).Result);
				break;
			case ScriptFieldType::Char:
				fieldData.SetValue<char>((char)node.GetValue<int>(name, 0).Result);
				break;

			case ScriptFieldType::Byte:
				fieldData.SetValue<uint8_t>((uint8_t)node.GetValue<int>(name, 0).Result);
				break;

			case ScriptFieldType::Short:
				fieldData.SetValue<int16_t>((int16_t)node.GetValue<int>(name, 0).Result);
				break;
			case ScriptFieldType::Int:
				fieldData.SetValue<int32_t>(node.GetValue<int32_t>(name, 0).Result);
				break;
			case ScriptFieldType::Long:
				fieldData.SetValue<int64_t>(node.GetValue<int64_t>(name, 0).Result);
				break;

			case ScriptFieldType::UShort:
				fieldData.SetValue<uint16_t>((uint16_t)node.GetValue<int>(name, 0).Result);
				break;
			case ScriptFieldType::UInt:
				fieldData.SetValue<uint32_t>(node.GetValue<uint32_t>(name, 0).Result);
				break;
			case ScriptFieldType::ULong:
				fieldData.SetValue<uint64_t>(node.GetValue<uint64_t>(name, 0).Result);
				break;
			case ScriptFieldType::String:
				fieldData.SetString(node.GetValue<std::string>(name, "").Result);
				break;
				// more types...

			default:
				break;

			}
		}
	}

}