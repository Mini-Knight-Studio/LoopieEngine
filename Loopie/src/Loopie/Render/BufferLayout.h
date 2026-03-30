#pragma once

#include <vector>
#include <string>

namespace Loopie {

	enum class GLVariableType {
		NONE,
		INT,
		FLOAT,
		BOOL,
		VEC2,
		VEC3,
		VEC4,
		MATRIX2,
		MATRIX3,
		MATRIX4
	};

	static unsigned int GetGLVariableSize(GLVariableType type) {
		switch (type) {
			case GLVariableType::INT:
			case GLVariableType::FLOAT:
			case GLVariableType::BOOL:
				return 4;
			case GLVariableType::VEC2:
				return 8;
			case GLVariableType::VEC3:
			case GLVariableType::VEC4:
			case GLVariableType::MATRIX2:
				return 16;
			case GLVariableType::MATRIX3:
				return 32;
			case GLVariableType::MATRIX4:
				return 64;
			default:
				return 0;
		}
	}



	struct BufferElement {

		unsigned int Index;
		GLVariableType Type;
		unsigned int Count;
		unsigned int Offset;

		BufferElement(unsigned int Index, unsigned int Offset, const GLVariableType& Type, unsigned int Count)
			: Index(Index), Offset(Offset), Type(Type), Count(Count)
		{

		}
	};

	class BufferLayout {
	public:

		void AddLayoutElement(unsigned int index,GLVariableType type, unsigned int count, const std::string& name) {
			m_layout.emplace_back(BufferElement{ index, m_stride, type, count });
			m_stride+=GetGLVariableSize(type)*count;
		}

		unsigned int GetStride()const { return m_stride; }

		const std::vector <BufferElement>& GetElements()const { return m_layout; }

		const BufferElement* GetElementByIndex(int index) const {
			for (const auto& element : m_layout) {
				if (element.Index == index)
					return &element;
			}
			return nullptr;
		}

	private:
		std::vector<BufferElement> m_layout;
		unsigned int m_stride = 0;
	};
}