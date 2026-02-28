#include "TopBarInterface.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Importers/TextureImporter.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Component.h"
#include "Loopie/Core/Log.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Loopie
{
    TopBarInterface::TopBarInterface()
    {
        std::vector<std::string> iconsToLoad = {
            "assets/icons/icon_play.png",
            "assets/icons/icon_pause.png",
            "assets/icons/icon_stop.png",
            "assets/icons/icon_nextFrame.png"
        };

        std::vector<Metadata> iconsToLoadMetadatas;
        for (size_t i = 0; i < iconsToLoad.size(); i++)
        {
            Metadata& meta = AssetRegistry::GetOrCreateMetadata(iconsToLoad[i]);
            TextureImporter::ImportImage(iconsToLoad[i], meta);
            iconsToLoadMetadatas.emplace_back(meta);
        }

        m_playIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[0]);
        m_pauseIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[1]);
        m_stopIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[2]);
        m_nextFrameIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[3]);
    }

    void TopBarInterface::Update(const InputEventManager& inputEvent)
    {
        if (m_actualMode == PAUSED || m_actualMode == DEACTIVATED)
            return;
        if (m_actualMode == END)
        {
            m_actualMode = DEACTIVATED;
            return;
        }

        if (m_actualMode == NEXTFRAME)
            m_actualMode = PAUSED;

        if (m_actualMode == START)
            m_actualMode = UPDATING;
    }

    void TopBarInterface::Render()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, m_toolbarHeight));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("##TopBar", nullptr, flags);

        float avail = ImGui::GetContentRegionAvail().x;
        bool hasStyle = false;
        float buttonWidth = 20.0f * 3 + ImGui::GetStyle().ItemSpacing.x * 2;
        float offset = (avail - buttonWidth) * 0.5f;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (m_actualMode == DEACTIVATED)
        {
            if (ImGui::ImageButton("play", (ImTextureID)m_playIcon->GetRendererId(), ImVec2(15, 15)))
            {	
                m_actualMode = START;
            }
        }
        else
        {
            if (ImGui::ImageButton("stop", (ImTextureID)m_stopIcon->GetRendererId(), ImVec2(15, 15)) || !ScriptingManager::IsRunning())
            {    
                m_actualMode = END;
            }
        }

        ImGui::SameLine();

        if (m_actualMode == PAUSED)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.5, 0.75, 1.0));
            hasStyle = true;
        }
        if (ImGui::ImageButton("pause", (ImTextureID)m_pauseIcon->GetRendererId(), ImVec2(15, 15)) && m_actualMode != DEACTIVATED)
        {
            if (m_actualMode == PAUSED)
                m_actualMode = UPDATING;
            else
                m_actualMode = PAUSED;
        }

        if (hasStyle)
        {
            ImGui::PopStyleColor();
            hasStyle = false;
        }

        ImGui::SameLine();

        if (m_actualMode == DEACTIVATED)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            hasStyle = true;
        }

        if (ImGui::ImageButton("nextFrame", (ImTextureID)m_nextFrameIcon->GetRendererId(), ImVec2(15, 15)) && m_actualMode == PAUSED)
        {
            m_actualMode = NEXTFRAME;
        }

        if (hasStyle)
        {
            ImGui::PopStyleColor();
            hasStyle = false;
        }

        ImGui::End();
    }

}