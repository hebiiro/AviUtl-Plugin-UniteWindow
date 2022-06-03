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
			setPrivateProfileInt(element, L"borderSnapRange", g_borderSnapRange);
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

// <layout> を作成する。
HRESULT saveLayout(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("saveLayout()\n"));

	// <layout> を作成する。
	MSXML2::IXMLDOMElementPtr layoutElement = appendElement(element, L"layout");

	setPrivateProfileLabel(layoutElement, L"layoutMode", g_layoutMode, g_layoutModeLabel);

	for (int i = 0; i < WindowPos::maxSize; i++)
	{
		// <window> を作成する。
		MSXML2::IXMLDOMElementPtr windowElement = appendElement(layoutElement, L"window");
		setPrivateProfileLabel(windowElement, L"pos", i, g_windowPosLabel);
		setPrivateProfileLabel(windowElement, L"id", g_windowArray[i], g_windowIdLabel);
	}

	// <vertSplit> を作成する。
	MSXML2::IXMLDOMElementPtr vertSplitElement = appendElement(layoutElement, L"vertSplit");
	setPrivateProfileInt(vertSplitElement, L"center", g_borders.m_vertCenter);
	setPrivateProfileInt(vertSplitElement, L"left", g_borders.m_vertLeft);
	setPrivateProfileInt(vertSplitElement, L"right", g_borders.m_vertRight);
	setPrivateProfileLabel(vertSplitElement, L"centerOrigin", g_borders.m_vertCenterOrigin, g_originLabel);
	setPrivateProfileLabel(vertSplitElement, L"leftOrigin", g_borders.m_vertLeftOrigin, g_originLabel);
	setPrivateProfileLabel(vertSplitElement, L"rightOrigin", g_borders.m_vertRightOrigin, g_originLabel);

	// <horzSplit> を作成する。
	MSXML2::IXMLDOMElementPtr horzSplitElement = appendElement(layoutElement, L"horzSplit");
	setPrivateProfileInt(horzSplitElement, L"center", g_borders.m_horzCenter);
	setPrivateProfileInt(horzSplitElement, L"top", g_borders.m_horzTop);
	setPrivateProfileInt(horzSplitElement, L"bottom", g_borders.m_horzBottom);
	setPrivateProfileLabel(horzSplitElement, L"centerOrigin", g_borders.m_horzCenterOrigin, g_originLabel);
	setPrivateProfileLabel(horzSplitElement, L"topOrigin", g_borders.m_horzTopOrigin, g_originLabel);
	setPrivateProfileLabel(horzSplitElement, L"bottomOrigin", g_borders.m_horzBottomOrigin, g_originLabel);

	return S_OK;
}

//---------------------------------------------------------------------
