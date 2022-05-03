#include "pch.h"
#include "UniteWindow.h"
#include "Resource.h"

//---------------------------------------------------------------------

// デバッグ用コールバック関数。デバッグメッセージを出力する
void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

//---------------------------------------------------------------------

AviUtlInternal g_auin;
HINSTANCE g_instance = 0;
HWND g_singleWindow = 0;
HTHEME g_theme = 0;
WNDPROC g_aviutlWindowProc = 0;
WNDPROC g_exeditWindowProc = 0;

AviUtlWindow g_aviutlWindow;
ExeditWindow g_exeditWindow;
SettingDialog g_settingDialog;

Window* g_windowArray[WindowPos::maxSize] =
{
	&g_aviutlWindow,
	&g_settingDialog,
	&g_exeditWindow,
	0,
};

int g_layoutMode = LayoutMode::horzSplit;
VertSplit g_vertSplit = {};
HorzSplit g_horzSplit = {};
int g_hotBorder = HotBorder::none;

int g_borderWidth = 8;
int g_captionHeight = 24;
COLORREF g_fillColor = RGB(0x55, 0x55, 0x55);
COLORREF g_borderColor = RGB(0x66, 0x66, 0x66);
COLORREF g_hotBorderColor = RGB(0x77, 0x77, 0x77);

POINT g_lastPos = {}; // ドラッグ処理に使う。

//---------------------------------------------------------------------

void initHook()
{
	MY_TRACE(_T("initHook()\n"));

	HMODULE user32 = ::GetModuleHandle(_T("user32.dll"));
	true_CreateWindowExA = (Type_CreateWindowExA)::GetProcAddress(user32, "CreateWindowExA");

	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	ATTACH_HOOK_PROC(CreateWindowExA);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("API フックに成功しました\n"));
	}
	else
	{
		MY_TRACE(_T("API フックに失敗しました\n"));
	}
}

void termHook()
{
	MY_TRACE(_T("termHook()\n"));
}

//---------------------------------------------------------------------

HWND createSingleWindow()
{
	MY_TRACE(_T("createSingleWindow()\n"));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc = singleWindowProc;
	wc.hInstance = g_instance;
	wc.lpszClassName = _T("UniteWindow");
	::RegisterClass(&wc);

	HWND hwnd = ::CreateWindowEx(
		0,
		_T("UniteWindow"),
		_T("UniteWindow"),
		WS_VISIBLE |
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, g_instance, 0);

	RECT rc; ::GetClientRect(hwnd, &rc);
	int cx = (rc.left + rc.right) / 2;
	int cy = (rc.top + rc.bottom) / 2;

	g_layoutMode = LayoutMode::horzSplit;

	g_vertSplit.m_center = cx;
	g_vertSplit.m_left = cy;
	g_vertSplit.m_right = cy;

	g_horzSplit.m_center = cy;
	g_horzSplit.m_top = cx;
	g_horzSplit.m_bottom = cx;

	return hwnd;
}

void normalizeLayoutVertSplit()
{
	MY_TRACE(_T("normalizeLayoutVertSplit()\n"));

	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	g_vertSplit.m_center = max(g_vertSplit.m_center, rc.left);
	g_vertSplit.m_center = min(g_vertSplit.m_center, rc.right - g_borderWidth);

	g_vertSplit.m_left = max(g_vertSplit.m_left, rc.top);
	g_vertSplit.m_left = min(g_vertSplit.m_left, rc.bottom - g_borderWidth);

	g_vertSplit.m_right = max(g_vertSplit.m_right, rc.top);
	g_vertSplit.m_right = min(g_vertSplit.m_right, rc.bottom - g_borderWidth);
}

void normalizeLayoutHorzSplit()
{
	MY_TRACE(_T("normalizeLayoutHorzSplit()\n"));

	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	g_horzSplit.m_center = max(g_horzSplit.m_center, rc.top);
	g_horzSplit.m_center = min(g_horzSplit.m_center, rc.bottom - g_borderWidth);

	g_horzSplit.m_top = max(g_horzSplit.m_top, rc.left);
	g_horzSplit.m_top = min(g_horzSplit.m_top, rc.right - g_borderWidth);

	g_horzSplit.m_bottom = max(g_horzSplit.m_bottom, rc.left);
	g_horzSplit.m_bottom = min(g_horzSplit.m_bottom, rc.right - g_borderWidth);
}

