#pragma once
#include "Loopie/Components/Camera.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Core/Application.h"

namespace Loopie
{
    class OrbitalCamera
    {
    public:
        OrbitalCamera();
        ~OrbitalCamera();

        void ProcessEvent(const InputEventManager& inputEvent);
        void Update();

        Camera* GetCamera() { return m_camera; }
        bool IsMoving() { return m_isCameraMoving;  }
		float GetMovementScale() const { return m_movementScale; }

    private:
        float m_yaw = 0;
        float m_pitch = 0;

        vec3 m_inputDirection = glm::vec3(0);
        vec3 m_panDirection = glm::vec3(0);
        vec3 m_inputRotation = glm::vec3(0);
        float m_zoomInput = 0;

        vec3 m_orbitOffset = glm::vec3(0, 0, 10);

        bool m_complexMovement = false;

        std::shared_ptr<Entity> m_entityToPivot;
        std::shared_ptr<Entity> m_entity;
        Camera* m_camera;


        bool m_isCameraMoving = false;


        float m_moveSpeed = 10.f;
		float m_faastMoveSpeed = 30.f;
		float m_movementScale = 1.f;
        float m_movementScaleIncrement = 0.1f;

        float m_speedAccumulation = 0;
        float m_speedIncrementalPercentage = 15.f;


		float m_rotationSpeed = 0.005f;
		float m_zoomSpeed = 1.f;
        float m_pivotZoomSpeed = 0.2f;
    };
}
