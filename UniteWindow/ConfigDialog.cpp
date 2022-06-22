#include "pch.h"
#include "ConfigDialog.h"

//---------------------------------------------------------------------

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
	ConfigDialog dialog(hwnd);

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
	::SetDlgItemInt(dialog, IDC_BORDER_VERT_CENTER, g_borders.m_vertCenter, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_VERT_LEFT, g_borders.m_vertLeft, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_VERT_RIGHT, g_borders.m_vertRight, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_HORZ_CENTER, g_borders.m_horzCenter, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_HORZ_TOP, g_borders.m_horzTop, TRUE);
	::SetDlgItemInt(dialog, IDC_BORDER_HORZ_BOTTOM, g_borders.m_horzBottom, TRUE);
	HWND hwndOrigin[6] = {};
	hwndOrigin[0] = ::GetDlgItem(dialog, IDC_BORDER_VERT_CENTER_ORIGIN);
	hwndOrigin[1] = ::GetDlgItem(dialog, IDC_BORDER_VERT_LEFT_ORIGIN);
	hwndOrigin[2] = ::GetDlgItem(dialog, IDC_BORDER_VERT_RIGHT_ORIGIN);
	hwndOrigin[3] = ::GetDlgItem(dialog, IDC_BORDER_HORZ_CENTER_ORIGIN);
	hwndOrigin[4] = ::GetDlgItem(dialog, IDC_BORDER_HORZ_TOP_ORIGIN);
	hwndOrigin[5] = ::GetDlgItem(dialog, IDC_BORDER_HORZ_BOTTOM_ORIGIN);
	for (int i = 0; i < 6; i++)
	{
		ComboBox_AddString(hwndOrigin[i], _T("左上基点"));
		ComboBox_AddString(hwndOrigin[i], _T("右下基点"));
	}
	ComboBox_SetCurSel(hwndOrigin[0], g_borders.m_vertCenterOrigin);
	ComboBox_SetCurSel(hwndOrigin[1], g_borders.m_vertLeftOrigin);
	ComboBox_SetCurSel(hwndOrigin[2], g_borders.m_vertRightOrigin);
	ComboBox_SetCurSel(hwndOrigin[3], g_borders.m_horzCenterOrigin);
	ComboBox_SetCurSel(hwndOrigin[4], g_borders.m_horzTopOrigin);
	ComboBox_SetCurSel(hwndOrigin[5], g_borders.m_horzBottomOrigin);
	::SetDlgItemInt(dialog, IDC_FILL_COLOR, g_fillColor, FALSE);
	::SetDlgItemInt(dialog, IDC_BORDER_COLOR, g_borderColor, FALSE);
	::SetDlgItemInt(dialog, IDC_HOT_BORDER_COLOR, g_hotBorderColor, FALSE);
	::SetDlgItemInt(dialog, IDC_ACTIVE_CAPTION_COLOR, g_activeCaptionColor, FALSE);
	::SetDlgItemInt(dialog, IDC_ACTIVE_CAPTION_TEXT_COLOR, g_activeCaptionTextColor, FALSE);
	::SetDlgItemInt(dialog, IDC_INACTIVE_CAPTION_COLOR, g_inactiveCaptionColor, FALSE);
	::SetDlgItemInt(dialog, IDC_INACTIVE_CAPTION_TEXT_COLOR, g_inactiveCaptionTextColor, FALSE);
	HWND hwndUseTheme = ::GetDlgItem(dialog, IDC_USE_THEME);
	Button_SetCheck(hwndUseTheme, g_useTheme);

	::EnableWindow(hwnd, FALSE);
	int retValue = dialog.doModal();
	::EnableWindow(hwnd, TRUE);
	::SetActiveWindow(hwnd);

	if (IDOK != retValue)
		return retValue;

	g_layoutMode = ComboBox_GetCurSel(hwndLayoutMode);
	for (int i = 0; i < WindowPos::maxSize; i++)
		g_windowArray[i] = getWindowFromComboBoxIndex(ComboBox_GetCurSel(hwndWindow[i]));
	g_borders.m_vertCenter = ::GetDlgItemInt(dialog, IDC_BORDER_VERT_CENTER, 0, TRUE);
	g_borders.m_vertLeft = ::GetDlgItemInt(dialog, IDC_BORDER_VERT_LEFT, 0, TRUE);
	g_borders.m_vertRight = ::GetDlgItemInt(dialog, IDC_BORDER_VERT_RIGHT, 0, TRUE);
	g_borders.m_horzCenter = ::GetDlgItemInt(dialog, IDC_BORDER_HORZ_CENTER, 0, TRUE);
	g_borders.m_horzTop = ::GetDlgItemInt(dialog, IDC_BORDER_HORZ_TOP, 0, TRUE);
	g_borders.m_horzBottom = ::GetDlgItemInt(dialog, IDC_BORDER_HORZ_BOTTOM, 0, TRUE);
	g_borders.m_vertCenterOrigin = ComboBox_GetCurSel(hwndOrigin[0]);
	g_borders.m_vertLeftOrigin = ComboBox_GetCurSel(hwndOrigin[1]);
	g_borders.m_vertRightOrigin = ComboBox_GetCurSel(hwndOrigin[2]);
	g_borders.m_horzCenterOrigin = ComboBox_GetCurSel(hwndOrigin[3]);
	g_borders.m_horzTopOrigin = ComboBox_GetCurSel(hwndOrigin[4]);
	g_borders.m_horzBottomOrigin = ComboBox_GetCurSel(hwndOrigin[5]);
	g_fillColor = ::GetDlgItemInt(dialog, IDC_FILL_COLOR, 0, FALSE);
	g_borderColor = ::GetDlgItemInt(dialog, IDC_BORDER_COLOR, 0, FALSE);
	g_hotBorderColor = ::GetDlgItemInt(dialog, IDC_HOT_BORDER_COLOR, 0, FALSE);
	g_activeCaptionColor = ::GetDlgItemInt(dialog, IDC_ACTIVE_CAPTION_COLOR, 0, FALSE);
	g_activeCaptionTextColor = ::GetDlgItemInt(dialog, IDC_ACTIVE_CAPTION_TEXT_COLOR, 0, FALSE);
	g_inactiveCaptionColor = ::GetDlgItemInt(dialog, IDC_INACTIVE_CAPTION_COLOR, 0, FALSE);
	g_inactiveCaptionTextColor = ::GetDlgItemInt(dialog, IDC_INACTIVE_CAPTION_TEXT_COLOR, 0, FALSE);
	g_useTheme = Button_GetCheck(hwndUseTheme);

	// レイアウトを再計算する。
	recalcLayout();

	// 再描画する。
	::InvalidateRect(hwnd, 0, FALSE);

	return retValue;
}

