#include "pch.h"
#include "UniteWindow.h"

//---------------------------------------------------------------------

HRESULT saveConfig()
{
	MY_TRACE(_T("saveConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance, fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".xml");

	return saveConfig(fileName, FALSE);
}

HRESULT saveConfig(LPCWSTR fileName, BOOL _export)
{
	MY_TRACE(_T("saveConfig(%ws, %d)\n"), fileName, _export);

	try
	{
		// ドキュメントを作成する。
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		// ドキュメントエレメントを作成する。
		MSXML2::IXMLDOMElementPtr element = appendElement(document, document, L"config");

		if (!_export) // エクスポートのときはこれらの変数は保存しない。
		{
			setPrivateProfileInt(element, L"borderWidth", g_borderWidth);
			setPrivateProfileInt(element, L"captionHeight", g_captionHeight);
			setPrivateProfileColor(element, L"fillColor", g_fillColor);
			setPrivateProfileColor(element, L"borderColor", g_borderColor);
			setPrivateProfileColor(element, L"hotBorderColor", g_hotBorderColor);
		}

		// ウィンドウ位置を保存する。
		setPrivateProfileWindow(element, L"singleWindow", g_singleWindow);

		// <layout> を作成する。
		saveLayout(element);

		return saveXMLDocument(document, fileName, L"UTF-16");
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
		return e.Error();
	}
}

LPCWSTR layoutModeToString(int value)
{
	switch (value)
	{
	case LayoutMode::vertSplit: return L"vertSplit";
	case LayoutMode::horzSplit: return L"horzSplit";
	}

	return L"";
}

LPCWSTR posToString(int i)
{
	switch (i)
	{
	case WindowPos::topLeft: return L"topLeft";
	case WindowPos::topRight: return L"topRight";
	case WindowPos::bottomLeft: return L"bottomLeft";
	case WindowPos::bottomRight: return L"bottomRight";
	}

	return L"";
}

LPCWSTR idToString(int i)
{
	Window* window = g_windowArray[i];

	if (window == &g_aviutlWindow) return L"aviutlWindow";
	else if (window == &g_exeditWindow) return L"exeditWindow";
	else if (window == &g_settingDialog) return L"settingDialog";

	return L"";
}

// <layout> を作成する。
HRESULT saveLayout(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("saveLayout()\n"));

	// <layout> を作成する。
	MSXML2::IXMLDOMElementPtr layoutElement = appendElement(element, L"layout");

	setPrivateProfileString(layoutElement, L"layoutMode", layoutModeToString(g_layoutMode));

	for (int i = 0; i < WindowPos::maxSize; i++)
	{
		LPCWSTR pos = posToString(i);
		LPCWSTR id = idToString(i);

		// <window> を作成する。
		MSXML2::IXMLDOMElementPtr windowElement = appendElement(layoutElement, L"window");
		setPrivateProfileString(windowElement, L"pos", pos);
		setPrivateProfileString(windowElement, L"id", id);
	}

	// <vertSplit> を作成する。
	MSXML2::IXMLDOMElementPtr vertSplitElement = appendElement(layoutElement, L"vertSplit");
	setPrivateProfileInt(vertSplitElement, L"center", g_vertSplit.m_center);
	setPrivateProfileInt(vertSplitElement, L"left", g_vertSplit.m_left);
	setPrivateProfileInt(vertSplitElement, L"right", g_vertSplit.m_right);

	// <horzSplit> を作成する。
	MSXML2::IXMLDOMElementPtr horzSplitElement = appendElement(layoutElement, L"horzSplit");
	setPrivateProfileInt(horzSplitElement, L"center", g_horzSplit.m_center);
	setPrivateProfileInt(horzSplitElement, L"top", g_horzSplit.m_top);
	setPrivateProfileInt(horzSplitElement, L"bottom", g_horzSplit.m_bottom);

	return S_OK;
}

//---------------------------------------------------------------------
