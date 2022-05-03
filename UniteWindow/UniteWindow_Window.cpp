#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

HWND Window::getWindow(HWND hwndContainer)
{
	return (HWND)::GetProp(hwndContainer, _T("UniteWindow.Window"));
}

void Window::setWindow(HWND hwndContainer, HWND hwnd)
{
	::SetProp(hwndContainer, _T("UniteWindow.Window"), hwnd);
}

HWND Window::createContainerWindow(HWND hwnd, WNDPROC wndProc, LPCTSTR className)
{
	MY_TRACE(_T("createContainerWindow(0x%08X, %s)\n"), hwnd, className);

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc = wndProc;
	wc.hInstance = g_instance;
	wc.lpszClassName = className;
	::RegisterClass(&wc);

	HWND hwndContainer = ::CreateWindowEx(
		0,
		className,
		className,
		WS_VISIBLE | WS_CHILD |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, 0, 0,
		g_singleWindow, 0, g_instance, 0);

	setWindow(hwndContainer, hwnd);

	return hwndContainer;
}

void Window::resize(LPCRECT rc)
{
	int x = rc->left;
	int y = rc->top;
	int w = rc->right - rc->left;
	int h = rc->bottom - rc->top;

	x = min(rc->left, rc->right);
	y = min(rc->top, rc->bottom);
	w = max(w, 0);
	h = max(h, 0);

	::MoveWindow(m_hwndContainer, x, y, w, h, TRUE);
}

//---------------------------------------------------------------------
