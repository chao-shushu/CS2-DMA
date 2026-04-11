#include "OS-ImGui_Base.h"

#include "../CustomFont.h"


/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Base.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/6/18	11:21
****************************************************/

namespace OSImGui
{
    bool OSImGui_Base::InitImGui(ID3D11Device* device, ID3D11DeviceContext* device_context)
    {

        ImGui::CreateContext();
        //ImGui::GetIO().Framerate = 0; // ��������� ���������� �� ����

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)myFont, sizeof(myFont), 15.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

        iconFont = io.Fonts->AddFontFromMemoryTTF((void*)myFont, sizeof(myFont), 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

        io.Fonts->Build();

        ImGui::StyleColorsDark();
        io.LogFilename = nullptr;

        if (!ImGui_ImplWin32_Init(Window.hWnd))
            throw OSException("ImGui_ImplWin32_Init() call failed.");
        if (!ImGui_ImplDX11_Init(device, device_context))
            throw OSException("ImGui_ImplDX11_Init() call failed.");

        return true;
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        g_Device.CleanupDeviceD3D();
        DestroyWindow(Window.hWnd);
        UnregisterClassA(Window.ClassName.c_str(), Window.hInstance);
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }

}