//---------------------------------------------------------------------

ConfigDialog::ConfigDialog(HWND hwnd)
	: Dialog(g_instance, MAKEINTRESOURCE(IDD_CONFIG), hwnd)
{
}

void ConfigDialog::onOK()
{
	Dialog::onOK();
}

void ConfigDialog::onCancel()
{
	Dialog::onCancel();
}

INT_PTR ConfigDialog::onDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		{
			UINT id = LOWORD(wParam);

			switch (id)
			{
			case IDC_FILL_COLOR:
			case IDC_BORDER_COLOR:
			case IDC_HOT_BORDER_COLOR:
			case IDC_ACTIVE_CAPTION_COLOR:
			case IDC_ACTIVE_CAPTION_TEXT_COLOR:
			case IDC_INACTIVE_CAPTION_COLOR:
			case IDC_INACTIVE_CAPTION_TEXT_COLOR:
				{
					HWND control = (HWND)lParam;

					COLORREF color = ::GetDlgItemInt(hwnd, id, 0, FALSE);

					static COLORREF customColors[16] = {};
					CHOOSECOLOR cc { sizeof(cc) };
					cc.hwndOwner = hwnd;
					cc.lpCustColors = customColors;
					cc.rgbResult = color;
					cc.Flags = CC_RGBINIT | CC_FULLOPEN;
					if (!::ChooseColor(&cc)) return TRUE;

					color = cc.rgbResult;

					::SetDlgItemInt(hwnd, id, color, FALSE);
					::InvalidateRect(control, 0, FALSE);

					return TRUE;
				}
			}

			break;
		}
	case WM_DRAWITEM:
		{
			UINT id = wParam;

			switch (id)
			{
			case IDC_FILL_COLOR:
			case IDC_BORDER_COLOR:
			case IDC_HOT_BORDER_COLOR:
			case IDC_ACTIVE_CAPTION_COLOR:
			case IDC_ACTIVE_CAPTION_TEXT_COLOR:
			case IDC_INACTIVE_CAPTION_COLOR:
			case IDC_INACTIVE_CAPTION_TEXT_COLOR:
				{
					DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;

					COLORREF color = ::GetDlgItemInt(hwnd, id, 0, FALSE);

					HBRUSH brush = ::CreateSolidBrush(color);
					FillRect(dis->hDC, &dis->rcItem, brush);
					::DeleteObject(brush);

					return TRUE;
				}
			}

			break;
		}
	}

	return Dialog::onDlgProc(hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
