﻿#include "External.h"
#include "SimplePbr.h"


void *operator new[](size_t Size, const char* /*Name*/, int /*Flags*/,
                     unsigned /*DebugFlags*/, const char* /*File*/, int /*Line*/)
{
    return malloc(Size);
}

void *operator new[](size_t Size, size_t Alignment, size_t AlignmentOffset, const char* /*Name*/,
                     int /*Flags*/, unsigned /*DebugFlags*/, const char* /*File*/, int /*Line*/)
{
    return _aligned_offset_malloc(Size, Alignment, AlignmentOffset);
}

namespace Priv
{

static void
UpdateFrameStats(HWND Window, const char* Name, double& OutTime, float& OutDeltaTime)
{
    static double PreviousTime = -1.0;
    static double HeaderRefreshTime = 0.0;
    static unsigned FrameCount = 0;

    if (PreviousTime < 0.0)
    {
        PreviousTime = Misc::GetTime();
        HeaderRefreshTime = PreviousTime;
    }

    OutTime = Misc::GetTime();
    OutDeltaTime = (float)(OutTime - PreviousTime);
    PreviousTime = OutTime;

    if ((OutTime - HeaderRefreshTime) >= 1.0)
    {
        const double FramesPerSecond = FrameCount / (OutTime - HeaderRefreshTime);
        const double MilliSeconds = (1.0 / FramesPerSecond) * 1000.0;
        char Header[256];
        snprintf(Header, sizeof(Header), "[%.1f fps  %.3f ms] %s", FramesPerSecond, MilliSeconds, Name);
        SetWindowText(Window, Header);
        HeaderRefreshTime = OutTime;
        FrameCount = 0;
    }
    FrameCount++;
}

static LRESULT CALLBACK
ProcessWindowMessage(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    ImGuiIO& Io = ImGui::GetIO();

    switch (Message)
    {
    case WM_LBUTTONDOWN:
        Io.MouseDown[0] = true;
        return 0;
    case WM_LBUTTONUP:
        Io.MouseDown[0] = false;
        return 0;
    case WM_RBUTTONDOWN:
        Io.MouseDown[1] = true;
        return 0;
    case WM_RBUTTONUP:
        Io.MouseDown[1] = false;
        return 0;
    case WM_MBUTTONDOWN:
        Io.MouseDown[2] = true;
        return 0;
    case WM_MBUTTONUP:
        Io.MouseDown[2] = false;
        return 0;
    case WM_MOUSEWHEEL:
        Io.MouseWheel += GET_WHEEL_DELTA_WPARAM(WParam) > 0 ? 1.0f : -1.0f;
        return 0;
    case WM_MOUSEMOVE:
        Io.MousePos.x = (signed short)(LParam);
        Io.MousePos.y = (signed short)(LParam >> 16);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        {
            if (WParam < 256)
            {
                Io.KeysDown[WParam] = true;
                if (WParam == VK_ESCAPE)
                    PostQuitMessage(0);
                return 0;
            }
        }
        break;
    case WM_KEYUP:
        {
            if (WParam < 256)
            {
                Io.KeysDown[WParam] = false;
                return 0;
            }
        }
        break;
    case WM_CHAR:
        {
            if (WParam > 0 && WParam < 0x10000)
            {
                Io.AddInputCharacter((unsigned short)WParam);
                return 0;
            }
        }
        break;
    }
    return DefWindowProc(Window, Message, WParam, LParam);
}

static HWND
InitializeWindow(const char* Name, unsigned Width, unsigned Height)
{
    WNDCLASS WinClass = {};
    WinClass.lpfnWndProc = ProcessWindowMessage;
    WinClass.hInstance = GetModuleHandle(nullptr);
    WinClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WinClass.lpszClassName = Name;
    if (!RegisterClass(&WinClass))
        assert(0);

    RECT Rect = { 0, 0, (LONG)Width, (LONG)Height };
    if (!AdjustWindowRect(&Rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 0))
        assert(0);

    HWND Window = CreateWindowEx(
        0, Name, Name, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        Rect.right - Rect.left, Rect.bottom - Rect.top,
        nullptr, nullptr, nullptr, 0);
    assert(Window);
    return Window;
}

static void
BeginFrame()
{
    ID3D12CommandAllocator* CmdAlloc = Dx::GCmdAlloc[Dx::GFrameIndex];
    ID3D12GraphicsCommandList* CmdList = Dx::GCmdList;

    CmdAlloc->Reset();
    CmdList->Reset(CmdAlloc, nullptr);

    Dx::SetDescriptorHeap();

    CmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, (float)Dx::GResolution[0], (float)Dx::GResolution[1]));
    CmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, Dx::GResolution[0], Dx::GResolution[1]));

    CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Dx::GSwapBuffers[Dx::GBackBufferIndex],
                                                                      D3D12_RESOURCE_STATE_PRESENT,
                                                                      D3D12_RESOURCE_STATE_RENDER_TARGET));
}