void normalizeLayout()
{
	MY_TRACE(_T("normalizeLayout()\n"));

	switch (g_layoutMode)
	{
	case LayoutMode::vertSplit:
		{
			normalizeLayoutVertSplit();

			break;
		}
	case LayoutMode::horzSplit:
		{
			normalizeLayoutHorzSplit();

			break;
		}
	}
}

void recalcLayoutVertSplit()
{
	MY_TRACE(_T("recalcLayoutVertSplit()\n"));

	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	if (g_windowArray[WindowPos::topLeft])
	{
		RECT rc2 =
		{
			rc.left,
			rc.top + g_captionHeight,
			g_vertSplit.m_center,
			g_vertSplit.m_left,
		};

		g_windowArray[WindowPos::topLeft]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::topRight])
	{
		RECT rc2 =
		{
			g_vertSplit.m_center + g_borderWidth,
			rc.top + g_captionHeight,
			rc.right,
			g_vertSplit.m_right,
		};

		g_windowArray[WindowPos::topRight]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::bottomLeft])
	{
		RECT rc2 =
		{
			rc.left,
			g_vertSplit.m_left + g_borderWidth + g_captionHeight,
			g_vertSplit.m_center,
			rc.bottom,
		};

		g_windowArray[WindowPos::bottomLeft]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::bottomRight])
	{
		RECT rc2 =
		{
			g_vertSplit.m_center + g_borderWidth,
			g_vertSplit.m_right + g_borderWidth + g_captionHeight,
			rc.right,
			rc.bottom,
		};

		g_windowArray[WindowPos::bottomRight]->resize(&rc2);
	}
}

void recalcLayoutHorzSplit()
{
	MY_TRACE(_T("recalcLayoutHorzSplit()\n"));

	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	if (g_windowArray[WindowPos::topLeft])
	{
		RECT rc2 =
		{
			rc.left,
			rc.top + g_captionHeight,
			g_horzSplit.m_top,
			g_horzSplit.m_center,
		};

		g_windowArray[WindowPos::topLeft]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::topRight])
	{
		RECT rc2 =
		{
			g_horzSplit.m_top + g_borderWidth,
			rc.top + g_captionHeight,
			rc.right,
			g_horzSplit.m_center,
		};

		g_windowArray[WindowPos::topRight]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::bottomLeft])
	{
		RECT rc2 =
		{
			rc.left,
			g_horzSplit.m_center + g_borderWidth + g_captionHeight,
			g_horzSplit.m_bottom,
			rc.bottom,
		};

		g_windowArray[WindowPos::bottomLeft]->resize(&rc2);
	}

	if (g_windowArray[WindowPos::bottomRight])
	{
		RECT rc2 =
		{
			g_horzSplit.m_bottom + g_borderWidth,
			g_horzSplit.m_center + g_borderWidth + g_captionHeight,
			rc.right,
			rc.bottom,
		};

		g_windowArray[WindowPos::bottomRight]->resize(&rc2);
	}
}

void recalcLayout()
{
	MY_TRACE(_T("recalcLayout()\n"));

	if (::IsIconic(g_singleWindow))
		return;

	switch (g_layoutMode)
	{
	case LayoutMode::vertSplit:
		{
			normalizeLayoutVertSplit();
			recalcLayoutVertSplit();

			break;
		}
	case LayoutMode::horzSplit:
		{
			normalizeLayoutHorzSplit();
			recalcLayoutHorzSplit();

			break;
		}
	}
}

int hitTestVertSplit(POINT point)
{
	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	if (point.x < g_vertSplit.m_center)
	{
		if (point.y >= g_vertSplit.m_left && point.y < g_vertSplit.m_left + g_borderWidth)
			return HotBorder::left;
	}
	else if (point.x < g_vertSplit.m_center + g_borderWidth)
	{
		return HotBorder::vertCenter;
	}
	else
	{
		if (point.y >= g_vertSplit.m_right && point.y < g_vertSplit.m_right + g_borderWidth)
			return HotBorder::right;
	}

	return HotBorder::none;
}

