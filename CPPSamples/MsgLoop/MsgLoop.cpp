#include <Windows.h>
#include <process.h>
#include <stdio.h>

using namespace std;

#define DELETE_PTR_IF_EXIST(pointer) \
	if (pointer != NULL){ \
		delete pointer; \
		pointer = NULL; \
	}

#define CLOSE_HANDLE_IF_EXIST(handle) \
	if (handle != NULL){ \
		CloseHandle(handle); \
		handle = NULL; \
	}

#define DESTROY_HWND_IF_EXIST(hwnd) \
	if (hwnd != NULL){ \
		DestroyWindow(hwnd); \
		hwnd = NULL; \
	}


class TestKeyWindow 
{
public:
	TestKeyWindow(HANDLE& hWndCloseEvent) :
		m_WndCloseEvent(hWndCloseEvent),
		m_hWnd(NULL),
		m_pOriWndProc(NULL),
		m_hMessagePumpThread(NULL)
	{
	}

	~TestKeyWindow()
	{
		CLOSE_HANDLE_IF_EXIST(m_hMessagePumpThread);
		DESTROY_HWND_IF_EXIST(m_hWnd);
	}

	void StartShowWindowThread()
	{
		UINT nMessageThreadId;
		m_hMessagePumpThread = (HANDLE)::_beginthreadex(NULL, 0, &TestKeyWindow::MessagePumpThreadProc, (LPVOID)this, 0, &nMessageThreadId);
	}

private:
	bool InitializeWindow()
	{
		m_hWnd = CreateWindow(L"Static",
			L"Test",
			WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			100,
			100,
			100,
			100,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			(LPVOID)this);

		if (m_hWnd == NULL) {
			DWORD err = GetLastError();
			printf("Error. CreateWindow() failed. Error code: %d.\n", err);
			return false;
		}

		SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)(this));
		m_pOriWndProc = SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)&TestKeyWindow::MyWndProc);

		ShowWindow(m_hWnd, SW_SHOW);
		return true;
	}

	static unsigned WINAPI MessagePumpThreadProc(LPVOID lpThreadParameter)
	{
		MSG msg;
		TestKeyWindow* pThis = (TestKeyWindow*)lpThreadParameter;
		if (!pThis->InitializeWindow()) {
			SetEvent(pThis->m_WndCloseEvent);
			return -1;
		}

		MSG* pmsg = &msg;
		while (GetMessage(pmsg, NULL, 0, 0)) {

			UINT uVkReal = 0;

			if (pmsg->wParam == VK_PROCESSKEY && (pmsg->message == WM_KEYDOWN || pmsg->message == WM_SYSKEYDOWN))
			{
				uVkReal = ImmGetVirtualKey(pmsg->hwnd);
				printf("VirtualKeyCode(wParam): 0x%02x, AdditionalInfo(RealVKey): 0x%08x\n", static_cast<unsigned short>(pmsg->wParam), uVkReal);
			}

			TranslateMessage(pmsg);
			DispatchMessage(pmsg);
		}

		return 0;
	}

	static LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			/*const auto immKey = ImmGetVirtualKey(hWnd);*/

			unsigned short virtualKey = static_cast<unsigned short>(wParam);
			//unsigned short virtualKey = static_cast<unsigned short>(immKey);
			unsigned int additionalInfo = static_cast<unsigned int>(lParam);

			break;
		}
		case WM_CLOSE:
		{
			TestKeyWindow* pThis = (TestKeyWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetEvent(pThis->m_WndCloseEvent);
			break;
		}
		default:
			break;
		}

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	HANDLE& m_WndCloseEvent;
	HANDLE m_hMessagePumpThread;
	HWND m_hWnd;
	LONG_PTR m_pOriWndProc;
};

unsigned int CALLBACK StartNoticeWnd(LPVOID lpThreadParameter)
{
	HANDLE hWndCloseEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	TestKeyWindow* main_window = new TestKeyWindow(hWndCloseEvent);
	main_window->StartShowWindowThread();

	DWORD dwResultCode = 0;
	do {
		WaitForSingleObject(hWndCloseEvent, INFINITE);
	} while (dwResultCode != WAIT_OBJECT_0);

	DELETE_PTR_IF_EXIST(main_window);
	CLOSE_HANDLE_IF_EXIST(hWndCloseEvent);
	return 0;
}



int main()
{
	unsigned int nThreadId = 0;
	HANDLE hThread = (HANDLE)::_beginthreadex(NULL, 0, &StartNoticeWnd, NULL, 0, &nThreadId);

	if (hThread == NULL) {
		printf("Error, _beginthreadex() failed.\n");
		return 1;
	}

	printf("Please press keys. Notes the \"Test\" Window should be foreground.\n");
	DWORD dwResultCode = 0;
	do {
		dwResultCode = WaitForSingleObject(hThread, INFINITE);
	} while (dwResultCode != WAIT_OBJECT_0);

	CLOSE_HANDLE_IF_EXIST(hThread);
	return 0;
}
