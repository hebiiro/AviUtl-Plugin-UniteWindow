#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

void AviUtlWindow::init(HWND hwnd)
{
	m_hwnd = hwnd;
	m_hwndContainer = createContainerWindow(
		hwnd, containerWndProc, _T("UniteWindow.AviUtlWindow"));

	::SetParent(m_hwnd, m_hwndContainer);

	DWORD style = ::GetWindowLong(m_hwnd, GWL_STYLE);
	style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	style |= WS_CHILD;
	::SetWindowLong(m_hwnd, GWL_STYLE, style);
#if 0
	DWORD exStyle = ::GetWindowLong(m_hwnd, GWL_EXSTYLE);
	exStyle |= WS_EX_NOACTIVATE;
	::SetWindowLong(m_hwnd, GWL_EXSTYLE, exStyle);
#endif
	::SetWindowPos(m_hwnd, 0, 0, 0, 0, 0,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	g_aviutlWindowProc = (WNDPROC)::SetWindowLongPtr(
		m_hwnd, GWLP_WNDPROC, (LONG_PTR)aviutlWindowProc);

	HICON icon = (HICON)::GetClassLong(m_hwnd, GCL_HICON);
	::SetClassLong(g_singleWindow, GCL_HICON, (LONG)icon);
	::SetClassLong(g_singleWindow, GCL_HICONSM, (LONG)icon);
}

LRESULT CALLBACK AviUtlWindow::containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {};
			HDC dc = ::BeginPaint(hwnd, &ps);
			HBRUSH brush = (HBRUSH)::SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwnd);
			if (brush) ::FillRect(dc, &ps.rcPaint, brush);
			::EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_SIZE:
		{
			RECT rc; ::GetClientRect(hwnd, &rc);

			HWND child = getWindow(hwnd);
			RECT rcClient; ::GetClientRect(child, &rcClient);
			RECT rcWindow; ::GetWindowRect(child, &rcWindow);
			::MapWindowPoints(child, 0, (POINT*)&rcClient, 2);

			rc.left += rcWindow.left - rcClient.left;
			rc.top += rcWindow.top - rcClient.top;
			rc.right += rcWindow.right - rcClient.right;
			rc.bottom += rcWindow.bottom - rcClient.bottom;

			int x = rc.left;
			int y = rc.top;
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;

			::MoveWindow(child, x, y, w, h, TRUE);

			break;
		}
	case WM_SETFOCUS:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			::SetFocus(g_aviutlWindow.m_hwnd);

			break;
		}
	}

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK aviutlWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		{
			MY_TRACE(_T("aviutlWindowProc(WM_DESTROY)\n"));

			saveConfig();

			break;
		}
	case WM_SETTEXT:
		{
//			MY_TRACE(_T("aviutlWindowProc(WM_SETTEXT)\n"));

			::InvalidateRect(g_singleWindow, 0, FALSE);

			AviUtl::EditHandle* editp = (AviUtl::EditHandle*)g_auin.GetEditp();

			char fileName[MAX_PATH] = {};
			if (editp->frame_n)
			{
				::StringCbCopyA(fileName, sizeof(fileName), editp->project_filename);
				::PathStripPathA(fileName);
			}
			else
			{
				::StringCbCopyA(fileName, sizeof(fileName), "無題");
			}
			::StringCbCatA(fileName, sizeof(fileName), " - AviUtl");
			::SetWindowTextA(g_singleWindow, fileName);

			break;
		}
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		{
			::InvalidateRect(g_singleWindow, 0, FALSE);

			break;
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			::SetFocus(hwnd);

			break;
		}
	}

	return ::CallWindowProc(g_aviutlWindowProc, hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