static void
EndFrame()
{
    Dx::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Dx::GSwapBuffers[Dx::GBackBufferIndex],
                                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                           D3D12_RESOURCE_STATE_PRESENT));
    VHR(Dx::GCmdList->Close());

    Dx::GCmdQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&Dx::GCmdList);
}

} // namespace Priv

int CALLBACK
WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    const char* WindowName = "SimplePbr";
    const unsigned WindowWidth = 1920;
    const unsigned WindowHeight = 1080;
    
    SetProcessDPIAware();
    ImGui::CreateContext();

    Dx::Initialize(Priv::InitializeWindow(WindowName, WindowWidth, WindowHeight));

    ImGuiIO& Io = ImGui::GetIO();
    Io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    Io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    Io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    Io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    Io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    Io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    Io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    Io.KeyMap[ImGuiKey_Home] = VK_HOME;
    Io.KeyMap[ImGuiKey_End] = VK_END;
    Io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    Io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    Io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    Io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    Io.KeyMap[ImGuiKey_A] = 'A';
    Io.KeyMap[ImGuiKey_C] = 'C';
    Io.KeyMap[ImGuiKey_V] = 'V';
    Io.KeyMap[ImGuiKey_X] = 'X';
    Io.KeyMap[ImGuiKey_Y] = 'Y';
    Io.KeyMap[ImGuiKey_Z] = 'Z';
    Io.ImeWindowHandle = Dx::GWindow;
    Io.RenderDrawListsFn = nullptr;
    Io.DisplaySize = ImVec2((float)Dx::GResolution[0], (float)Dx::GResolution[1]);
    ImGui::GetStyle().WindowRounding = 0.0f;

    Gui::Initialize();
    SimplePbr::Initialize();

    // Upload resources to the GPU.
    VHR(Dx::GCmdList->Close());
    Dx::GCmdQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&Dx::GCmdList);
    Dx::WaitForGpu();

    for (ID3D12Resource* Resource : Dx::GIntermediateResources)
        SAFE_RELEASE(Resource);

    Dx::GIntermediateResources.clear();


    for (;;)
    {
        MSG Message = {};
        if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&Message);
            if (Message.message == WM_QUIT)
                break;
        }
        else
        {
            double Time;
            float DeltaTime;
            Priv::UpdateFrameStats(Dx::GWindow, WindowName, Time, DeltaTime);

            ImGuiIO& Io = ImGui::GetIO();
            Io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            Io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            Io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
            Io.DeltaTime = DeltaTime;

            Priv::BeginFrame();
            ImGui::NewFrame();
            SimplePbr::Update(Time, DeltaTime);
            ImGui::Render();
            Gui::Render();
            Priv::EndFrame();
            Dx::PresentFrame();
        }
    }

    Dx::WaitForGpu();
    SimplePbr::Shutdown();
    Gui::Shutdown();
    Dx::Shutdown();
    return 0;
}

#include "SimplePbr.cpp"
#include "Misc.cpp"
#include "Directx12.cpp"
#include "Gui.cpp"
// vim: set ts=4 sw=4 expandtab: