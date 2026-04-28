#pragma once

#include <vector>

#include "Loopie/Render/BufferLayout.h"
namespace Loopie
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, unsigned int size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		void SetData(const void* data, unsigned int size);
		void SetDataDynamic(const void* data, unsigned int size);

		BufferLayout& GetLayout() { return m_layout; }
		void SetLayout(const BufferLayout& layout) { m_layout = layout; }

		unsigned int GetRendererID()const { return m_rendererID; }

	private:
		unsigned int m_rendererID = 0;
		BufferLayout m_layout;
	};
}