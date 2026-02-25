#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"

namespace Loopie
{
	class RectTransform : public Transform
	{
	public:
		DEFINE_TYPE(RectTransform)

        RectTransform(float w = 100.f, float h = 100.f);

		bool IsRectTransform() const override { return true; }

        float GetWidth() const override;
        float GetHeight() const override;

        void SetWidth(float w) override;
		void SetHeight(float h) override;

        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;

		void RefreshMatrices() const override;

    private:
        float m_width;
        float m_height;
	};
}