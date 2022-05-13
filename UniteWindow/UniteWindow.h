#pragma once

//---------------------------------------------------------------------

class Window
{
public:
	HWND m_hwnd = 0;
	HWND m_hwndContainer = 0;
public:
	virtual void init(HWND hwnd) = 0;
	static HWND getWindow(HWND hwndContainer);
	static void setWindow(HWND hwndContainer, HWND hwnd);
	static HWND createContainerWindow(HWND child, WNDPROC wndProc, LPCTSTR className);
	void resize(LPCRECT rc);
};

class AviUtlWindow : public Window
{
public:
	virtual void init(HWND hwnd);
	static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class ExeditWindow : public Window
{
public:
	virtual void init(HWND hwnd);
	static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class SettingDialog : public Window
{
public:
	virtual void init(HWND hwnd);
	void updateScrollBar();
	void scroll(int bar, WPARAM wParam);
	void recalcLayout();
	static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

struct WindowPos
{
	static const int topLeft = 0;
	static const int topRight = 1;
	static const int bottomLeft = 2;
	static const int bottomRight = 3;

	static const int maxSize = 4;
};

struct
{
	int value;
	LPCWSTR label;

} const g_windowPosLabel[] =
{
	{ WindowPos::topLeft, L"topLeft" },
	{ WindowPos::topRight, L"topRight" },
	{ WindowPos::bottomLeft, L"bottomLeft" },
	{ WindowPos::bottomRight, L"bottomRight" },
};

struct LayoutMode
{
	static const int vertSplit = 0;
	static const int horzSplit = 1;

	static const int maxSize = 2;
};

struct
{
	int value;
	LPCWSTR label;

} const g_layoutModeLabel[] =
{
	{ LayoutMode::vertSplit, L"vertSplit" },
	{ LayoutMode::horzSplit, L"horzSplit" },
};

struct Origin
{
	static const int topLeft = 0;
	static const int bottomRight = 1;
};

struct
{
	int value;
	LPCWSTR label;

} const g_originLabel[] =
{
	{ Origin::topLeft, L"topLeft" },
	{ Origin::bottomRight, L"bottomRight" },
};

struct Borders
{
	int m_vertCenter;
	int m_vertLeft;
	int m_vertRight;

	int m_vertCenterOrigin;
	int m_vertLeftOrigin;
	int m_vertRightOrigin;

	int m_horzCenter;
	int m_horzTop;
	int m_horzBottom;

	int m_horzCenterOrigin;
	int m_horzTopOrigin;
	int m_horzBottomOrigin;
};

struct HotBorder
{
	static const int none = 0;

	static const int vertCenter = 1;
	static const int vertLeft = 2;
	static const int vertRight = 3;

	static const int horzCenter = 4;
	static const int horzTop = 5;
	static const int horzBottom = 6;
};

struct CommandID
{
	static const int ShowConfigDialog = 1000;
	static const int ImportLayout = 1001;
	static const int ExportLayout = 1002;
};

struct WindowMessage
{
	static const UINT WM_POST_INIT = WM_APP + 1;
};

//---------------------------------------------------------------------

extern AviUtlInternal g_auin;
extern HINSTANCE g_instance;
extern HWND g_singleWindow;
extern HTHEME g_theme;
extern WNDPROC g_aviutlWindowProc;
extern WNDPROC g_exeditWindowProc;

extern AviUtlWindow g_aviutlWindow;
extern ExeditWindow g_exeditWindow;
extern SettingDialog g_settingDialog;

extern Window* g_windowArray[WindowPos::maxSize];
extern int g_layoutMode;
extern Borders g_borders;
extern int g_hotBorder;

extern int g_borderWidth;
extern int g_captionHeight;
extern int g_borderSnapRange;
extern COLORREF g_fillColor;
extern COLORREF g_borderColor;
extern COLORREF g_hotBorderColor;
extern COLORREF g_activeCaptionColor;
extern COLORREF g_activeCaptionTextColor;
extern COLORREF g_inactiveCaptionColor;
extern COLORREF g_inactiveCaptionTextColor;

//---------------------------------------------------------------------

struct
{
	Window* value;
	LPCWSTR label;

} const g_windowIdLabel[] =
{
	{ &g_aviutlWindow, L"aviutlWindow" },
	{ &g_exeditWindow, L"exeditWindow" },
	{ &g_settingDialog, L"settingDialog" },
	{ 0, L"" },
};

//---------------------------------------------------------------------

HWND createSingleWindow();
void normalizeLayoutHorzSplit();
void normalizeLayoutVertSplit();
void recalcLayoutHorzSplit();
void recalcLayoutVertSplit();
void recalcLayout();
LRESULT CALLBACK singleWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK aviutlWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK exeditWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT loadConfig();
HRESULT loadConfig(LPCWSTR fileName, BOOL _import);
HRESULT loadLayout(const MSXML2::IXMLDOMElementPtr& element);

HRESULT saveConfig();
HRESULT saveConfig(LPCWSTR fileName, BOOL _export);
HRESULT saveLayout(const MSXML2::IXMLDOMElementPtr& element);

//---------------------------------------------------------------------

DECLARE_HOOK_PROC(LRESULT, WINAPI, SettingDialogProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
DECLARE_HOOK_PROC(HWND, WINAPI, CreateWindowExA, (DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param));
DECLARE_HOOK_PROC(HMENU, WINAPI, GetMenu, (HWND hwnd));
DECLARE_HOOK_PROC(BOOL, WINAPI, SetMenu, (HWND hwnd, HMENU menu));
DECLARE_HOOK_PROC(BOOL, WINAPI, DrawMenuBar, (HWND hwnd));
DECLARE_HOOK_PROC(HWND, WINAPI, FindWindowExA, (HWND parent, HWND childAfter, LPCSTR className, LPCSTR windowName));
DECLARE_HOOK_PROC(HWND, WINAPI, FindWindowW, (LPCWSTR className, LPCWSTR windowName));
DECLARE_HOOK_PROC(HWND, WINAPI, GetWindow, (HWND hwnd, UINT cmd));
DECLARE_HOOK_PROC(BOOL, WINAPI, EnumThreadWindows, (DWORD threadId, WNDENUMPROC enumProc, LPARAM lParam));
DECLARE_HOOK_PROC(BOOL, WINAPI, EnumWindows, (WNDENUMPROC enumProc, LPARAM lParam));

COLORREF WINAPI Dropper_GetPixel(HDC dc, int x, int y);
HWND WINAPI KeyboardHook_GetActiveWindow();

//---------------------------------------------------------------------