int hitTestHorzSplit(POINT point)
{
	RECT rc; ::GetClientRect(g_singleWindow, &rc);

	if (point.y < g_horzSplit.m_center)
	{
		if (point.x >= g_horzSplit.m_top && point.x < g_horzSplit.m_top + g_borderWidth)
			return HotBorder::top;
	}
	else if (point.y < g_horzSplit.m_center + g_borderWidth)
	{
		return HotBorder::horzCenter;
	}
	else
	{
		if (point.x >= g_horzSplit.m_bottom && point.x < g_horzSplit.m_bottom + g_borderWidth)
			return HotBorder::bottom;
	}

	return HotBorder::none;
}

int hitTest(POINT point)
{
	switch (g_layoutMode)
	{
	case LayoutMode::vertSplit: return hitTestVertSplit(point);
	case LayoutMode::horzSplit: return hitTestHorzSplit(point);
	}

	return HotBorder::none;
}

void dragBorder(POINT offset)
{
	switch (g_hotBorder)
	{
	case HotBorder::horzCenter:	g_horzSplit.m_center += offset.y; break;
	case HotBorder::top:		g_horzSplit.m_top += offset.x; break;
	case HotBorder::bottom:		g_horzSplit.m_bottom += offset.x; break;
	case HotBorder::vertCenter:	g_vertSplit.m_center += offset.x; break;
	case HotBorder::left:		g_vertSplit.m_left += offset.y; break;
	case HotBorder::right:		g_vertSplit.m_right += offset.y; break;
	}
}

BOOL getHotBorderRect(LPRECT rc)
{
	RECT rcClient; ::GetClientRect(g_singleWindow, &rcClient);

	switch (g_hotBorder)
	{
	case HotBorder::horzCenter:
		{
			rc->left = rcClient.left;
			rc->top = g_horzSplit.m_center;
			rc->right = rcClient.right;
			rc->bottom = g_horzSplit.m_center + g_borderWidth;

			return TRUE;
		}
	case HotBorder::top:
		{
			rc->left = g_horzSplit.m_top;
			rc->top = rcClient.top;
			rc->right = g_horzSplit.m_top + g_borderWidth;
			rc->bottom = g_horzSplit.m_center;

			return TRUE;
		}
	case HotBorder::bottom:
		{
			rc->left = g_horzSplit.m_bottom;
			rc->top = g_horzSplit.m_center + g_borderWidth;
			rc->right = g_horzSplit.m_bottom + g_borderWidth;
			rc->bottom = rcClient.bottom;

			return TRUE;
		}
	case HotBorder::vertCenter:
		{
			rc->left = g_vertSplit.m_center;
			rc->top = rcClient.top;
			rc->right = g_vertSplit.m_center + g_borderWidth;
			rc->bottom = rcClient.bottom;

			return TRUE;
		}
	case HotBorder::left:
		{
			rc->left = rcClient.left;
			rc->top = g_vertSplit.m_left;
			rc->right = g_vertSplit.m_center;
			rc->bottom = g_vertSplit.m_left + g_borderWidth;

			return TRUE;
		}
	case HotBorder::right:
		{
			rc->left = g_vertSplit.m_center + g_borderWidth;
			rc->top = g_vertSplit.m_right;
			rc->right = rcClient.right;
			rc->bottom = g_vertSplit.m_right + g_borderWidth;

			return TRUE;
		}
	}

	return FALSE;
}

void drawCaption(HDC dc, HWND hwnd, Window* window)
{
	RECT rc; ::GetWindowRect(window->m_hwndContainer, &rc);
	::MapWindowPoints(0, hwnd, (POINT*)&rc, 2);
	rc.bottom = rc.top - 1;
	rc.top = rc.top - g_captionHeight;

	WCHAR text[MAX_PATH] = {};
	::GetWindowTextW(window->m_hwnd, text, MAX_PATH);

	::DrawThemeBackground(g_theme, dc, MENU_BARBACKGROUND, MBI_NORMAL, &rc, 0);
	::DrawThemeText(g_theme, dc, MENU_BARITEM, MBI_NORMAL,
		text, ::lstrlenW(text), DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, &rc);
}

