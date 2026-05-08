#pragma once

#include "Loopie/Core/UUID.h"
#include "Loopie/Core/IIdentificable.h"

namespace Loopie
{
	enum ResourceType
	{
		UNKNOWN,
		TEXTURE,
		MESH,
		MATERIAL,
		SHADER,
		SCENE,
		AUDIO,
		SCRIPT,
		FONT,
		SPRITE,
	};

	class Resource : public IIdentificable
	{
	public:
		Resource(UUID uuid, ResourceType type) : m_uuid(uuid), m_type(type) {}
		virtual ~Resource();

		const UUID &GetUUID() { return m_uuid; }
		void ReloadUUID() { m_uuid = UUID(); }

		virtual bool Load() = 0;

	protected:
		UUID m_uuid;
		ResourceType m_type;
	};
}