#pragma once


namespace Loopie
{
	// *** ShadowMap standard depth *** - PSS 26/03/26
	// The standard depth for a ShadowMap is 24 bits. Can be increased or lowered
	// for more / less samples (either higher accuracy or optimization)
	// The value you should look for is GL_DEPTH_COMPONENT24 (in the .cpp) if you want to modify it.
	class ShadowMap
	{
	public:
		ShadowMap(int width = 2048, int height = 2048); 
		~ShadowMap();

		void Bind() const;
		void Unbind() const;

		void Clear() const;

		void BindTexture(unsigned int slot);

		unsigned int GetTextureId() { return m_textureBufferID; }
		unsigned int GetWidth() { return m_width; }
		unsigned int GetHeight() { return m_height; }

	private:
		unsigned int m_fboID = 0;

		unsigned int m_textureBufferID = 0;
		unsigned int m_width = 0;
		unsigned int m_height = 0;
	};
}