//---------------------------------------------------------------------

BOOL isDescendants(HWND hwnd, HWND target)
{
	MY_TRACE(_T("isDescendants(0x%08X, 0x%08X)\n"), hwnd, target);

	while (target)
	{
		if (target == hwnd)
			return TRUE;

		target = (HWND)::GetWindowLongPtr(target, GWLP_HWNDPARENT);
//		target = ::GetParent(target);
//		target = ::GetWindow(target, GW_OWNER);
		MY_TRACE_HWND(target);
	}

	return FALSE;
}

int getComboBoxIndexFromWindow(Window* window)
{
	if (window == &g_aviutlWindow) return 0;
	else if (window == &g_exeditWindow) return 1;
	else if (window == &g_settingDialog) return 2;

	return 3;
}

Window* getWindowFromComboBoxIndex(int i)
{
	switch (i)
	{
	case 0: return &g_aviutlWindow;
	case 1: return &g_exeditWindow;
	case 2: return &g_settingDialog;
	}

	return 0;
}

int showConfigDialog(HWND hwnd)
{
	Dialog dialog(g_instance, MAKEINTRESOURCE(IDD_CONFIG), hwnd);

	HWND hwndLayoutMode = ::GetDlgItem(dialog, IDC_LAYOUT_MODE);
	ComboBox_AddString(hwndLayoutMode, _T("垂直分割"));
	ComboBox_AddString(hwndLayoutMode, _T("水平分割"));
	ComboBox_SetCurSel(hwndLayoutMode, g_layoutMode);

	HWND hwndWindow[WindowPos::maxSize] = {};
	hwndWindow[WindowPos::topLeft] = ::GetDlgItem(dialog, IDC_WINDOW_TOP_LEFT);
	hwndWindow[WindowPos::topRight] = ::GetDlgItem(dialog, IDC_WINDOW_TOP_RIGHT);
	hwndWindow[WindowPos::bottomLeft] = ::GetDlgItem(dialog, IDC_WINDOW_BOTTOM_LEFT);
	hwndWindow[WindowPos::bottomRight] = ::GetDlgItem(dialog, IDC_WINDOW_BOTTOM_RIGHT);
	for (int i = 0; i < WindowPos::maxSize; i++)
	{
		ComboBox_AddString(hwndWindow[i], _T("AviUtlウィンドウ"));
		ComboBox_AddString(hwndWindow[i], _T("拡張編集ウィンドウ"));
		ComboBox_AddString(hwndWindow[i], _T("設定ダイアログ"));
		ComboBox_AddString(hwndWindow[i], _T("ウィンドウなし"));
		ComboBox_SetCurSel(hwndWindow[i], getComboBoxIndexFromWindow(g_windowArray[i]));
	}
	::SetDlgItemInt(dialog, IDC_BORDER_VERT_CENTER, g_vertSplit.m_center, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_LEFT, g_vertSplit.m_left, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_RIGHT, g_vertSplit.m_right, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_HORZ_CENTER, g_horzSplit.m_center, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_TOP, g_horzSplit.m_top, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_BOTTOM, g_horzSplit.m_bottom, TRUE);

	int retValue = dialog.doModal();

	if (IDOK != retValue)
		return retValue;

	g_layoutMode = ComboBox_GetCurSel(hwndLayoutMode);
	for (int i = 0; i < WindowPos::maxSize; i++)
		g_windowArray[i] = getWindowFromComboBoxIndex(ComboBox_GetCurSel(hwndWindow[i]));
	g_vertSplit.m_center = ::GetDlgItemInt(dialog, IDC_BORDER_VERT_CENTER, 0, TRUE);
	g_vertSplit.m_left = ::GetDlgItemInt(dialog, IDC_BORDER_LEFT, 0, TRUE);
	g_vertSplit.m_right = ::GetDlgItemInt(dialog, IDC_BORDER_RIGHT, 0, TRUE);
	g_horzSplit.m_center = ::GetDlgItemInt(dialog, IDC_BORDER_HORZ_CENTER, 0, TRUE);
	g_horzSplit.m_top = ::GetDlgItemInt(dialog, IDC_BORDER_TOP, 0, TRUE);
	g_horzSplit.m_bottom = ::GetDlgItemInt(dialog, IDC_BORDER_BOTTOM, 0, TRUE);

	// レイアウトを再計算する。
	recalcLayout();

	// 再描画する。
	::InvalidateRect(hwnd, 0, FALSE);

	return retValue;
}

