#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Scripting/ScriptingClass.h"

#include <memory>
#include <string>

struct _MonoObject;
struct _MonoClass;
struct _MonoMethod;
struct _MonoString;

namespace Loopie {

	struct ScriptFieldData
	{
		ScriptField Field;

		ScriptFieldData()
		{
			memset(m_buffer, 0, sizeof(m_buffer));
		}

		template<typename T>
		T GetValue() const
		{
			return *(const T*)m_buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= sizeof(m_buffer));
			memcpy(m_buffer, &value, sizeof(T));
		}

		const std::string& GetString() const { return m_string; }
		void SetString(const std::string& value) { m_string = value; }

		void* GetBuffer() { return m_buffer; }
		const void* GetBuffer() const { return m_buffer; }

	private:
		uint8_t m_buffer[16];
		std::string m_string;
	};

	class ScriptClass : public Component
	{
	public:
		DEFINE_TYPE(ScriptClass)

		ScriptClass(const std::string& className);
		~ScriptClass();

		void Init() override {};

		void SetUp();
		void DestroyInstance();

		void InvokeOnCreate();
		void InvokeOnUpdate();

		void InvokeOnDrawGizmo();

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

		void SetClass(const std::string& fullName);
		const std::string& GetClassName() const { return m_className; }
		std::shared_ptr<ScriptingClass> GetScriptingClass() const { return m_scriptingClass; }

		_MonoObject* GetInstance() const { return m_instance; }

		template<typename T>
		T GetRuntimeFieldValue(const std::string& name)
		{
			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetRuntimeFieldValue(const std::string& name, T value)
		{
			SetFieldValueInternal(name, &value);
		}

		template<typename T>
		T GetFieldValue(const std::string& name) const
		{
			auto it = m_scriptFields.find(name);
			if (it == m_scriptFields.end())
				return T();

			return it->second.GetValue<T>();
		}

		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			m_scriptFields[name].SetValue<T>(value);
		}

		std::string GetRuntimeFieldString(const std::string& name);
		void SetRuntimeFieldString(const std::string& name, const std::string& value);

		std::string GetFieldString(const std::string& name) const;
		void SetFieldString(const std::string& name, const std::string& value);

		bool IsSameType(std::shared_ptr<ScriptingClass> scriptingClass) const{
			return m_scriptingClass->IsSameType(scriptingClass);
		}

		bool IsSameType(const std::string& type) const{
			return m_scriptingClass->IsSameType(type);
		}

	private:

		bool GetFieldValueInternal(const std::string& fieldName, void* buffer); /// Used to get data when mono is running
		bool SetFieldValueInternal(const std::string& fieldName, const void* value); /// Used to change data when mono is running

	private:
		inline static char s_FieldValueBuffer[16];

		std::shared_ptr<ScriptingClass> m_scriptingClass;
		std::unordered_map<std::string, ScriptFieldData> m_scriptFields;  /// Used to store data when mono not running

		std::string m_className;

		_MonoObject* m_instance = nullptr;
		_MonoMethod* m_OnCreate = nullptr;
		_MonoMethod* m_OnUpdate = nullptr;
		_MonoMethod* m_OnDrawGizmo = nullptr;
		_MonoMethod* m_Constructor = nullptr;

		
	};
}