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
			getPrivateProfileInt(element, L"borderSnapRange", g_borderSnapRange);
			getPrivateProfileColor(element, L"fillColor", g_fillColor);
			getPrivateProfileColor(element, L"borderColor", g_borderColor);
			getPrivateProfileColor(element, L"hotBorderColor", g_hotBorderColor);
			getPrivateProfileColor(element, L"activeCaptionColor", g_activeCaptionColor);
			getPrivateProfileColor(element, L"activeCaptionTextColor", g_activeCaptionTextColor);
			getPrivateProfileColor(element, L"inactiveCaptionColor", g_inactiveCaptionColor);
			getPrivateProfileColor(element, L"inactiveCaptionTextColor", g_inactiveCaptionTextColor);
			getPrivateProfileBool(element, L"useTheme", g_useTheme);
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

// <layout> を読み込む。
HRESULT loadLayout(const MSXML2::IXMLDOMElementPtr& element)
{
	MY_TRACE(_T("loadLayout()\n"));

	// <layout> を読み込む。
	MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"layout");
	int c = nodeList->length;
	for (int i = 0; i < c; i++)
	{
		MSXML2::IXMLDOMElementPtr layoutElement = nodeList->item[i];

		// <layout> のアトリビュートを読み込む。

		// layoutMode を取得する。
		getPrivateProfileLabel(layoutElement, L"layoutMode", g_layoutMode, g_layoutModeLabel);

		{
			// <window> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->selectNodes(L"window");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr windowElement = nodeList->item[i];

				// pos を取得する。
				int pos = WindowPos::maxSize;
				getPrivateProfileLabel(windowElement, L"pos", pos, g_windowPosLabel);
				if (pos == WindowPos::maxSize) continue;

				// id を取得する。
				getPrivateProfileLabel(windowElement, L"id", g_windowArray[pos], g_windowIdLabel);
			}
		}

		{
			// <vertSplit> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->selectNodes(L"vertSplit");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr vertSplitElement = nodeList->item[i];

				// <vertSplit> のアトリビュートを読み込む。

				getPrivateProfileInt(vertSplitElement, L"center", g_borders.m_vertCenter);
				getPrivateProfileInt(vertSplitElement, L"left", g_borders.m_vertLeft);
				getPrivateProfileInt(vertSplitElement, L"right", g_borders.m_vertRight);

				getPrivateProfileLabel(vertSplitElement, L"centerOrigin", g_borders.m_vertCenterOrigin, g_originLabel);
				getPrivateProfileLabel(vertSplitElement, L"leftOrigin", g_borders.m_vertLeftOrigin, g_originLabel);
				getPrivateProfileLabel(vertSplitElement, L"rightOrigin", g_borders.m_vertRightOrigin, g_originLabel);
			}
		}

		{
			// <horzSplit> を読み込む。
			MSXML2::IXMLDOMNodeListPtr nodeList = layoutElement->selectNodes(L"horzSplit");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr horzSplitElement = nodeList->item[i];

				// <horzSplit> のアトリビュートを読み込む。

				getPrivateProfileInt(horzSplitElement, L"center", g_borders.m_horzCenter);
				getPrivateProfileInt(horzSplitElement, L"top", g_borders.m_horzTop);
				getPrivateProfileInt(horzSplitElement, L"bottom", g_borders.m_horzBottom);

				getPrivateProfileLabel(horzSplitElement, L"centerOrigin", g_borders.m_horzCenterOrigin, g_originLabel);
				getPrivateProfileLabel(horzSplitElement, L"topOrigin", g_borders.m_horzTopOrigin, g_originLabel);
				getPrivateProfileLabel(horzSplitElement, L"bottomOrigin", g_borders.m_horzBottomOrigin, g_originLabel);
			}
		}
	}

	return S_OK;
}

//---------------------------------------------------------------------
