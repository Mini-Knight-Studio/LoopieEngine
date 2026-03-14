#include "TextureBuffer.h"

#include "Loopie/Core/Log.h"

#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <glad/glad.h>

namespace Loopie
{
	TextureBuffer::TextureBuffer(const unsigned char* data, int width, int height, int channels) : m_rendererId(0)
	{
		GLenum format = GL_RGB;
		GLenum internalFormat = GL_RGB8;

		if (channels == 1) {
			format = GL_RED;
			internalFormat = GL_R8;
		}
		else if (channels == 4) {
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
		}

		glGenTextures(1, &m_rendererId);
		glBindTexture(GL_TEXTURE_2D, m_rendererId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		Unbind();
	}

	TextureBuffer::~TextureBuffer()
	{
		glDeleteTextures(1, &m_rendererId);
	}

	void TextureBuffer::Bind(unsigned int unit) const
	{
		if (unit > 31)
		{
			Log::Error("Texture unit {0} exceeds maximum (31).", unit);
			return;
		}
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, m_rendererId);

	}

	void TextureBuffer::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}