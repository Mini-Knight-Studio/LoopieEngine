#pragma once

namespace Loopie
{
    class ShaderStorageBuffer
    {
    private:
        unsigned int m_rendererID = 0;
        unsigned int m_allocatedSize = 0;

    public:
        ShaderStorageBuffer();
        ~ShaderStorageBuffer();

        void SetData(const void* data, unsigned int byteSize);

        void BindToLayout(unsigned int layoutIndex) const;
        void Bind() const;
        void Unbind() const;

        unsigned int GetRendererID()const { return m_rendererID; }
    };
}