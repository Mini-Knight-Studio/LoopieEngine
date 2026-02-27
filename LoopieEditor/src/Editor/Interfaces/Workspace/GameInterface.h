#include "Editor/Interfaces/Interface.h"
#include "Loopie/Render/FrameBuffer.h"

namespace Loopie {
	class Camera;
	class GameInterface : public Interface {
	public:
		GameInterface();
		~GameInterface() = default;
		void Init() override {}
		void Render() override;

		void StartScene();
		void EndScene();

		bool IsVisible() { return m_visible; }

		std::shared_ptr<FrameBuffer> GetFrameBuffer() const { return m_buffer; }

		Camera* GetCamera();

	private:
		bool m_visible = false;
		std::shared_ptr<FrameBuffer> m_buffer;
		ivec2 m_windowSize = ivec2(0);
	};
}