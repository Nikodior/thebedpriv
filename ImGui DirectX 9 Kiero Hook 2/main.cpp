#include "includes.h"

#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

#include "csgo.hpp"
using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

void InitImGui(LPDIRECT3DDEVICE9 pDevice) // **Dziala**
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}


bool init = false;

bool show_main = true;

int screenX = GetSystemMetrics(SM_CXSCREEN);
int screenY = GetSystemMetrics(SM_CYSCREEN);

// Funkcje i inne zeczy potrzebne do "cheat'a".
bool esp = false; 
bool bhop = false; 

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) // *Dziala*
{
	if (!init)
	{
		InitImGui(pDevice);
		init = true;
	}

	if (GetAsyncKeyState(VK_CAPITAL) & 1)
	{
		show_main = !show_main;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (show_main)
	{
		//ImGui::ShowStyleEditor();     // Style pod koniec dodam D:D:

		ImGui::Begin("TheBed Priv");
		ImGui::Checkbox("ESP", &esp);
		ImGui::Checkbox("Bhop", &bhop);
		ImGui::End();
	}


	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { // *Dziala*

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) // *Dziala*
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skipni do nastepnego okna

	window = handle;
	return FALSE; // okno zostalo znalezione abort wyszukania. 
}

HWND GetProcessWindow() // Dziala 
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved) // Cos sie kurwa zjebalo
{
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success) // cos sie kurwa zjebalo
		{
			kiero::bind(42, (void**)& oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}

DWORD WINAPI BhopThread(LPVOID lp) // Wypierdala jak klikne w menu na funkcje "Bhop". Chuj wie czemu. *Do naprawy xD*
{
	DWORD gameModule = (DWORD)GetModuleHandle("cluent.dll"); 
	DWORD LocalPlayer = *(DWORD*)(gameModule + dwLocalPlayer);
	while (LocalPlayer == NULL) 
	{
		LocalPlayer = *(DWORD*)(gameModule + dwLocalPlayer);
	}
	while (true)
	{
		if (bhop)
		{
			DWORD flag = *(BYTE*)(LocalPlayer + m_fFlags);
			if (GetAsyncKeyState(VK_SPACE) && flag & (1 << 0))
			{
				*(DWORD*)(gameModule + dwForceJump) = 6; 
			}
		}
	}
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) // Dziala bez problemu. 
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		CreateThread(nullptr, 0, BhopThread, hMod, 0, nullptr); 
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
