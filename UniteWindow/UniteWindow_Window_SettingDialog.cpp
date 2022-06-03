#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

void SettingDialog::init(HWND hwnd)
{
	m_hwnd = hwnd;
	m_hwndContainer = createContainerWindow(
		hwnd, containerWndProc, _T("UniteWindow.SettingDialog"));

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
}

void SettingDialog::updateScrollBar()
{
	RECT rc; ::GetClientRect(m_hwnd, &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	RECT rcContainer; ::GetClientRect(m_hwndContainer, &rcContainer);
	int cw = rcContainer.right - rcContainer.left;
	int ch = rcContainer.bottom - rcContainer.top;

	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = w - 1;
	si.nPage = cw;
	::SetScrollInfo(m_hwndContainer, SB_HORZ, &si, TRUE);
	si.nMin = 0;
	si.nMax = h - 1;
	si.nPage = ch;
	::SetScrollInfo(m_hwndContainer, SB_VERT, &si, TRUE);
}

void SettingDialog::scroll(int bar, WPARAM wParam)
{
	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
	::GetScrollInfo(m_hwndContainer, bar, &si);

	switch (LOWORD(wParam))
	{
	case SB_LEFT:      si.nPos = si.nMin; break;
	case SB_RIGHT:     si.nPos = si.nMax; break;
	case SB_LINELEFT:  si.nPos += -10; break;
	case SB_LINERIGHT: si.nPos +=  10; break;
	case SB_PAGELEFT:  si.nPos += -60; break;
	case SB_PAGERIGHT: si.nPos +=  60; break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION: si.nPos = HIWORD(wParam); break;
	}

	::SetScrollInfo(m_hwndContainer, bar, &si, TRUE);
}

void SettingDialog::recalcLayout()
{
	RECT rc; ::GetClientRect(m_hwndContainer, &rc);

	RECT rcClient; ::GetClientRect(m_hwnd, &rcClient);
	RECT rcWindow; ::GetWindowRect(m_hwnd, &rcWindow);
	::MapWindowPoints(m_hwnd, 0, (POINT*)&rcClient, 2);

	rc.left += rcWindow.left - rcClient.left;
	rc.top += rcWindow.top - rcClient.top;
	rc.right += rcWindow.right - rcClient.right;
	rc.bottom += rcWindow.bottom - rcClient.bottom;

	int x = rc.left;
	int y = rc.top;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	x -= ::GetScrollPos(m_hwndContainer, SB_HORZ);
	y -= ::GetScrollPos(m_hwndContainer, SB_VERT);

	::SetWindowPos(m_hwnd, 0, x, y, 0, 0,
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

// 設定ダイアログのサイズをコンテナウィンドウのサイズまで広げる。
void SettingDialog::extend()
{
	RECT rc; ::GetWindowRect(m_hwnd, &rc);
	RECT rcContainer; ::GetWindowRect(m_hwndContainer, &rcContainer);

	int w = rc.right - rc.left;
	int h = m_rawWindowSize.cy;
	int cw = rcContainer.right - rcContainer.left;
	int ch = rcContainer.bottom - rcContainer.top;

	// 初期化が完全に終わっていないときは何もしない。
	if (w == 0 || h == 0 || cw == 0 || ch == 0) return;

	// WM_SIZE ハンドラをブロック状態にしてからサイズを変更する。
	m_blockSizeHandler = TRUE;
	::SetWindowPos(m_hwnd, 0, 0, 0, w, max(h, ch),
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	m_blockSizeHandler = FALSE;
}

LRESULT CALLBACK SettingDialog::containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETFOCUS:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			::SetFocus(g_settingDialog.m_hwnd);

			break;
		}
	}

	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {};
			HDC dc = ::BeginPaint(hwnd, &ps);
			HBRUSH brush = ::CreateSolidBrush(g_fillColor);
			FillRect(dc, &ps.rcPaint, brush);
			::DeleteObject(brush);
			::EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_SIZE:
		{
			g_settingDialog.updateScrollBar();
			g_settingDialog.recalcLayout();

			g_settingDialog.extend();

			break;
		}
	case WM_VSCROLL:
		{
			g_settingDialog.scroll(SB_VERT, wParam);
			g_settingDialog.recalcLayout();

			break;
		}
	case WM_HSCROLL:
		{
			g_settingDialog.scroll(SB_HORZ, wParam);
			g_settingDialog.recalcLayout();

			break;
		}
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		{
			// マウス座標を取得する。
			POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			// マウス座標を変換する。
			::MapWindowPoints(hwnd, g_settingDialog.m_hwnd, &point, 1);

			// 設定ダイアログにメッセージを転送する。
			return ::SendMessage(g_settingDialog.m_hwnd, message, wParam, MAKELPARAM(point.x, point.y));
		}
	case WM_MOUSEWHEEL:
		{
			MY_TRACE(_T("SettingDialog::containerWndProc(WM_MOUSEWHEEL, %d)\n"), wParam);

			g_settingDialog.scroll(SB_VERT, ((int)wParam > 0) ? SB_PAGEUP : SB_PAGEDOWN);
			g_settingDialog.recalcLayout();

			break;
		}
	}

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, SettingDialogProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
{
	switch (message)
	{
	case WM_GETMINMAXINFO:
		{
			MY_TRACE(_T("SettingDialogProc(WM_GETMINMAXINFO)\n"));

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMaxTrackSize.y *= 3;

			break;
		}
	case WM_SIZE:
		{
			MY_TRACE(_T("SettingDialogProc(WM_SIZE)\n"));

			// WM_SIZE ハンドラがブロックされているときは何もしない。
			if (g_settingDialog.m_blockSizeHandler) break;

			// 元のウィンドウ矩形のサイズを取得しておく。
			RECT rc; ::GetWindowRect(g_settingDialog.m_hwnd, &rc);
			g_settingDialog.m_rawWindowSize.cx = rc.right - rc.left;
			g_settingDialog.m_rawWindowSize.cy = rc.bottom - rc.top;

			g_settingDialog.updateScrollBar();
			g_settingDialog.recalcLayout();
			::InvalidateRect(g_settingDialog.m_hwndContainer, 0, FALSE);

			g_settingDialog.extend();

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

	return true_SettingDialogProc(hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
