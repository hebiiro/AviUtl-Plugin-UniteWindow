﻿#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

void ExeditWindow::init(HWND hwnd)
{
	m_hwnd = hwnd;
	m_hwndContainer = createContainerWindow(
		hwnd, containerWndProc, _T("UniteWindow.ExeditWindow"));

	::SetParent(m_hwnd, m_hwndContainer);

	DWORD style = ::GetWindowLong(m_hwnd, GWL_STYLE);
	style &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	::SetWindowLong(m_hwnd, GWL_STYLE, style);
#if 0
	DWORD exStyle = ::GetWindowLong(m_hwnd, GWL_EXSTYLE);
	exStyle |= WS_EX_NOACTIVATE;
	::SetWindowLong(m_hwnd, GWL_EXSTYLE, exStyle);
#endif
	::SetWindowPos(m_hwnd, 0, 0, 0, 0, 0,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	g_exeditWindowProc = (WNDPROC)::SetWindowLongPtr(
		m_hwnd, GWLP_WNDPROC, (LONG_PTR)exeditWindowProc);
}

LRESULT CALLBACK ExeditWindow::containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

			rc.bottom -= g_auin.GetLayerHeight() / 2;
			::SendMessage(child, WM_SIZING, WMSZ_BOTTOM, (LPARAM)&rc);

			int x = rc.left;
			int y = rc.top;
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;

			::MoveWindow(child, x, y, w, h, TRUE);

			break;
		}
	}

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK exeditWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATE:
		{
			MY_TRACE(_T("exeditWindowProc(WM_ACTIVATE, 0x%08X, 0x%08X)\n"), wParam, lParam);

			if (LOWORD(wParam) == WA_CLICKACTIVE)
				::SetWindowPos(g_singleWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			break;
		}
	case WM_NCPAINT:
		{
			HDC dc = ::GetWindowDC(hwnd);
			RECT rc; ::GetWindowRect(hwnd, &rc);
			::OffsetRect(&rc, -rc.left, -rc.top);
			HBRUSH brush = (HBRUSH)::SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwnd);
			if (brush) ::FillRect(dc, &rc, brush);
			::ReleaseDC(hwnd, dc);
			return 0;
		}
	}

	return ::CallWindowProc(g_exeditWindowProc, hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
