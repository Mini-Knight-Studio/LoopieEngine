#include "Loopie/Render/FrameBuffer.h"
#include "Loopie/Core/Application.h"

namespace Loopie {
	class Camera;

	class Interface {
	public:
		Interface() = default;
		~Interface() = default;
		virtual void Update(const InputEventManager& inputEvent) {};
		virtual void Render() {};
		virtual void Init() {};

		bool IsFocused() const { return m_focused; }

	protected:
		bool m_focused = false;
	};


	class GameLogic : public Interface {
	public:
		GameLogic();
		~GameLogic();
		void Init() override;
		void Update(const InputEventManager& inputEvent) override;
		void Render() override;

		void StartScene();
		void EndScene();

		bool IsVisible() { return m_visible; }

		Camera* GetCamera();

	private:
		bool m_visible = false;
		std::shared_ptr<FrameBuffer> m_buffer;
		ivec2 m_windowSize = ivec2(0);
	};
}