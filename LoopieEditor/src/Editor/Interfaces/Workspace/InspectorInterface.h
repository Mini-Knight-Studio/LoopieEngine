#pragma once 

#include "Editor/Interfaces/Interface.h"
#include "Loopie/Events/IObserver.h"
#include "Editor/Events/EditorEventTypes.h"

#include <memory>

namespace Loopie {
	class Transform;
	class Camera;
	class MeshRenderer;
	class ScriptClass;
	class Material;
	class Texture;
	class Animator;
	class BoxCollider;
	class RectTransform;
	class Canvas;
	class CanvasScaler;
	class Image;
	class Text;
	class Button;
	class AudioSource;
	class AudioListener;
	class FunctionCall;
	class Metadata;

	class InspectorInterface : public Interface , public IObserver<OnEntityOrFileNotification>{
	public:
		InspectorInterface();
		~InspectorInterface();
		void Init() override;
		void Render() override;

	private:

		enum class InspectorMode {
			EntityMode,
			ImportMode,
			None
		};

		void DrawEntityInspector(const std::shared_ptr<Entity>& entity);
		void DrawFileImportSettings(const std::filesystem::path& path);

		///EntityRelated
		void DrawEntityConfig(const std::shared_ptr<Entity>& entity);
		void DrawTransform(Transform* transform);
		void DrawCamera(Camera* camera);
		void DrawMeshRenderer(MeshRenderer* meshRenderer);
		void DrawAnimator(Animator* animator);
		void DrawScriptClass(ScriptClass* scriptClass);
		void DrawCanvas(Canvas* canvas);
		void DrawCanvasScaler(CanvasScaler* canvasScaler);
		void DrawImage(Image* image);
		void DrawText(Text* text);
		void DrawButton(Button* button);
		void DrawBoxCollider(BoxCollider* boxCollider);
		void DrawAudioSource(AudioSource* source);
		void DrawAudioListener(AudioListener* listener);

		void AddComponent(const std::shared_ptr<Entity>& entity);
		bool ComponentContextMenu(Component* component, bool canRemove = true);

		///FilesRelated
		void DrawMetadata(const Metadata* metadata);
		void DrawMaterial(std::shared_ptr<Material> material);

		// Inherited via IObserver
		void OnNotify(const OnEntityOrFileNotification& id) override;
	
	private:

		enum class ButtonImageSlot
		{
			Normal,
			Hovered,
			Pressed,
			Disabled
		};

		void DrawImageButtonSlot(Button* button, ButtonImageSlot slot);

	private:

		InspectorMode m_mode = InspectorMode::None;
		bool m_locked = false;

		std::weak_ptr<Entity> m_currentEntity;
		std::filesystem::path m_currentFile;
	};
}