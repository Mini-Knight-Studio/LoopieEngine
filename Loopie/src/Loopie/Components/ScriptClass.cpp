#include "ScriptClass.h"

#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Math/MathTypes.h"

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

		m_gcHandle = mono_gchandle_new(m_instance, true);

		m_OnCreate = m_scriptingClass->GetMethod("OnCreate", 0);
		m_OnUpdate = m_scriptingClass->GetMethod("OnUpdate", 0);
		m_OnDestroy = m_scriptingClass->GetMethod("OnDestroy", 0);
		m_OnDrawGizmo = m_scriptingClass->GetMethod("OnDrawGizmo", 0);

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
		for (const ScriptField& field : m_scriptingClass->GetFields())
		{
			const std::string& name = field.Name;
			const ScriptFieldData& fieldData = m_scriptFields[name];

			if (field.Type == ScriptFieldType::String)
				SetRuntimeFieldString(name, fieldData.GetString());
			else if (field.Type == ScriptFieldType::Entity)
				SetRuntimeEntityField(name, fieldData.GetString());
			else
				SetFieldValueInternal(name, fieldData.GetBuffer());
		}
	}

	_MonoObject* ScriptClass::GetInstance() const
	{
		if (!m_gcHandle)
			return nullptr;

		return mono_gchandle_get_target(m_gcHandle);
	}

	bool ScriptClass::IsValid() const{
		return m_gcHandle != 0 && m_instance != nullptr && m_scriptingClass != nullptr;
	}

	void ScriptClass::DestroyInstance()
	{
		if (m_gcHandle)
		{
			mono_gchandle_free(m_gcHandle);
			m_gcHandle = 0;
		}

		m_instance = nullptr;
		m_OnCreate = nullptr;
		m_OnUpdate = nullptr;
		m_OnDestroy = nullptr;
		m_OnDrawGizmo = nullptr;
	}

	void ScriptClass::InvokeOnCreate()
	{
		if (m_OnCreate) {
			m_scriptingClass->InvokeMethod(m_instance, m_OnCreate);
		}
	}

	void ScriptClass::InvokeOnUpdate()
	{
		if (m_OnUpdate) {
			m_scriptingClass->InvokeMethod(m_instance, m_OnUpdate);
		}
	}

	void ScriptClass::InvokeOnDestroy()
	{
		if (m_OnDestroy) {
			m_scriptingClass->InvokeMethod(m_instance, m_OnDestroy);
		}
	}

	void ScriptClass::InvokeOnDrawGizmo()
	{
		if (m_OnDrawGizmo) {
			m_scriptingClass->InvokeMethod(m_instance, m_OnDrawGizmo);
		}
	}

	void ScriptClass::SetClass(const std::string& fullName)
	{
		m_className = fullName;
	}

	std::string ScriptClass::GetRuntimeFieldString(const std::string& fieldName)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return {};
		MonoString* monoStr = nullptr;
		mono_field_get_value(m_instance, field->ClassField, &monoStr);

		if (!monoStr)
			return {};

		char* utf8 = mono_string_to_utf8(monoStr);
		std::string result = utf8;
		mono_free(utf8);

		return result;
	}

	void ScriptClass::SetRuntimeFieldString(const std::string& fieldName, const std::string& value)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return;

		MonoString* monoStr = mono_string_new(mono_domain_get(), value.c_str());
		mono_field_set_value(m_instance, field->ClassField, monoStr);
	}

	std::string ScriptClass::GetRuntimeEntityField(const std::string& fieldName)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return "";

		MonoObject* obj = nullptr;
		mono_field_get_value(m_instance, field->ClassField, &obj);
		if (!obj) 
			return "";

		MonoClass* klass = mono_object_get_class(obj);
		MonoProperty* prop = mono_class_get_property_from_name(klass, "ID");

		MonoObject* idStrObj = mono_property_get_value(prop, obj, nullptr, nullptr);

		char* utf8 = mono_string_to_utf8((MonoString*)idStrObj);
		std::string uuid = utf8;
		mono_free(utf8);

		return uuid;
	}

	void ScriptClass::SetRuntimeEntityField(const std::string& fieldName, const std::string& value)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return;

		UUID uuid = UUID(value);
		MonoObject* entityObj = ScriptingManager::CreateManagedEntity(uuid);
		mono_field_set_value(m_instance, field->ClassField, entityObj);
	}

	std::string ScriptClass::GetFieldString(const std::string& fieldName) const
	{
		auto it = m_scriptFields.find(fieldName);
		if (it == m_scriptFields.end())
			return "";

		return it->second.GetString();
	}

	void ScriptClass::SetFieldString(const std::string& fieldName, const std::string& value)
	{
		m_scriptFields[fieldName].SetString(value);
	}

	bool ScriptClass::GetFieldValueInternal(const std::string& fieldName, void* buffer)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return false;
		mono_field_get_value(m_instance, field->ClassField, buffer);
		return true;
	}

	bool ScriptClass::SetFieldValueInternal(const std::string& fieldName, const void* value)
	{
		const ScriptField* field = m_scriptingClass->FindField(fieldName);
		if (!field)
			return false;
		mono_field_set_value(m_instance, field->ClassField, (void*)value);
		return true;
	}

	JsonNode ScriptClass::Serialize(JsonNode& parent) const
	{
		JsonNode scriptObj = parent.CreateObjectField("script");

		scriptObj.CreateField("class_id", GetClassName());
		JsonNode node = scriptObj.CreateObjectField("fields");

		if (!m_scriptingClass)
			return scriptObj;

		const auto& fields = m_scriptingClass->GetFields();
		for (const ScriptField& field : fields)
		{
			const std::string& name = field.Name;
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
			case ScriptFieldType::Entity:
				node.CreateField(name, GetFieldString(name));
				break;
				// more types ...
			case ScriptFieldType::Vector2: {
				JsonNode vectorNode = node.CreateObjectField(name);
				vec2 vector = GetFieldValue<vec2>(name);
				vectorNode.CreateField<float>("x", vector.x);
				vectorNode.CreateField<float>("y", vector.y);
				break;
			}

			case ScriptFieldType::Vector3: {
				JsonNode vectorNode = node.CreateObjectField(name);
				vec3 vector = GetFieldValue<vec3>(name);
				vectorNode.CreateField<float>("x", vector.x);
				vectorNode.CreateField<float>("y", vector.y);
				vectorNode.CreateField<float>("z", vector.z);
				break;
			}

			case ScriptFieldType::Vector4:{
				JsonNode vectorNode = node.CreateObjectField(name);
				vec4 vector = GetFieldValue<vec4>(name);
				vectorNode.CreateField<float>("x", vector.x);
				vectorNode.CreateField<float>("y", vector.y);
				vectorNode.CreateField<float>("z", vector.z);
				vectorNode.CreateField<float>("w", vector.w);
				break;
			}
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

		if (!m_scriptingClass)
			return;

		JsonNode node = data.Child("fields");
		const auto& fields = m_scriptingClass->GetFields();
		for (const ScriptField& scriptField : fields)
		{
			const std::string& name = scriptField.Name;
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
			case ScriptFieldType::Entity:
				fieldData.SetString(node.GetValue<std::string>(name, "").Result);
				break;
				// more types...

			case ScriptFieldType::Vector2: {

				vec2 vector = vec2(1);
				JsonNode vectorNode = node.Child(name);
				vector.x = vectorNode.GetValue<float>("x").Result;
				vector.y = vectorNode.GetValue<float>("y").Result;
				fieldData.SetValue<vec2>(vector);
				break;
			}

			case ScriptFieldType::Vector3: {
				vec3 vector = vec3(1);
				JsonNode vectorNode = node.Child(name);
				vector.x = vectorNode.GetValue<float>("x").Result;
				vector.y = vectorNode.GetValue<float>("y").Result;
				vector.z = vectorNode.GetValue<float>("z").Result;
				fieldData.SetValue<vec3>(vector);
				break;
			}

			case ScriptFieldType::Vector4: {
				vec4 vector = vec4(1);
				JsonNode vectorNode = node.Child(name);
				vector.x = vectorNode.GetValue<float>("x").Result;
				vector.y = vectorNode.GetValue<float>("y").Result;
				vector.z = vectorNode.GetValue<float>("z").Result;
				vector.w = vectorNode.GetValue<float>("w").Result;
				fieldData.SetValue<vec4>(vector);
				break;
			}
			default:
				break;

			}
		}
	}

}