BOOL importLayout(HWND hwnd)
{
	// ファイル選択ダイアログを表示してファイル名を取得する。

	WCHAR fileName[MAX_PATH] = {};

	WCHAR folderName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance, folderName, MAX_PATH);
	::PathRemoveExtensionW(folderName);

	OPENFILENAMEW ofn = { sizeof(ofn) };
	ofn.hwndOwner = hwnd;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = L"レイアウトのインポート";
	ofn.lpstrInitialDir = folderName;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"レイアウトファイル (*.xml)\0*.xml\0" "すべてのファイル (*.*)\0*.*\0";
	ofn.lpstrDefExt = L"xml";

	if (!::GetOpenFileNameW(&ofn))
		return FALSE;

	// レイアウトファイルをインポートする。
	loadConfig(fileName, TRUE);

	// レイアウトを再計算する。
	recalcLayout();

	// 再描画する。
	::InvalidateRect(hwnd, 0, FALSE);

	return TRUE;
}

BOOL exportLayout(HWND hwnd)
{
	// ファイル選択ダイアログを表示してファイル名を取得する。

	WCHAR fileName[MAX_PATH] = {};

	WCHAR folderName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance, folderName, MAX_PATH);
	::PathRemoveExtensionW(folderName);

	OPENFILENAMEW ofn = { sizeof(ofn) };
	ofn.hwndOwner = hwnd;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = L"レイアウトのエクスポート";
	ofn.lpstrInitialDir = folderName;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"レイアウトファイル (*.xml)\0*.xml\0" "すべてのファイル (*.*)\0*.*\0";
	ofn.lpstrDefExt = L"xml";

	if (!::GetSaveFileNameW(&ofn))
		return FALSE;

	// レイアウトファイルをエクスポートする。
	saveConfig(fileName, TRUE);

	return TRUE;
}

