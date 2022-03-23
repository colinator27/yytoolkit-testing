#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.
#include <vector>       // Include the STL vector.
#include <tuple>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

YYRValue EasyGMLCall(YYTKPlugin* pPlugin, const std::string& Name, const std::vector<YYRValue>& rvRef)
{
    // Get the callbuiltin function from the core API
    auto CallBuiltin = pPlugin->GetCoreExport<bool(*)(YYRValue& Result,
        const std::string& Name,
        CInstance* Self,
        CInstance* Other,
        const std::vector<YYRValue>& Args)>("CallBuiltin");

    // Call it like normal
    YYRValue Result;
    CallBuiltin(Result, Name, nullptr, nullptr, rvRef);

    return Result;
}

bool imguiInitialized = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Handles all events that happen inside the game.
YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    switch (pEvent->GetEventType())
    {
    case EVT_WNDPROC:
        if (imguiInitialized)
        {
            auto data = dynamic_cast<YYTKWindowProcEvent*>(pEvent);
            auto& args = data->Arguments();
            ImGui_ImplWin32_WndProcHandler(std::get<0>(args), std::get<1>(args), std::get<2>(args), std::get<3>(args));
        }
        break;
    case EVT_ENDSCENE:
        if (!imguiInitialized)
        {
            auto CallBuiltin = pPlugin->GetCoreExport<bool(*)(YYRValue& Result,
                const std::string& Name,
                CInstance* Self,
                CInstance* Other,
                const std::vector<YYRValue>& Args)>("CallBuiltin");

            YYRValue Result, Result2;
            bool Success = CallBuiltin(Result, "window_device", nullptr, nullptr, {});
            bool Success2 = CallBuiltin(Result2, "window_handle", nullptr, nullptr, {});
                
            if (!Success)
            {
                printf("window_device() failed\n");
                break;
            }
            else
            {
                if (!Success2)
                {
                    printf("window_handle() failed\n");
                    break;
                }

                IDirect3DDevice9* d3d = (IDirect3DDevice9*)Result.As<RValue>().Pointer;
                HWND* window = (HWND*)Result2.As<RValue>().Pointer;

                printf("window device: %p\n", d3d);
                printf("window handle: %p\n", window);

                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                ImGui_ImplWin32_Init(window);
                ImGui_ImplDX9_Init(d3d);
                imguiInitialized = true;
            }
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Test plugin");
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        break;
    }
        
    return YYTK_OK;
}

DllExport YYTKStatus PluginUnload(YYTKPlugin* pPlugin)
{
    return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    // Set 'PluginEventHandler' as the function to call when we a game event happens.
    // This is not required if you don't need to modify code entries / draw with D3D / anything else that requires precise timing.
    pPlugin->PluginHandler = PluginEventHandler;
    pPlugin->PluginUnload = PluginUnload;

    printf("[Test Plugin] - Plugin loaded for YYTK version %s\n", YYSDK_VERSION);

    // Tell the core everything went fine.
    return YYTK_OK;
}

// Boilerplate setup for a Windows DLL, can just return TRUE.
// This has to be here or else you get linker errors (unless you disable the main method)
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return 1;
}