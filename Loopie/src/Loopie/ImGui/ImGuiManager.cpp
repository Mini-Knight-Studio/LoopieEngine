#include "ImGuiManager.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Window.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

namespace Loopie {
	ImGuiManager::~ImGuiManager()
	{
		ImGui_ImplSDL3_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiManager::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = "assets/imgui.ini";

		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		Window& window = Application::GetInstance().GetWindow();
		ImGui_ImplSDL3_InitForOpenGL(window.GetSDLWindow(), window.GetSDL_GL_Context());
		ImGui_ImplOpenGL3_Init();

		//ImGuizmo::AllowAxisFlip(false);

		CustomImGui();
	}

	void ImGuiManager::GetEvents(const SDL_Event& event)
	{
		ImGui_ImplSDL3_ProcessEvent(&event);
	}

	void ImGuiManager::StartFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::BeginFrame();
	}

	void ImGuiManager::EndFrame()
	{
		Window& window = Application::GetInstance().GetWindow();
		ivec2 size = window.GetSize();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)size.x, (float)size.y);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{

			SDL_Window* backup_window = window.GetSDLWindow();
			SDL_GLContext backup_context = window.GetSDL_GL_Context();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			SDL_GL_MakeCurrent(backup_window, backup_context);
		}
	}

    void ImGuiManager::CustomImGui()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        // ==============================
        // STYLE SETTINGS
        // ==============================
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 3.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
		style.ChildBorderSize = 4.f;

        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 10.0f;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;

        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        style.IndentSpacing = 22.0f;
        style.CellPadding = ImVec2(6, 6);

        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        // ==============================
        // COLOR VARIABLES (Elegant & Neutral)
        // ==============================
        ImVec4* colors = style.Colors;
       

		// Base theme colors
		ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		ImVec4 black = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		ImVec4 grayDarkest = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		ImVec4 grayDark = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		ImVec4 grayMedium = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		ImVec4 grayLight1 = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		ImVec4 grayLight2 = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		ImVec4 grayLight3 = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
		ImVec4 grayText = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		ImVec4 borderColor = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);

		// Accent colors
		ImVec4 accent = ImVec4(0.26f, 0.78f, 0.78f, 1.00f);
		ImVec4 accentHover = ImVec4(0.35f, 0.88f, 0.88f, 1.00f);
		ImVec4 accentActive = ImVec4(0.18f, 0.65f, 0.65f, 1.00f);

		// Text Colors
		colors[ImGuiCol_Text] = white;
		colors[ImGuiCol_TextDisabled] = grayText;
		colors[ImGuiCol_TextLink] = accent;
		colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);

		// Window Colors
		colors[ImGuiCol_WindowBg] = grayDark;
		colors[ImGuiCol_ChildBg] = grayDarkest;
		colors[ImGuiCol_PopupBg] = grayMedium;
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(black.x, black.y, black.z, 0.60f);

		// Border Colors
		colors[ImGuiCol_Border] = borderColor;
		colors[ImGuiCol_BorderShadow] = black;

		// Frame Colors
		colors[ImGuiCol_FrameBg] = grayMedium;
		colors[ImGuiCol_FrameBgHovered] = grayLight1;
		colors[ImGuiCol_FrameBgActive] = grayLight1;

		// Title Bar Colors
		colors[ImGuiCol_TitleBg] = grayDark;
		colors[ImGuiCol_TitleBgActive] = grayMedium;
		colors[ImGuiCol_TitleBgCollapsed] = grayDark;

		// Menu Bar Colors
		colors[ImGuiCol_MenuBarBg] = grayMedium;

		// Scrollbar Colors
		colors[ImGuiCol_ScrollbarBg] = grayDark;
		colors[ImGuiCol_ScrollbarGrab] = grayMedium;
		colors[ImGuiCol_ScrollbarGrabHovered] = grayLight1;
		colors[ImGuiCol_ScrollbarGrabActive] = grayLight1;

		// Interactive Element Colors
		colors[ImGuiCol_CheckMark] = accent;
		colors[ImGuiCol_SliderGrab] = accent;
		colors[ImGuiCol_SliderGrabActive] = accentActive;

		// Button Colors
		colors[ImGuiCol_Button] = grayMedium;
		colors[ImGuiCol_ButtonHovered] = grayLight1;
		colors[ImGuiCol_ButtonActive] = grayLight2;

		// Header Colors
		colors[ImGuiCol_Header] = grayMedium;
		colors[ImGuiCol_HeaderHovered] = grayLight1;
		colors[ImGuiCol_HeaderActive] = grayLight2;

		// Separator Colors
		colors[ImGuiCol_Separator] = borderColor;
		colors[ImGuiCol_SeparatorHovered] = accentHover;
		colors[ImGuiCol_SeparatorActive] = accentActive;

		// Resize Grip Colors
		colors[ImGuiCol_ResizeGrip] = ImVec4(accent.x, accent.y, accent.z, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent.x, accent.y, accent.z, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(accent.x, accent.y, accent.z, 0.95f);

		// Tab Colors
		colors[ImGuiCol_Tab] = grayMedium;
		colors[ImGuiCol_TabHovered] = grayLight2;
		colors[ImGuiCol_TabSelected] = grayLight2;
		colors[ImGuiCol_TabSelectedOverline] = white;
		colors[ImGuiCol_TabDimmed] = grayMedium;
		colors[ImGuiCol_TabDimmedSelected] = grayMedium;
		colors[ImGuiCol_TabDimmedSelectedOverline] = grayMedium;

		// Docking Colors
		colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = grayDark;

		// Plot Colors
		colors[ImGuiCol_PlotLines] = accent;
		colors[ImGuiCol_PlotLinesHovered] = accentHover;
		colors[ImGuiCol_PlotHistogram] = accent;
		colors[ImGuiCol_PlotHistogramHovered] = accentHover;

		// Table Colors
		colors[ImGuiCol_TableHeaderBg] = grayMedium;
		colors[ImGuiCol_TableBorderStrong] = borderColor;
		colors[ImGuiCol_TableBorderLight] = ImVec4(borderColor.x, borderColor.y, borderColor.z, 0.30f);
		colors[ImGuiCol_TableRowBg] = black;
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(white.x, white.y, white.z, 0.06f);

		// Drag & Drop Colors
		colors[ImGuiCol_DragDropTarget] = white; // Yellow highlight

		// Navigation Colors
		colors[ImGuiCol_NavCursor] = accent;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(white.x, white.y, white.z, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(black.x, black.y, black.z, 0.80f);
    }

}