#pragma once

#include "Loopie/Core/UUID.h"
#include "Loopie/Core/IIdentificable.h"

namespace Loopie {
	enum ResourceType
	{
		UNKNOWN,
		TEXTURE,
		MESH,
		MATERIAL,
		SHADER,
		SCENE,
		SCRIPT
	};

	class Resource : public  IIdentificable {
	public:
		Resource(UUID uuid, ResourceType type) : m_uuid(uuid), m_type(type) {}
		virtual ~Resource();

		const UUID& GetUUID() { return m_uuid; }

		virtual bool Load() = 0;
	
		void IncrementReferenceCount() { m_referenceCount++; }
		void DecrementReferenceCount();
		unsigned int GetReferenceCount() const { return m_referenceCount; }	

	protected:
		UUID m_uuid;
		ResourceType m_type;
		unsigned int m_referenceCount = 0;

	};
}