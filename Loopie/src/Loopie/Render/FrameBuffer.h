#pragma once

namespace Loopie
{
	class FrameBuffer
	{
	public:
		enum class FrameBufferFormat { RGBA8, RGBA16F };
		
		FrameBuffer(unsigned int width, unsigned int height, FrameBufferFormat internalFormat = FrameBufferFormat::RGBA16F, bool withDepth = true);
		~FrameBuffer();

		void Bind() const;
		void Unbind() const;

		void Clear() const;

		void Resize(unsigned int width, unsigned int height);

		unsigned int GetRendererId() { return m_rendererID; }
		unsigned int GetTextureId() { return m_textureBufferID; }
		unsigned int GetDepthId() { return m_depthTextureID; }
		unsigned int GetWidth() { return m_width; }
		unsigned int GetHeight() { return m_height; }

	private:
		unsigned int m_rendererID = 0;

		unsigned int m_textureBufferID = 0;
		unsigned int m_depthTextureID = 0;
		unsigned int m_width = 0;
		unsigned int m_height = 0;
		unsigned int m_internalFormat = 0;
		bool m_withDepth = true;
	};
}