LRESULT CALLBACK singleWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSCOMMAND:
		{
			MY_TRACE(_T("singleWindowProc(WM_SYSCOMMAND)\n"));

			switch (wParam)
			{
			case CommandID::ShowConfigDialog:
				{
					// UniteWindow の設定ダイアログを開く。
					showConfigDialog(hwnd);

					break;
				}
			case CommandID::ImportLayout:
				{
					// レイアウトファイルをインポートする。
					importLayout(hwnd);

					break;
				}
			case CommandID::ExportLayout:
				{
					// レイアウトファイルをエクスポートする。
					exportLayout(hwnd);

					break;
				}
			}

			break;
		}
	case WM_CREATE:
		{
			MY_TRACE(_T("singleWindowProc(WM_CREATE)\n"));

			UINT dpi = ::GetDpiForWindow(hwnd);
			g_theme = ::OpenThemeDataForDpi(hwnd, VSCLASS_MENU, dpi);
//			g_theme = ::OpenThemeData(hwnd, VSCLASS_MENU);
			MY_TRACE_HEX(g_theme);

			HMENU menu = ::GetSystemMenu(hwnd, FALSE);
			::InsertMenu(menu, 0, MF_BYPOSITION | MF_STRING, CommandID::ImportLayout, _T("レイアウトのインポート"));
			::InsertMenu(menu, 1, MF_BYPOSITION | MF_STRING, CommandID::ExportLayout, _T("レイアウトのエクスポート"));
			::InsertMenu(menu, 2, MF_BYPOSITION | MF_STRING, CommandID::ShowConfigDialog, _T("UniteWindowの設定"));
			::InsertMenu(menu, 3, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

			break;
		}
	case WM_DESTROY:
		{
			MY_TRACE(_T("singleWindowProc(WM_DESTROY)\n"));

			::CloseThemeData(g_theme), g_theme = 0;

			break;
		}
	case WM_CLOSE:
		{
			MY_TRACE(_T("singleWindowProc(WM_CLOSE)\n"));

			return ::SendMessage(g_aviutlWindow.m_hwnd, message, wParam, lParam);
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = ::BeginPaint(hwnd, &ps);

			{
				HBRUSH brush = ::CreateSolidBrush(g_fillColor);
				FillRect(dc, &ps.rcPaint, brush);
				::DeleteObject(brush);
			}

			{
				// ホットボーダーを描画する。
				RECT rcHotBorder;
				if (getHotBorderRect(&rcHotBorder))
				{
					HBRUSH brush = ::CreateSolidBrush(g_hotBorderColor);
					::FillRect(dc, &rcHotBorder, brush);
					::DeleteObject(brush);
				}
			}

			{
				// 各ウィンドウのキャプションを描画する。

				LOGFONTW lf = {};
				::GetThemeSysFont(g_theme, TMT_CAPTIONFONT, &lf);
				HFONT font = ::CreateFontIndirectW(&lf);
				HFONT oldFont = (HFONT)::SelectObject(dc, font);

				drawCaption(dc, hwnd, &g_aviutlWindow);
				drawCaption(dc, hwnd, &g_exeditWindow);
				drawCaption(dc, hwnd, &g_settingDialog);

				::SelectObject(dc, oldFont);
				::DeleteObject(font);
			}

			EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_NCACTIVATE:
		{
			MY_TRACE(_T("singleWindowProc(WM_NCACTIVATE, %d, 0x%08X)\n"), wParam, lParam);

			if (isDescendants(hwnd, (HWND)lParam))
				return ::DefWindowProc(hwnd, message, TRUE, 0);

			break;
		}
	case WM_SIZE:
		{
			recalcLayout();

			break;
		}
	case WM_SETCURSOR:
		{
			if (hwnd == (HWND)wParam)
			{
				POINT point; ::GetCursorPos(&point);
				::ScreenToClient(hwnd, &point);

				int hotBorder = hitTest(point);

				switch (hotBorder)
				{
				case HotBorder::vertCenter:
				case HotBorder::top:
				case HotBorder::bottom:
					{
						::SetCursor(::LoadCursor(0, IDC_SIZEWE));

						return TRUE;
					}
				case HotBorder::horzCenter:
				case HotBorder::left:
				case HotBorder::right:
					{
						::SetCursor(::LoadCursor(0, IDC_SIZENS));

						return TRUE;
					}
				}
			}

			break;
		}
	case WM_LBUTTONDOWN:
		{
			MY_TRACE(_T("singleWindowProc(WM_LBUTTONDOWN)\n"));

			// マウス座標を取得する。
			POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			// マウス座標にあるボーダーを取得する。
			g_hotBorder = hitTest(point);

			// ボーダーが有効かチェックする。
			if (g_hotBorder != HotBorder::none)
			{
				// 最後のマウス座標を記憶しておく。
				g_lastPos = point;

				// マウスキャプチャを開始する。
				::SetCapture(hwnd);

				// 再描画する。
				::InvalidateRect(hwnd, 0, FALSE);
			}

			break;
		}
	case WM_LBUTTONUP:
		{
			MY_TRACE(_T("singleWindowProc(WM_LBUTTONUP)\n"));

			// マウス座標を取得する。
			POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			// マウスをキャプチャ中かチェックする。
			if (::GetCapture() == hwnd)
			{
				// マウスキャプチャを終了する。
				::ReleaseCapture();

				POINT offset =
				{
					point.x - g_lastPos.x,
					point.y - g_lastPos.y,
				};

				// ボーダーを動かす。
				dragBorder(offset);

				// レイアウトを再計算する。
				recalcLayout();

				// 再描画する。
				::InvalidateRect(hwnd, 0, FALSE);
			}

			break;
		}
	case WM_MOUSEMOVE:
		{
			// マウス座標を取得する。
			POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			// マウスをキャプチャ中かチェックする。
			if (::GetCapture() == hwnd)
			{
				POINT offset =
				{
					point.x - g_lastPos.x,
					point.y - g_lastPos.y,
				};

				// ボーダーを動かす。
				dragBorder(offset);

				// レイアウトを再計算する。
				recalcLayout();

				// 再描画する。
				::InvalidateRect(hwnd, 0, FALSE);

				// 最後のマウス座標を更新する。
				g_lastPos = point;
			}
			else
			{
				// マウス座標にあるボーダーを取得する。
				int hotBorder = hitTest(point);

				// ホットボーダーと別のボーダーかチェックする。
				if (g_hotBorder != hotBorder)
				{
					// ホットボーダーを更新する。
					g_hotBorder = hotBorder;

					// 再描画する。
					::InvalidateRect(hwnd, 0, FALSE);
				}

				// マウスリーブイベントをトラックする。
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				::TrackMouseEvent(&tme);
			}

			break;
		}
	case WM_MOUSELEAVE:
		{
			MY_TRACE(_T("singleWindowProc(WM_MOUSELEAVE)\n"));

			// 無効なボーダーを取得する。
			int hotBorder = HotBorder::none;

			// ホットボーダーと別のボーダーかチェックする。
			if (g_hotBorder != hotBorder)
			{
				// ホットボーダーを更新する。
				g_hotBorder = hotBorder;

				// 再描画する。
				::InvalidateRect(hwnd, 0, FALSE);
			}

			break;
		}
	}

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------

IMPLEMENT_HOOK_PROC_NULL(HWND, WINAPI, CreateWindowExA, (DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param))
{
	if (::lstrcmpiA(className, "AviUtl") == 0 ||
		::lstrcmpiA(className, "ExtendedFilterClass") == 0)
	{
		MY_TRACE(_T("CreateWindowExA(%hs, %hs)\n"), className, windowName);
	}

	if (::lstrcmpiA(windowName, "AviUtl") == 0)
	{
		g_singleWindow = createSingleWindow();

		loadConfig();

//		parent = g_singleWindow;
	}

	HWND hwnd = true_CreateWindowExA(exStyle, className, windowName, style, x, y, w, h, parent, menu, instance, param);

	if (::lstrcmpiA(windowName, "AviUtl") == 0)
	{
		g_aviutlWindow.init(hwnd);
	}
	else if (::lstrcmpiA(windowName, "拡張編集") == 0)
	{
		g_auin.init();

		true_SettingDialogProc = g_auin.HookSettingDialogProc(hook_SettingDialogProc);
		MY_TRACE_HEX(true_SettingDialogProc);
		MY_TRACE_HEX(hook_SettingDialogProc);

		g_exeditWindow.init(hwnd);
	}
	else if (::lstrcmpiA(windowName, "ExtendedFilter") == 0)
	{
		g_settingDialog.init(hwnd);

		recalcLayout();

		::SetForegroundWindow(g_singleWindow);
	}

	return hwnd;
}

//---------------------------------------------------------------------

EXTERN_C BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			// ロケールを設定する。
			// これをやらないと日本語テキストが文字化けするので最初に実行する。
			_tsetlocale(LC_ALL, _T(""));

			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			// この DLL のハンドルをグローバル変数に保存しておく。
			g_instance = instance;
			MY_TRACE_HEX(g_instance);

			// この DLL の参照カウンタを増やしておく。
			WCHAR moduleFileName[MAX_PATH] = {};
			::GetModuleFileNameW(g_instance, moduleFileName, MAX_PATH);
			::LoadLibraryW(moduleFileName);

			initHook();

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			MY_TRACE(_T("DLL_PROCESS_DETACH\n"));

			termHook();

			break;
		}
	}

	return TRUE;
}

//---------------------------------------------------------------------
