#include "Loopie/Math/MathTypes.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/Resources/Types/Font.h"
#include "Loopie/Components/Text.h"

#include <memory>
#include <string>

namespace Loopie {
	
	class UIRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void DrawRect(const vec2& posPixels, const vec2& sizePixels, const vec4& color);
		static void DrawImage(const vec2& posPixels, const vec2& sizePixels, const std::shared_ptr<Texture>& texture, const vec4& tint);
		static void DrawImage(const vec2& posPixels, const vec2& sizePixels, const std::shared_ptr<Texture>& texture, const vec4& tint, const vec4& uvRect);
		static void DrawImageWorld(const matrix4& modelMatrix, const std::shared_ptr<Texture>& texture, const vec4& tint);
		static void DrawImageWorld(const matrix4& modelMatrix, const std::shared_ptr<Texture>& texture, const vec4& tint, const vec4& uvRect);

		static void DrawTextContainer(const vec2& posPixels, const vec2& sizePixels, const std::string& text, const std::shared_ptr<Font>& font, const vec4& color, float scale,
			TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign, TextWrapMode wrapMode = TextWrapMode::NoWrap);

		static void DrawTextWorld(const matrix4& modelMatrix, const vec2& sizePixels, const std::string& text, const std::shared_ptr<Font>& font, const vec4& color, float scale,
			TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign, TextWrapMode wrapMode = TextWrapMode::NoWrap);

	private:
		static void EnsureInit();

		static float AlignFactor(TextHorizontalAlignment alignment);
		static float AlignFactor(TextVerticalAlignment alignment);

		static bool IsSpaceExceptNewline(char c);
		static float MeasureCharAdvance(const std::shared_ptr<Font>& font, unsigned char ch, float fontScale, float spaceAdvance);
		static float MeasureStringAdvance(const std::shared_ptr<Font>& font, const std::string& s, float fontScale, float spaceAdvance);
		static std::string WrapTextToWidth(const std::string& text, const std::shared_ptr<Font>& font, float fontScale,
			float maxWidth, TextWrapMode wrapMode);

	private:
		static bool s_initialized;
		static std::shared_ptr<VertexArray> s_quadVAO;
		static std::shared_ptr<VertexBuffer> s_quadVBO;
		static std::shared_ptr<IndexBuffer> s_quadEBO;
		static std::shared_ptr<Material> s_material;
		static Shader* s_shader;
	};
}