#include "ShaderStorageBuffer.h"

#include <glad/glad.h>

namespace Loopie {
	ShaderStorageBuffer::ShaderStorageBuffer() {
		glGenBuffers(1, &m_rendererID);
	}

	ShaderStorageBuffer::~ShaderStorageBuffer() {
		glDeleteBuffers(1, &m_rendererID);
	}

	void ShaderStorageBuffer::Bind() const {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rendererID);
	}

	void ShaderStorageBuffer::Unbind() const {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderStorageBuffer::BindToLayout(unsigned int layoutIndex) const {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, layoutIndex, m_rendererID);
	}

	void ShaderStorageBuffer::SetData(const void* data, unsigned int byteSize) {
		Bind();
		if (byteSize > m_allocatedSize) {
			glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_DYNAMIC_DRAW);
			m_allocatedSize = byteSize;
		}
		else {
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, byteSize, data);
		}
		Unbind();
	}
}