#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

HRESULT loadConfig()
{
	MY_TRACE(_T("loadConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance, fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".xml");

	return loadConfig(fileName, FALSE);
}

HRESULT loadConfig(LPCWSTR fileName, BOOL _import)
{
	MY_TRACE(_T("loadConfig(%ws, %d)\n"), fileName, _import);

	try
	{
		// MSXML を使用する。
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		// 設定ファイルを開く。
		if (document->load(fileName) == VARIANT_FALSE)
		{
			MY_TRACE(_T("%s を開けませんでした\n"), fileName);

			return S_FALSE;
		}

		MSXML2::IXMLDOMElementPtr element = document->documentElement;

		if (!_import) // インポートのときはこれらの変数は取得しない。
		{
			getPrivateProfileInt(element, L"borderWidth", g_borderWidth);
			getPrivateProfileInt(element, L"captionHeight", g_captionHeight);
			getPrivateProfileColor(element, L"fillColor", g_fillColor);
			getPrivateProfileColor(element, L"borderColor", g_borderColor);
			getPrivateProfileColor(element, L"hotBorderColor", g_hotBorderColor);
		}

		// ウィンドウ位置を取得する。
		getPrivateProfileWindow(element, L"singleWindow", g_singleWindow);

		// <layout> を読み込む。
		loadLayout(element);

		MY_TRACE(_T("設定ファイルの読み込みに成功しました\n"));

		return S_OK;
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("設定ファイルの読み込みに失敗しました\n"));
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
		return e.Error();
	}
}

int stringToLayoutMode(LPCWSTR layoutModeString)
{
	if (::lstrcmpiW(layoutModeString, L"vertSplit") == 0) return LayoutMode::vertSplit;
	else if (::lstrcmpiW(layoutModeString, L"horzSplit") == 0) return LayoutMode::horzSplit;

	return LayoutMode::maxSize;
}

int stringToPos(LPCWSTR posString)
{
	if (::lstrcmpiW(posString, L"topLeft") == 0) return WindowPos::topLeft;
	else if (::lstrcmpiW(posString, L"topRight") == 0) return WindowPos::topRight;
	else if (::lstrcmpiW(posString, L"bottomLeft") == 0) return WindowPos::bottomLeft;
	else if (::lstrcmpiW(posString, L"bottomRight") == 0) return WindowPos::bottomRight;

	return WindowPos::maxSize;
}

Window* stringToWindow(LPCWSTR idString)
{
	if (::lstrcmpiW(idString, L"aviutlWindow") == 0) return &g_aviutlWindow;
	else if (::lstrcmpiW(idString, L"exeditWindow") == 0) return &g_exeditWindow;
	else if (::lstrcmpiW(idString, L"settingDialog") == 0) return &g_settingDialog;

	return 0;
}

// <layout> を読み込む。
HRESULT loadLayout(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("loadLayout()\n"));

	// <layout> を読み込む。
	MSXML2::IXMLDOMNodeListPtr nodeList = element->getElementsByTagName(L"layout");
	int c = nodeList->length;
	for (int i = 0; i < c; i++)
	{
		MSXML2::IXMLDOMElementPtr layoutElement = nodeList->item[i];

		// <layout> のアトリビュートを読み込む。

		// layoutMode を取得する。
		_bstr_t layoutModeString = L"";
		getPrivateProfileString(layoutElement, L"layoutMode", layoutModeString);
		int layoutMode = stringToLayoutMode(layoutModeString);
		if (layoutMode != LayoutMode::maxSize) g_layoutMode = layoutMode;

		{
			// <window> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->getElementsByTagName(L"window");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr windowElement = nodeList->item[i];

				// pos を取得する。
				_bstr_t posString = L"";
				getPrivateProfileString(windowElement, L"pos", posString);
				int pos = stringToPos(posString);
				if (pos == WindowPos::maxSize) continue;

				// id を取得する。
				_bstr_t idString = L"";
				getPrivateProfileString(windowElement, L"id", idString);
				Window* window = stringToWindow(idString);

				g_windowArray[pos] = window;
			}
		}

		{
			// <vertSplit> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->getElementsByTagName(L"vertSplit");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr vertSplitElement = nodeList->item[i];

				// <vertSplit> のアトリビュートを読み込む。

				getPrivateProfileInt(vertSplitElement, L"center", g_vertSplit.m_center);
				getPrivateProfileInt(vertSplitElement, L"left", g_vertSplit.m_left);
				getPrivateProfileInt(vertSplitElement, L"right", g_vertSplit.m_right);
			}
		}

		{
			// <horzSplit> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->getElementsByTagName(L"horzSplit");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr horzSplitElement = nodeList->item[i];

				// <horzSplit> のアトリビュートを読み込む。

				getPrivateProfileInt(horzSplitElement, L"center", g_horzSplit.m_center);
				getPrivateProfileInt(horzSplitElement, L"top", g_horzSplit.m_top);
				getPrivateProfileInt(horzSplitElement, L"bottom", g_horzSplit.m_bottom);
			}
		}
	}

	return S_OK;
}

//---------------------------------------------------------------------
