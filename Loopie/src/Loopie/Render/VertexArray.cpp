#include "VertexArray.h"
#include <glad/glad.h>
namespace Loopie
{
    static GLenum ConvertGLVariableTypeToGlType(GLVariableType type) {
        switch (type)
        {
        case Loopie::GLVariableType::INT:
            return GL_INT;
            break;
        case Loopie::GLVariableType::FLOAT:
        case Loopie::GLVariableType::MATRIX2:
        case Loopie::GLVariableType::MATRIX3:
        case Loopie::GLVariableType::MATRIX4:
            return GL_FLOAT;
            break;
        case Loopie::GLVariableType::BOOL:
            return GL_BOOL;
            break;
        default:
            return GL_NONE;
            break;
        }
    }

    VertexArray::VertexArray()
    {
        glGenVertexArrays(1, &m_rendererID);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &m_rendererID);
    }

    void VertexArray::Bind() const
    {
        glBindVertexArray(m_rendererID);
    }

    void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void VertexArray::AddBuffer(VertexBuffer* vbo, IndexBuffer* ebo)
    {
        Bind();

        ebo->Bind();
        m_ebo = ebo;
        vbo->Bind();
        m_vbo = vbo;

        const BufferLayout& layout = m_vbo->GetLayout();
        for (const auto& element : layout.GetElements())
        {
            glEnableVertexAttribArray(element.Index);
            if(element.Type == GLVariableType::INT)
                glVertexAttribIPointer(element.Index, element.Count, ConvertGLVariableTypeToGlType(element.Type), layout.GetStride(), (const void*)(uintptr_t)element.Offset);
			else
                glVertexAttribPointer(element.Index, element.Count, ConvertGLVariableTypeToGlType(element.Type), GL_FALSE, layout.GetStride(), (const void*)(uintptr_t)element.Offset);
        }
        
        m_vbo->Unbind();
        Unbind();
        m_ebo->Unbind();
    }
    const IndexBuffer& Loopie::VertexArray::GetIndexBuffer() const
    {
        return *m_ebo;
    }
}