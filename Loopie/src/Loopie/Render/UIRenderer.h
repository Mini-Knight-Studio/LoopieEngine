#include "Loopie/Math/MathTypes.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"

#include <memory>

namespace Loopie {
	
	class UIRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void DrawRect(const vec2& posPixels, const vec2& sizePixels, const vec4& color);

	private:
		static void EnsureInit();
	
	private:
		static bool s_initialized;
		static std::shared_ptr<VertexArray> s_quadVAO;
		static std::shared_ptr<VertexBuffer> s_quadVBO;
		static std::shared_ptr<IndexBuffer> s_quadEBO;
		static std::shared_ptr<Material> s_material;
		static Shader* s_shader;
	};
}