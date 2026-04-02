#include "ShadowMap.h"
#include "Loopie/Core/Log.h"

#include <glad/glad.h>

namespace Loopie
{
	ShadowMap::ShadowMap(int width, int height)
	{
		m_width = width;
		m_height = height;
		// Generate the FBO (glGenFramebuffers)
		glGenFramebuffers(1, &m_fboID);

		//Generate the texture (glGenTextures)
		glGenTextures(1, &m_textureBufferID);

		// Bind the texture and set it up — glBindTexture, then glTexImage2D to allocate it, 
		// then all the glTexParameteri calls plus the glTexParameterfv for border color
		glBindTexture(GL_TEXTURE_2D, m_textureBufferID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Bind the FBO (glBindFramebuffer)
		Bind();

		// Attach the texture to the FBO (glFramebufferTexture2D)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureBufferID, 0);

		// Tell OpenGL there's no color (glDrawBuffer(GL_NONE), glReadBuffer(GL_NONE))
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// Check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Log::Critical("Framebuffer is not complete");
		}

		// Unbind everything(FBO back to 0, texture back to 0)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	ShadowMap::~ShadowMap()
	{
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureBufferID);
	}

	void ShadowMap::Bind() const 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
	}

	void ShadowMap::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowMap::Clear() const 
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void ShadowMap::BindTexture(unsigned int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_textureBufferID);
	}
}