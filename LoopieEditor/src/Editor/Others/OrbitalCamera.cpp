#include "OrbitalCamera.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Editor/Interfaces/Workspace/HierarchyInterface.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Math/AABB.h"

namespace Loopie
{
	OrbitalCamera::OrbitalCamera()
	{
		m_entity = std::make_shared<Entity>("OrbitalCamera");
        m_entityToPivot = m_entity;
		m_entity->AddComponent<Transform>();
		m_camera = m_entity->AddComponent<Camera>( 45.0f,  0.1f, 1000.0f, false);
	}

	OrbitalCamera::~OrbitalCamera()
	{

	}

    void OrbitalCamera::ProcessEvent(const InputEventManager& inputEvent)
    {
        m_inputDirection = vec3(0);
        m_inputRotation = vec3(0);
        m_panDirection = vec3(0);
        m_zoomInput = 0;

        m_isCameraMoving = false;

        vec2 mouseScroll = inputEvent.GetScrollDelta();
        vec2 mouseDelta = inputEvent.GetMouseDelta();

        float moveSpeed = 0;

		

        if (inputEvent.GetKeyStatus(SDL_SCANCODE_LALT) == KeyState::REPEAT && m_entity != m_entityToPivot)
        {
            if (inputEvent.GetMouseButtonStatus(0) == KeyState::REPEAT)
            {
                m_inputRotation = vec3(mouseDelta.x, mouseDelta.y, 0) * m_rotationSpeed;
            }
            if (inputEvent.GetMouseButtonStatus(2) == KeyState::REPEAT)
            {
                m_orbitOffset.z += mouseDelta.y * m_pivotZoomSpeed;
            }
        }
        else
        {
            if (inputEvent.GetMouseButtonStatus(1) == KeyState::REPEAT)
            {
                m_entityToPivot = m_entity;
                m_panDirection = vec3(-mouseDelta.x, mouseDelta.y, 0);
            }

            if (inputEvent.GetMouseButtonStatus(2) == KeyState::REPEAT)
            {
                m_entityToPivot = m_entity;
                m_inputRotation = vec3(mouseDelta.x, mouseDelta.y, 0) * m_rotationSpeed;

                if (mouseScroll.y != 0) {
					m_movementScale += mouseScroll.y * m_movementScaleIncrement;
                    if (m_movementScale < 0)
                        m_movementScale = 0.f;
                }

                bool shiftPressed = inputEvent.GetKeyStatus(SDL_SCANCODE_LSHIFT) == KeyState::REPEAT;
                moveSpeed = shiftPressed ? m_faastMoveSpeed : m_moveSpeed;
                moveSpeed += m_speedAccumulation;

                if (inputEvent.GetKeyStatus(SDL_SCANCODE_W) == KeyState::REPEAT) m_inputDirection.z += moveSpeed * m_movementScale;
                if (inputEvent.GetKeyStatus(SDL_SCANCODE_S) == KeyState::REPEAT) m_inputDirection.z -= moveSpeed * m_movementScale;
                if (inputEvent.GetKeyStatus(SDL_SCANCODE_A) == KeyState::REPEAT) m_inputDirection.x -= moveSpeed * m_movementScale;
                if (inputEvent.GetKeyStatus(SDL_SCANCODE_D) == KeyState::REPEAT) m_inputDirection.x += moveSpeed * m_movementScale;


                if (m_inputDirection != vec3(0))
                    m_isCameraMoving = true;

            }
            else
            {
                if (mouseScroll.y != 0)
                    m_zoomInput = mouseScroll.y * m_zoomSpeed;
            }
            
        }

        if (inputEvent.GetKeyStatus(SDL_SCANCODE_F) == KeyState::DOWN)
        {
			auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
            if (selectedEntity != nullptr)
            {
                m_entityToPivot = selectedEntity;
                MeshRenderer* renderer = selectedEntity->GetComponent<MeshRenderer>();
                if (renderer)
                {
                    vec3 objectScale = renderer->GetWorldAABB().GetSize();
                    float maxScaleValue = objectScale.x;
                    maxScaleValue = objectScale.y > maxScaleValue ? objectScale.y : maxScaleValue;
                    maxScaleValue = objectScale.z > maxScaleValue ? objectScale.z : maxScaleValue;
                    m_orbitOffset.z = -5 * maxScaleValue;
                }
            }
        }


        if (m_isCameraMoving) {
            float increaseAmount = (moveSpeed * m_speedIncrementalPercentage) / 100.f;
            m_speedAccumulation += increaseAmount * Time::GetDeltaTime();
        }
        else {
            m_speedAccumulation = 0;
        }

    }

    void OrbitalCamera::Update()
    {
        if (m_inputRotation == vec3{ 0.f, 0.f ,0.f} && m_inputDirection == vec3{ 0.f, 0.f, 0.f } && m_panDirection == vec3{ 0.f, 0.f, 0.f } && m_orbitOffset.z == 0.f)
            return;
        
        Transform* transform = m_entity->GetTransform();
 
        if (m_entityToPivot != m_entity)
        {
            m_yaw += m_inputRotation.x;
            m_pitch += m_inputRotation.y;

            quaternion rotation = quaternion((vec3{ m_pitch, m_yaw, 0.f }));

            Transform* pivotTransform = m_entityToPivot->GetTransform();
            vec3 pivotPos = pivotTransform->GetPosition();

            vec3 camPos = transform->GetPosition();

            vec3 offset = rotation * m_orbitOffset;

            vec3 newPos = pivotPos + offset;
            transform->SetPosition(newPos);
            transform->SetRotation(rotation);
        }
        else
        {
            m_yaw += m_inputRotation.x;
            m_pitch += m_inputRotation.y;

            m_inputDirection.x *= (float)Time::GetDeltaTime();
            m_inputDirection.z *= (float)Time::GetDeltaTime();

            m_inputDirection += m_panDirection/10.f;
            m_inputDirection.z += m_zoomInput;

            quaternion yawRotation = normalize(angleAxis(m_yaw, vec3(0, 1, 0)));
            quaternion pitchRotation = normalize(angleAxis(m_pitch, vec3(1, 0, 0)));
            quaternion orbitRotation = yawRotation * pitchRotation;

            transform->Translate(m_inputDirection, ObjectSpace::Local);
            transform->SetLocalRotation(orbitRotation);
        }
    }
}