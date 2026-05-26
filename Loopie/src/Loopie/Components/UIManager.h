#pragma once

#include "Loopie/Components/Component.h"

#include "Loopie/UI/UINavigable.h"

#include "Loopie/Core/UUID.h"

#include "Loopie/Math/MathTypes.h"

#include <vector>

namespace Loopie
{
	class Scene;

	class UIManager : public Component
	{
	public:
		DEFINE_TYPE(UIManager)

		enum class BindingType
		{
			KeyboardScancode = 0,
			GamepadButton = 1,
			GamepadAxis = 2
		};

		struct InputBinding
		{
			BindingType Type = BindingType::KeyboardScancode;
			int Code = 0;
			float AxisDirection = 1.0f;
		};

		struct AxisKey
		{
			int Code;
			float Direction;

			bool operator==(const AxisKey& other) const
			{
				return Code == other.Code && Direction == other.Direction;
			}
		};

		struct AxisKeyHash
		{
			std::size_t operator()(const AxisKey& k) const
			{
				return std::hash<int>()(k.Code) ^ std::hash<int>()((int)(k.Direction * 1000));
			}
		};


		UIManager();
		~UIManager() override = default;

		void Init() override {}
		void OnUpdate() override;
		void OnSceneDeserialized() override;

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

		UUID GetSelectedEntity() const { return m_selectedEntity; }
		void SetSelectedEntity(const UUID& entityUUID);
		void ClearSelection();

		bool GetBlockNavigation() const { return m_blockNavigation; }
		void SetBlockNavigation(bool blocked) { m_blockNavigation = blocked; }

		std::vector<InputBinding>& GetMoveUpBindings() { return m_moveUpBindings; }
		std::vector<InputBinding>& GetMoveDownBindings() { return m_moveDownBindings; }
		std::vector<InputBinding>& GetMoveLeftBindings() { return m_moveLeftBindings; }
		std::vector<InputBinding>& GetMoveRightBindings() { return m_moveRightBindings; }
		std::vector<InputBinding>& GetSelectBindings() { return m_selectBindings; }

		const std::vector<InputBinding>& GetMoveUpBindings() const { return m_moveUpBindings; }
		const std::vector<InputBinding>& GetMoveDownBindings() const { return m_moveDownBindings; }
		const std::vector<InputBinding>& GetMoveLeftBindings() const { return m_moveLeftBindings; }
		const std::vector<InputBinding>& GetMoveRightBindings() const { return m_moveRightBindings; }
		const std::vector<InputBinding>& GetSelectBindings() const { return m_selectBindings; }

	private:
		struct PickCandidate;

		static UIElement* FindUIElementComponent(const std::shared_ptr<Entity>& entity, bool allowInactive = false);
		static bool CandidateLess(const PickCandidate& a, const PickCandidate& b);
		static void CollectOverlayPickCandidatesRecursive(const std::shared_ptr<Entity>& entity, const vec2& mouseCanvas,
			int canvasSortingLayer, int canvasOrderInLayer,
			std::vector<PickCandidate>& outCandidates, uint64_t& inOutTraversal);
		static bool TryApplyDeserializedSelection(Scene& scene, const UUID& desired);

		bool AnyPressed(const std::vector<InputBinding>& bindings) const;
		void TryMove(UINavigationDirection dir);
		void TrySubmit();
		void HandleMouseSelection();

	private:
		UUID m_initialSelectedEntity = UUID::Invalid;
		UUID m_selectedEntity = UUID::Invalid;

		std::vector<InputBinding> m_moveUpBindings;
		std::vector<InputBinding> m_moveDownBindings;
		std::vector<InputBinding> m_moveLeftBindings;
		std::vector<InputBinding> m_moveRightBindings;
		std::vector<InputBinding> m_selectBindings;
		bool m_applySelectionOnSceneDeserialized = false;

		mutable std::unordered_map<AxisKey, bool, AxisKeyHash> m_axisWasActive;
		bool m_blockNavigation = false;
	};
}
