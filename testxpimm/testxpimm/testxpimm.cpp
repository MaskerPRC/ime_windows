// testxpimm.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "testxpimm.h"
#include "Windows.h"
#include <Msctf.h>
#include <Ctffunc.h>
#include <Commctrl.h>
#include "setupapi.h"

typedef struct
{
    DWORD dwLangID;                 // language id
    BOOL bDefLang;                  // default language
    BOOL bNoAddCat;                 // don't add category
    UINT uInputType;                // default input type
    LPARAM lParam;                  // item data
    int iIdxTips;                   // index of Tips
    CLSID clsid;                    // tip clsid
    GUID guidProfile;               // tip profile guid
    HKL hklSub;                     // tip substitute hkl
    ATOM atmDefTipName;             // default input name
    ATOM atmTVItemName;             // tree view item name
} TVITEMNODE, * LPTVITEMNODE;
typedef struct
{
    DWORD dwLangID;                  //  语言ID。 
    BOOL bEnabled;                   //  启用状态。 
    BOOL bDefault;                   //  默认配置文件。 
    BOOL fEngineAvailable;           //  发动机状态。 
    BOOL bNoAddCat;                  //  不添加类别。 
    UINT uInputType;                 //  输入类型。 
    CLSID clsid;                     //  TIP CLSID。 
    GUID guidProfile;                //  刀尖配置文件指南。 
    HKL hklSub;                      //  TIP替代方案香港。 
    UINT iLayout;                    //  偏移量进入键盘布局数组。 
    ATOM atmTipText;                 //  布局文本名称。 
} TIPS, * LPTIPS;

typedef struct langnode_s
{
    WORD wStatus;                   // status flags
    UINT iLayout;                   // offset into layout array
    HKL hkl;                        // hkl
    HKL hklUnload;                  // hkl of currently loaded layout
    UINT iLang;                     // offset into lang array
    HANDLE hLangNode;               // handle to free for this structure
    int nIconIME;                   // IME icon
    struct langnode_s* pNext;       // ptr to next langnode
    UINT uModifiers;                // hide Hotkey stuff here
    UINT uVKey;                     //   so we can rebuild the hotkey record
} LANGNODE, * LPLANGNODE;

typedef struct
{
    DWORD dwID;                     // language id
    ATOM atmLanguageName;           // language name - localized
    TCHAR szSymbol[3];              // 2 letter indicator symbol (+ null)
    UINT iUseCount;                 // usage count for this language
    UINT iNumCount;                 // number of links attached
    DWORD dwDefaultLayout;          // default layout id
    LPLANGNODE pNext;               // ptr to lang node structure
} INPUTLANG, * LPINPUTLANG;
typedef PVOID HINF;
typedef struct
{
    DWORD dwID;                     // numeric id
    BOOL bInstalled;                // if layout is installed
    UINT iSpecialID;                // special id (0xf001 for dvorak etc)
    ATOM atmLayoutFile;             // layout file name
    ATOM atmLayoutText;             // layout text name
    ATOM atmIMEFile;                // IME file name
} LAYOUT, * LPLAYOUT;


#define DESC_MAX             MAX_PATH    // max size of a description
#define ALLOCBLOCK           3           // # items added to block for alloc/realloc
#define HKL_LEN              9           // max # chars in hkl id + null

#define LIST_MARGIN          2           // for making the list box look good

#define MB_OK_OOPS           (MB_OK    | MB_ICONEXCLAMATION)    // msg box flags


#define TV_ITEM_TYPE_LANG       0x0001
#define TV_ITEM_TYPE_GROUP      0x0002
#define TV_ITEM_TYPE_KBD        0x0010
#define TV_ITEM_TYPE_SPEECH     0x0020
#define TV_ITEM_TYPE_PEN        0x0040
#define TV_ITEM_TYPE_TIP        0x0100
#define TV_ITEM_TYPE_EXTERNAL   0x0200
#define TV_ITEM_TYPE_SMARTTAG   0x0400

#define IDC_KBDL_DISABLED              1207
#define IDC_KBDL_DISABLED_2            1208

#define INPUT_TYPE_KBD          TV_ITEM_TYPE_KBD
#define INPUT_TYPE_PEN          TV_ITEM_TYPE_PEN
#define INPUT_TYPE_SPEECH       TV_ITEM_TYPE_SPEECH
#define INPUT_TYPE_TIP          TV_ITEM_TYPE_TIP
#define INPUT_TYPE_EXTERNAL     TV_ITEM_TYPE_EXTERNAL
#define INPUT_TYPE_SMARTTAG     TV_ITEM_TYPE_SMARTTAG
#define IDS_LOCALE_UNKNOWN             82


#define IDC_GROUPBOX1                  1100
#define IDC_LOCALE_DEFAULT             1101
#define IDC_LOCALE_DEFAULT_TEXT        1102
#define IDC_GROUPBOX2                  1103
#define IDC_INPUT_LIST                 1104
#define IDC_INPUT_LIST_TEXT            1105
#define IDC_GROUPBOX3                  1106

#define IDC_KBDL_INPUT_FRAME           1200
#define IDC_KBDL_LOCALE                1201
#define IDC_KBDL_LAYOUT_TEXT           1202
#define IDC_KBDL_ADD                   1204
#define IDC_KBDL_EDIT                  1205
#define IDC_KBDL_DELETE                1206
#define IDC_KBDL_DISABLED              1207
#define IDC_KBDL_DISABLED_2            1208
#define IDC_KBDL_CAPSLOCK_FRAME        1209
#define IDC_KBDL_CAPSLOCK              1210
#define IDC_KBDL_SHIFTLOCK             1211
#define IDC_KBDL_SET_DEFAULT           1213
#define IDC_KBDL_SHORTCUT_FRAME        1214
#define IDC_KBDL_ALT_SHIFT             1215
#define IDC_KBDL_CTRL_SHIFT            1216
#define IDC_KBDL_NO_SHIFT              1217
#define IDC_KBDL_INDICATOR             1218
#define IDC_KBDLA_LOCALE               1219
#define IDC_KBDLA_LOCALE_TEXT          1220
#define IDC_KBDLA_LAYOUT               1221
#define IDC_KBDLA_LAYOUT_TEXT          1222
#define IDC_KBDLE_LOCALE               1223
#define IDC_KBDLE_LOCALE_TEXT          1224
#define IDC_KBDLE_LAYOUT               1225
#define IDC_KBDLE_LAYOUT_TEXT          1226
#define IDC_KBDL_ONSCRNKBD             1227
#define IDC_KBDL_UP                    1228
#define IDC_KBDL_DOWN                  1229

#define IDC_KBDL_IME_SETTINGS          1230
#define IDC_KBDL_HOTKEY_LIST           1231
#define IDC_KBDL_HOTKEY_SEQUENCE       1232
#define IDC_KBDL_HOTKEY                1233
#define IDC_KBDL_HOTKEY_FRAME          1234
#define IDC_KBDL_CHANGE_HOTKEY         1235
#define IDC_KBDLH_KEY_COMBO            1236
#define IDC_KBDLH_CTRL                 1237
#define IDC_KBDLH_L_ALT                1238
#define IDC_KBDLH_SHIFT                1239
#define IDC_KBDLH_LAYOUT_TEXT          1240
#define IDC_KBDLH_ENABLE               1241
#define IDC_KBDLH_GRAVE                1242
#define IDC_KBDLH_VLINE                1243
#define IDC_KBDLH_PLUS                 1244
#define IDC_KBDLH_CTRL2                1245
#define IDC_KBDLH_L_ALT2               1246
#define IDC_KBDLH_SHIFT2               1247
#define IDC_KBDLH_PLUS2                1248
#define IDC_KBDLH_LANGHOTKEY           1249
#define IDC_KBDLH_LAYOUTHOTKEY         1250

#define IDC_PEN_TIP                    1300
#define IDC_PEN_TEXT                   1301

#define IDC_SPEECH_TIP                 1400
#define IDC_SPEECH_TEXT                1401
#define IDC_EXTERNAL_TIP               1450
#define IDC_EXTERNAL_TEXT              1451

#define IDC_ADVANCED_CUAS_ENABLE       1460
#define IDC_ADVANCED_CUAS_TEXT         1461
#define IDC_ADVANCED_CTFMON_DISABLE    1462
#define IDC_ADVANCED_CTFMON_TEXT       1463


#define IDS_INPUT_KEYBOARD             1500
#define IDS_INPUT_PEN                  1501
#define IDS_INPUT_SPEECH               1502
#define IDS_INPUT_EXTERNAL             1503



//
//  Input Locale Property Page HotKey Controls.
//
#define IDC_HOTKEY_SETTING             1600
#define IDC_TB_SETTING                 1601
#define IDC_TB_BEHAVIOR_FRAME          1602
#define IDC_TB_SHOWLANGBAR             1603
#define IDC_TB_EXTRAICON               1604
#define IDC_TB_SHRINK                  1605
#define IDC_TB_CLOSE                   1606
#define IDC_TB_TRANSPARENCY_FRAME      1607
#define IDC_TB_OPAQUE                  1608
#define IDC_TB_LOWTRANS                1609
#define IDC_TB_HIGHTRANS               1610
#define IDC_TB_LABELS_FRAME            1611
#define IDC_TB_TEXTLABELS              1612
#define IDC_TB_NOTEXTLABELS            1613
#define IDC_TB_DISABLETEXTSERVICE      1614



//
//  Hotkey Strings.
//

#define IDS_VK_NONE                    2200
#define IDS_VK_SPACE                   2201
#define IDS_VK_PRIOR                   2202
#define IDS_VK_NEXT                    2203
#define IDS_VK_END                     2204
#define IDS_VK_HOME                    2205
#define IDS_VK_F1                      2206
#define IDS_VK_F2                      2207
#define IDS_VK_F3                      2208
#define IDS_VK_F4                      2209
#define IDS_VK_F5                      2210
#define IDS_VK_F6                      2211
#define IDS_VK_F7                      2212
#define IDS_VK_F8                      2213
#define IDS_VK_F9                      2214
#define IDS_VK_F10                     2215
#define IDS_VK_F11                     2216
#define IDS_VK_F12                     2217
#define IDS_VK_OEM_SEMICLN             2218
#define IDS_VK_OEM_EQUAL               2219
#define IDS_VK_OEM_COMMA               2220
#define IDS_VK_OEM_MINUS               2221
#define IDS_VK_OEM_PERIOD              2222
#define IDS_VK_OEM_SLASH               2223
#define IDS_VK_OEM_3                   2224
#define IDS_VK_OEM_LBRACKET            2225
#define IDS_VK_OEM_BSLASH              2226
#define IDS_VK_OEM_RBRACKET            2227
#define IDS_VK_OEM_QUOTE               2228
#define IDS_VK_A                       2229
#define IDS_VK_NONE1                   (IDS_VK_A + 26)
#define IDS_VK_0                       (IDS_VK_A + 27)

#define LANG_ACTIVE          0x0001      // language is active
#define LANG_ORIGACTIVE      0x0002      // language was active to start with
#define LANG_CHANGED         0x0004      // user changed status of language
#define ICON_LOADED          0x0010      // icon read in from file
#define LANG_DEFAULT         0x0020      // current language
#define LANG_DEF_CHANGE      0x0040      // language default has changed
#define LANG_IME             0x0080      // IME
#define LANG_HOTKEY          0x0100      // a hotkey has been defined
#define LANG_UNLOAD          0x0200      // unload language
#define LANG_UPDATE          0x8000      // language needs to be updated

#define US_LOCALE            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define IS_DIRECT_SWITCH_HOTKEY(p)                                   \
                             (((p) >= IME_HOTKEY_DSWITCH_FIRST) &&   \
                              ((p) <= IME_HOTKEY_DSWITCH_LAST))


//
#define CHAR_NULL            TEXT('\0')
#define CHAR_COLON           TEXT(':')

#define CHAR_GRAVE           TEXT('`')


#define INF_STYLE_NONE 0x00000000
#define INF_STYLE_OLDNT 0x00000001
#define INF_STYLE_WIN4 0x00000002

HWND g_hDlg;
HWND g_hwndTV;

HTREEITEM g_hTVRoot;
HIMAGELIST g_hImageList;

UINT g_cTVItemSize = 0;

BOOL g_OSNT4 = FALSE;
BOOL g_OSNT5 = FALSE;
BOOL g_OSWIN95 = FALSE;

BOOL g_bAdvChanged = FALSE;

static BOOL g_bGetSwitchLangHotKey = TRUE;
static BOOL g_bCoInit = FALSE;
static LPINPUTLANG g_lpLang = NULL;

static DWORD g_dwPrimLangID = 0;
static UINT g_iThaiLayout = 0;
static BOOL g_bPenOrSapiTip = FALSE;
static BOOL g_bExtraTip = FALSE;

UINT g_iInputs = 0;
UINT g_iOrgInputs = 0;

WNDPROC g_lpfnTVWndProc = NULL;

TCHAR szPropHwnd[] = TEXT("PROP_HWND");
TCHAR szPropIdx[] = TEXT("PROP_IDX");

TCHAR szDefault[260];
TCHAR szInputTypeKbd[260];
TCHAR szInputTypePen[260];
TCHAR szInputTypeSpeech[260];
TCHAR szInputTypeExternal[260];



static BOOL g_bAdmin_Privileges = FALSE;
static BOOL g_bSetupCase = FALSE;

static BOOL g_bCHSystem = FALSE;
static BOOL g_bMESystem = FALSE;
static BOOL g_bShowRtL = FALSE;

static UINT g_iEnabledTips = 0;
static UINT g_iEnabledKbdTips = 0;
static DWORD g_dwToolBar = 0;
static DWORD g_dwChanges = 0;

static UINT g_iLangBuff;
static HANDLE g_hLang;
static UINT g_nLangBuffSize;

static LPLAYOUT g_lpLayout = NULL;
static UINT g_iLayoutBuff;
static HANDLE g_hLayout;
static UINT g_nLayoutBuffSize;
static UINT g_iLayoutIME;         // Number of IME keyboard layouts.
static int g_iUsLayout;
static DWORD g_dwAttributes;

static int g_cyText;
static int g_cyListItem;

static LPTIPS g_lpTips = NULL;
static UINT g_iTipsBuff;
static UINT g_nTipsBuffSize;
static HANDLE g_hTips;
static TCHAR c_szLocaleInfo[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Locale");
static TCHAR c_szLocaleInfoNT4[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language");
static TCHAR c_szLayoutPath[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static TCHAR c_szLayoutFile[] = TEXT("layout file");
static TCHAR c_szLayoutText[] = TEXT("layout text");
static TCHAR c_szLayoutID[] = TEXT("layout id");
static TCHAR c_szInstalled[] = TEXT("installed");
static TCHAR c_szIMEFile[] = TEXT("IME File");
static TCHAR c_szDisplayLayoutText[] = TEXT("Layout Display Name");

static TCHAR c_szKbdLayouts[] = TEXT("Keyboard Layout");
static TCHAR c_szPreloadKey[] = TEXT("Preload");
static TCHAR c_szSubstKey[] = TEXT("Substitutes");
static TCHAR c_szToggleKey[] = TEXT("Toggle");
static TCHAR c_szToggleHotKey[] = TEXT("Hotkey");
static TCHAR c_szToggleLang[] = TEXT("Language Hotkey");
static TCHAR c_szToggleLayout[] = TEXT("Layout Hotkey");
static TCHAR c_szAttributes[] = TEXT("Attributes");
static TCHAR c_szKbdPreloadKey[] = TEXT("Keyboard Layout\\Preload");
static TCHAR c_szKbdSubstKey[] = TEXT("Keyboard Layout\\Substitutes");
static TCHAR c_szKbdToggleKey[] = TEXT("Keyboard Layout\\Toggle");
static TCHAR c_szInternat[] = TEXT("internat.exe");
static char  c_szInternatA[] = "internat.exe";

static TCHAR c_szTipInfo[] = TEXT("SOFTWARE\\Microsoft\\CTF");
static TCHAR c_szCTFMon[] = TEXT("CTFMON.EXE");
static char  c_szCTFMonA[] = "ctfmon.exe";

static TCHAR c_szScanCodeKey[] = TEXT("Keyboard Layout\\IMEtoggle\\scancode");
static TCHAR c_szValueShiftLeft[] = TEXT("Shift Left");
static TCHAR c_szValueShiftRight[] = TEXT("Shift Right");

static TCHAR c_szIndicator[] = TEXT("Indicator");
static TCHAR c_szCTFMonClass[] = TEXT("CicLoaderWndClass");

static TCHAR c_szLoadImmPath[] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IMM");

static TCHAR c_szPrefixCopy[] = TEXT("KEYBOARD_");
static TCHAR c_szKbdInf[] = TEXT("kbd.inf");
static TCHAR c_szKbdInf9x[] = TEXT("multilng.inf");
static TCHAR c_szKbdIMEInf9x[] = TEXT("ime.inf");
static TCHAR c_szIMECopy[] = TEXT("IME_");

static TCHAR c_szTipCategoryEnable[] = TEXT("Enable");
static TCHAR c_szCTFTipPath[] = TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\");
static TCHAR c_szLangProfileKey[] = TEXT("LanguageProfile");
static TCHAR c_szSubstituteLayout[] = TEXT("SubstituteLayout");

static TCHAR c_szKbdPreloadKey_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout\\Preload");
static TCHAR c_szKbdSubstKey_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout\\Substitutes");
static TCHAR c_szKbdToggleKey_DefUser[] = TEXT(".DEFAULT\\Keyboard Layout\\Toggle");
static TCHAR c_szRunPath_DefUser[] = TEXT(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");

static TCHAR c_szHelpFile[] = TEXT("input.hlp");

static TCHAR szIntlInf[] = TEXT("intl.inf");


HINSTANCE g_hShlwapi = NULL;

typedef HRESULT (*funType)(HKEY,
    TCHAR[],
    TCHAR[MAX_PATH],
    int);
funType pfnSHLoadRegUIString;


#pragma comment(lib, "Imm32.lib")

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTXPIMM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTXPIMM));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTXPIMM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTXPIMM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL GetLanguageName(
    LCID lcid,
    LPTSTR lpLangName,
    UINT cchLangName)
{
    BOOL bRet = TRUE;

    WCHAR wszLangName[MAX_PATH];

    if (!GetLocaleInfoW(lcid,
        LOCALE_SLANGUAGE,
        wszLangName,
        ARRAYSIZE(wszLangName)))
    {
        LoadString(hInst, IDS_LOCALE_UNKNOWN, lpLangName, cchLangName);
        bRet = FALSE;
    }
    else
    {
        lstrcpynW(lpLangName, wszLangName, cchLangName);
    }

    return bRet;
}


DWORD TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}

HKL GetSubstituteHKL(REFCLSID rclsid, LANGID langid, REFGUID guidProfile)
{
    HKEY hkey;
    DWORD cb;
    HKL hkl = NULL;
    TCHAR szSubKeyPath[MAX_PATH];
    TCHAR szSubHKL[MAX_PATH];

    lstrcpynW(szSubKeyPath, c_szCTFTipPath, ARRAYSIZE(szSubKeyPath));

    StringFromGUID2(rclsid, (LPOLESTR)szSubKeyPath + lstrlen(szSubKeyPath), 100);

    lstrcat(szSubKeyPath, TEXT("\\"));
    // lstrcat(szSubKeyPath, TEXT("\\"), ARRAYSIZE(szSubKeyPath));
    
    lstrcat(szSubKeyPath, c_szLangProfileKey);
    // lstrcat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), c_szLangProfileKey);

    lstrcat(szSubKeyPath, TEXT("\\"));
    // lstrcat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), TEXT("\\"));
    swprintf(szSubKeyPath + lstrlen(szSubKeyPath),
        ARRAYSIZE(szSubKeyPath) - lstrlen(szSubKeyPath),
        TEXT("0x%08x"),
        langid);
    lstrcat(szSubKeyPath, TEXT("\\"));
    // lstrcat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), TEXT("\\"));

    StringFromGUID2(guidProfile, (LPOLESTR)szSubKeyPath + lstrlen(szSubKeyPath), 50);

    if (RegOpenKey(HKEY_LOCAL_MACHINE, szSubKeyPath, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(szSubHKL);
        RegQueryValueEx(hkey, c_szSubstituteLayout, NULL, NULL, (LPBYTE)szSubHKL, &cb);
        RegCloseKey(hkey);

        if ((szSubHKL[0] == '0') && ((szSubHKL[1] == 'X') || (szSubHKL[1] == 'x')))
        {
            hkl = (HKL)IntToPtr(TransNum(szSubHKL + 2));

            if (LOWORD(hkl) != langid)
                hkl = 0;
        }
    }
    return hkl;
}

LPTVITEMNODE CreateTVItemNode(DWORD dwLangID)
{
    LPTVITEMNODE pTVItemNode;
    HANDLE hItemNode;

    //
    //  Create the new node.
    //
    if (!(pTVItemNode = (LPTVITEMNODE)LocalAlloc(LPTR, sizeof(TVITEMNODE))))
    {
        return (NULL);
    }

    g_cTVItemSize++;

    //
    //  Fill in the new node with the appropriate info.
    //
    pTVItemNode->dwLangID = dwLangID;
    pTVItemNode->bDefLang = FALSE;
    pTVItemNode->iIdxTips = -1;
    pTVItemNode->atmDefTipName = 0;
    pTVItemNode->atmTVItemName = 0;
    pTVItemNode->lParam = 0;

    //
    //  Return the pointer to the new node.
    //
    return (pTVItemNode);
}

BOOL CALLBACK Locale_EnumChildWndProc(HWND hwnd, LPARAM lParam)
{

    EnableWindow(hwnd, FALSE);
    ShowWindow(hwnd, SW_HIDE);

    return TRUE;
}


void Locale_EnablePane(
    HWND hwnd,
    BOOL bEnable,
    UINT DisableId)
{

    if (!bEnable)
    {
        if (DisableId == IDC_KBDL_DISABLED_2)
        {
            //
            //  Disable all controls.
            //
            EnumChildWindows(hwnd, (WNDENUMPROC)Locale_EnumChildWndProc, 0);

            ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), SW_SHOW);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), TRUE);
        }
        else
        {
            if (!g_iEnabledTips)
            {
                //
                //  Disable all controls.
                //
                EnumChildWindows(hwnd, (WNDENUMPROC)Locale_EnumChildWndProc, 0);

                ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), SW_SHOW);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), TRUE);
            }
            else
            {
                //
                //  Disable Add, Property, Hotkey and default language setting controls.
                //
                EnableWindow(GetDlgItem(hwnd, IDC_LOCALE_DEFAULT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_ADD), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_HOTKEY_SETTING), FALSE);
            }
        }
    }

    return;
}

int CompareStringTIP(LPTSTR lpStr1, LPTSTR lpStr2)
{
    if (g_bCHSystem)
    {
        TCHAR szTemp[MAX_PATH];
        UINT uSize1 = lstrlen(lpStr1);
        UINT uSize2 = lstrlen(lpStr2);
        UINT uSizeDef = lstrlen(szDefault);

        if (uSize1 == uSize2)
            return lstrcmp(lpStr1, lpStr2);

        if (uSize1 > uSizeDef)
        {
            if (lstrcmp(lpStr1 + uSize1 - uSizeDef, szDefault) == 0)
            {
                lstrcpynW(szTemp, lpStr1, ARRAYSIZE(szTemp));
                *(szTemp + uSize1 - uSizeDef) = TEXT('\0');

                return lstrcmp(szTemp, lpStr2);
            }
        }

        if (uSize2 > uSizeDef)
        {
            if (lstrcmp(lpStr2 + uSize2 - uSizeDef, szDefault) == 0)
            {
                lstrcpynW(szTemp, lpStr2, ARRAYSIZE(szTemp));
                *(szTemp + uSize2 - uSizeDef) = TEXT('\0');

                return lstrcmp(szTemp, lpStr1);
            }
        }
    }

    return lstrcmp(lpStr1, lpStr2);
}


HTREEITEM
FindTVLangItem(DWORD dwLangID, LPTSTR lpLangText)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TV_ITEM tvItem;
    HTREEITEM hLangItem;
    LPTVITEMNODE pTVLangNode;

    TCHAR szLangName[MAX_PATH];

    tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
    tvItem.pszText = szLangName;
    tvItem.cchTextMax = sizeof(szLangName) / sizeof(TCHAR);

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot);
        hLangItem != NULL;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        tvItem.hItem = hLangItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            int iSize = lstrlen(lpLangText);

            pTVLangNode = (LPTVITEMNODE)tvItem.lParam;

            if (!pTVLangNode)
            {
                continue;
            }

            *(szLangName + min(iSize, tvItem.cchTextMax)) = TEXT('\0');

            if (!CompareStringTIP(szLangName, lpLangText) ||
                (dwLangID && (pTVLangNode->dwLangID == dwLangID)))
            {
                return hLangItem;
            }
        }
    }

    return NULL;
}

LPLANGNODE Locale_AddToLinkedList(
    UINT idx,
    HKL hkl)
{
    LPINPUTLANG pInpLang = &g_lpLang[idx];
    LPLANGNODE pLangNode;
    LPLANGNODE pTemp;
    HANDLE hLangNode;

    //
    //  Create the new node.
    //
    if (!(hLangNode = LocalAlloc(LHND, sizeof(LANGNODE))))
    {
        return (NULL);
    }
    pLangNode = (LPLANGNODE)LocalLock(hLangNode);

    //
    //  Fill in the new node with the appropriate info.
    //
    pLangNode->wStatus = 0;
    pLangNode->iLayout = (UINT)(-1);
    pLangNode->hkl = hkl;
    pLangNode->hklUnload = hkl;
    pLangNode->iLang = idx;
    pLangNode->hLangNode = hLangNode;
    pLangNode->pNext = NULL;
    pLangNode->nIconIME = -1;

    //
    //  If an hkl is given, see if it's an IME.  If so, mark the status bit.
    //
    if ((hkl) && ((HIWORD(hkl) & 0xf000) == 0xe000))
    {
        pLangNode->wStatus |= LANG_IME;
    }

    //
    //  Put the new node in the list.
    //
    pTemp = pInpLang->pNext;
    if (pTemp == NULL)
    {
        pInpLang->pNext = pLangNode;
    }
    else
    {
        while (pTemp->pNext != NULL)
        {
            pTemp = pTemp->pNext;
        }
        pTemp->pNext = pLangNode;
    }

    //
    //  Increment the count.
    //
    pInpLang->iNumCount++;

    //
    //  Return the pointer to the new node.
    //
    return (pLangNode);
}
BOOL IsUnregisteredFEDummyHKL(HKL hkl)
{
    HKEY hKey;
    BOOL bRet = FALSE;
    TCHAR szFEDummyHKL[10];

    switch (LANGIDFROMLCID(hkl))
    {
    case 0x411: break;
    case 0x412: break;
    case 0x404: break;
    case 0x804: break;
    default:
        goto Exit;
    }

    if (HIWORD((DWORD)(UINT_PTR)hkl) != LOWORD((DWORD)(UINT_PTR)hkl))
    {
        goto Exit;
    }

    wsprintfW(szFEDummyHKL, TEXT("%08x"), LOWORD((DWORD)(UINT_PTR)hkl));

    //
    //  Now read all of preload hkl from the registry.
    //
    if (RegOpenKey(HKEY_CURRENT_USER, c_szKbdPreloadKey, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwIndex;
        DWORD cchValue, cbData;
        LONG dwRetVal;
        TCHAR szValue[MAX_PATH];           // language id (number)
        TCHAR szData[MAX_PATH];            // language name

        dwIndex = 0;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        cbData = sizeof(szData);
        dwRetVal = RegEnumValue(hKey,
            dwIndex,
            szValue,
            &cchValue,
            NULL,
            NULL,
            (LPBYTE)szData,
            &cbData);

        if (dwRetVal != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return (FALSE);
        }


        //
        //  There is FE dummy hkl. we will skip this hkl if it is not loaded
        //  from Preload registry section.
        //
        bRet = TRUE;

        do
        {
            if (!lstrcmp(szFEDummyHKL, szData))
            {
                HKEY hSubKey;
                BOOL bSubHKL = FALSE;

                //
                //  Check substitute hkl.
                //
                if (RegOpenKey(HKEY_CURRENT_USER,
                    c_szKbdSubstKey,
                    &hSubKey) == ERROR_SUCCESS)
                {
                    if (RegQueryValueEx(hSubKey, szData,
                        NULL, NULL,
                        NULL, NULL)
                        == ERROR_SUCCESS)
                    {
                        bSubHKL = TRUE;
                    }
                    RegCloseKey(hSubKey);

                    if (bSubHKL)
                        goto Next;
                }

                //
                //  Found dummy hkl from preload section, so we need to display
                //  this dummy hkl
                //
                bRet = FALSE;
                break;
            }

        Next:
            dwIndex++;
            cchValue = sizeof(szValue) / sizeof(TCHAR);
            szValue[0] = TEXT('\0');
            cbData = sizeof(szData);
            szData[0] = TEXT('\0');
            dwRetVal = RegEnumValue(hKey,
                dwIndex,
                szValue,
                &cchValue,
                NULL,
                NULL,
                (LPBYTE)szData,
                &cbData);

        } while (dwRetVal == ERROR_SUCCESS);

        RegCloseKey(hKey);
    }

Exit:
    return bRet;
}
BOOL IsTipSubstituteHKL(HKL hkl)
{
    UINT ctr;

    //
    //  Search substitute HKL of Tips.
    //
    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {
        if (hkl == g_lpTips[ctr].hklSub)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void Locale_FetchIndicator(
    LPLANGNODE pLangNode)
{
    TCHAR szData[MAX_PATH];
    LPINPUTLANG pInpLang = &g_lpLang[pLangNode->iLang];

    //
    //  Get the indicator by using the first 2 characters of the
    //  abbreviated language name.
    //
    if (GetLocaleInfo(LOWORD(pInpLang->dwID),
        LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
        szData,
        ARRAYSIZE(szData)))
    {
        //
        //  Save the first two characters.
        //
        pInpLang->szSymbol[0] = szData[0];
        pInpLang->szSymbol[1] = szData[1];
        pInpLang->szSymbol[2] = TEXT('\0');
    }
    else
    {
        //
        //  Id wasn't found.  Return question marks.
        //
        pInpLang->szSymbol[0] = TEXT('?');
        pInpLang->szSymbol[1] = TEXT('?');
        pInpLang->szSymbol[2] = TEXT('\0');
    }
}

BOOL Region_OpenIntlInfFile(HINF* phInf)
{
    HINF hIntlInf;

    //
    //  Open the intl.inf file.
    //
    hIntlInf = SetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);
    if (hIntlInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }
    if (!SetupOpenAppendInfFile(NULL, hIntlInf, NULL))
    {
        SetupCloseInfFile(hIntlInf);
        return (FALSE);
    }

    *phInf = hIntlInf;

    return (TRUE);
}
BOOL Region_CloseInfFile(HINF* phInf)
{
    SetupCloseInfFile(*phInf);
    *phInf = INVALID_HANDLE_VALUE;

    return (TRUE);
}
BOOL Region_ReadDefaultLayoutFromInf(
    LPTSTR pszLocale,
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    LPDWORD pdwLocale2,
    LPDWORD pdwLayout2,
    HINF hIntlInf)
{
    INFCONTEXT Context;
    TCHAR szPair[MAX_PATH * 2];
    LPTSTR pPos;
    DWORD dwLangIn = LANGIDFROMLCID(TransNum(pszLocale));
    int iField;

    //
    //  Get the first (default) LANGID:HKL pair for the given locale.
    //    Example String: "0409:00000409"
    //
    szPair[0] = 0;
    if (SetupFindFirstLine(hIntlInf,
        TEXT("Locales"),
        pszLocale,
        &Context))
    {
        SetupGetStringField(&Context, 5, szPair, MAX_PATH, NULL);
    }

    //
    //  Make sure we have a string.
    //
    if (szPair[0] == 0)
    {
        return (FALSE);
    }

    //
    //  Find the colon in the string and then set the position
    //  pointer to the next character.
    //
    pPos = szPair;
    while (*pPos)
    {
        if ((*pPos == CHAR_COLON) && (pPos != szPair))
        {
            *pPos = 0;
            pPos++;
            break;
        }
        pPos++;
    }

    if (pdwLayout2)
        *pdwLayout2 = 0;
    if (pdwLocale2)
        *pdwLocale2 = 0;

    //
    //  If there is a layout, then return the input locale and the layout.
    //
    if ((*pPos) &&
        (*pdwLocale = TransNum(szPair)) &&
        (*pdwLayout = TransNum(pPos)))
    {
        if ((!pdwLocale2) ||
            (!pdwLayout2) ||
            (dwLangIn == LANGIDFROMLCID(*pdwLocale)))
        {
            return (TRUE);
        }

        //
        //  If we get here, the language has a default layout that has a
        //  different locale than the language (e.g. Thai).  We want the
        //  default locale to be English (so that logon can occur with a US
        //  keyboard), but the first Thai keyboard layout should be installed
        //  when the Thai locale is chosen.  This is why we have two locales
        //  and layouts passed back to the caller.
        //
        iField = 6;
        while (SetupGetStringField(&Context, iField, szPair, MAX_PATH, NULL))
        {
            DWORD dwLoc, dwLay;

            //
            //  Make sure we have a string.
            //
            if (szPair[0] == 0)
            {
                iField++;
                continue;
            }

            //
            //  Find the colon in the string and then set the position
            //  pointer to the next character.
            //
            pPos = szPair;

            while (*pPos)
            {
                if ((*pPos == CHAR_COLON) && (pPos != szPair))
                {
                    *pPos = 0;
                    pPos++;
                    break;
                }
                pPos++;
            }

            if (*pPos == 0)
            {
                iField++;
                continue;
            }

            dwLoc = TransNum(szPair);
            dwLay = TransNum(pPos);
            if ((dwLoc == 0) || (dwLay == 0))
            {
                iField++;
                continue;
            }
            if (LANGIDFROMLCID(dwLoc) == dwLangIn)
            {
                *pdwLayout2 = dwLay;
                *pdwLocale2 = dwLoc;
                return (TRUE);
            }
            iField++;
        }

        //
        //  If we get here, then no matching locale could be found.
        //  This should not happen, but do the right thing and
        //  only pass back the default layout if it does.
        //
        return (TRUE);
    }

    //
    //  Return failure.
    //
    return (FALSE);
}


void AddKbdLayoutOnKbdTip(HKL hkl, UINT iLayout)
{
    UINT ctr;

    //
    //  Search substitute HKL of Tips.
    //
    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {

        if (hkl == g_lpTips[ctr].hklSub)
        {
            if (iLayout)
                g_lpTips[ctr].iLayout = iLayout;
        }
    }
}
HMODULE LoadSystemLibrary(
    LPCTSTR lpModuleName)
{
    UINT uRet = 0;
    HINSTANCE hModule = NULL;
    TCHAR szModulePath[MAX_PATH + 1];

    szModulePath[0] = TEXT('\0');

    uRet = GetSystemDirectory(szModulePath, ARRAYSIZE(szModulePath));

    if (uRet >= ARRAYSIZE(szModulePath))
    {
        // we don't have a room to copy module name.
        uRet = 0;
    }
    else if (uRet)
    {
        if (szModulePath[uRet - 1] != TEXT('\\'))
        {
            szModulePath[uRet] = TEXT('\\');
            uRet++;
        }

        if (ARRAYSIZE(szModulePath) - uRet > (UINT)lstrlen(lpModuleName))
        {
            lstrcpyn(&szModulePath[uRet],
                lpModuleName,
                ARRAYSIZE(szModulePath) - uRet);
        }
        else
        {
            uRet = 0;
        }
    }

    if (uRet)
    {
        hModule = LoadLibrary(szModulePath);
    }

    return hModule;
}

BOOL Locale_FileExists(
    LPTSTR pFileName)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL bRet;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(pFileName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    else
    {
        FindClose(FindHandle);
        bRet = TRUE;
    }

    SetErrorMode(OldMode);

    return (bRet);
}



BOOL Locale_LoadLayouts(
    HWND hwnd)
{
    HKEY hKey;
    HKEY hkey1;
    DWORD cb;
    DWORD dwIndex;
    LONG dwRetVal;
    DWORD dwValue;
    DWORD dwType;
    TCHAR szValue[MAX_PATH];           // language id (number)
    TCHAR szData[MAX_PATH];            // language name
    TCHAR szSystemDir[MAX_PATH * 2];
    UINT SysDirLen;
    DWORD dwLayoutID;
    BOOL bLoadedLayout;

    //
    //  Load shlwapi module to get the localized layout name
    //
    g_hShlwapi = LoadSystemLibrary(TEXT("shlwapi.dll"));

    if (g_hShlwapi)
    {
        //
        //  Get address SHLoadRegUIStringW
        //
        pfnSHLoadRegUIString = (funType)GetProcAddress(g_hShlwapi, (LPCSTR)439);
    }

    //
    //  Now read all of the layouts from the registry.
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLayoutPath, &hKey) != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    dwRetVal = RegEnumKey(hKey,
        dwIndex,
        szValue,
        sizeof(szValue) / sizeof(TCHAR));

    if (dwRetVal != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    g_hLayout = LocalAlloc(LHND, ALLOCBLOCK * sizeof(LAYOUT));
    g_nLayoutBuffSize = ALLOCBLOCK;
    g_iLayoutBuff = 0;
    g_iLayoutIME = 0;                    // number of IME layouts.
    g_lpLayout = (LPLAYOUT)LocalLock(g_hLayout);

    if (!g_hLayout)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    //
    //  Save the system directory string.
    //
    szSystemDir[0] = 0;
    if (SysDirLen = GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        if (SysDirLen > MAX_PATH)
        {
            SysDirLen = 0;
            szSystemDir[0] = 0;
        }
        else if (szSystemDir[SysDirLen - 1] != TEXT('\\'))
        {
            szSystemDir[SysDirLen] = TEXT('\\');
            szSystemDir[SysDirLen + 1] = 0;
            SysDirLen++;
        }
    }

    do
    {
        //
        //  New layout - get the layout id, the layout file name, and
        //  the layout description string.
        //
        if (g_iLayoutBuff + 1 == g_nLayoutBuffSize)
        {
            HANDLE hTemp;

            LocalUnlock(g_hLayout);

            g_nLayoutBuffSize += ALLOCBLOCK;
            hTemp = LocalReAlloc(g_hLayout,
                g_nLayoutBuffSize * sizeof(LAYOUT),
                LHND);
            if (hTemp == NULL)
            {
                break;
            }

            g_hLayout = hTemp;
            g_lpLayout = (LPLAYOUT)LocalLock(g_hLayout);
        }

        //
        //  Get the layout id
        //
        dwLayoutID = TransNum(szValue);

        //
        //  Save the layout id.
        //
        g_lpLayout[g_iLayoutBuff].dwID = dwLayoutID;

        lstrcpynW(szData, c_szLayoutPath, ARRAYSIZE(szData));
        lstrcatW(szData, TEXT("\\"));
        // strcat(szData, ARRAYSIZE(szData), TEXT("\\"));
        lstrcatW(szData, szValue);
        // lstrcatW(szData, ARRAYSIZE(szData), szValue);

        if (RegOpenKey(HKEY_LOCAL_MACHINE, szData, &hkey1) == ERROR_SUCCESS)
        {
            //
            //  Get the name of the layout file.
            //
            szValue[0] = TEXT('\0');
            cb = sizeof(szValue);
            if ((RegQueryValueEx(hkey1,
                c_szLayoutFile,
                NULL,
                NULL,
                (LPBYTE)szValue,
                &cb) == ERROR_SUCCESS) &&
                (cb > sizeof(TCHAR)))
            {
                g_lpLayout[g_iLayoutBuff].atmLayoutFile = AddAtom(szValue);

                //
                //  See if the layout file exists already.
                //
                lstrcpynW(szSystemDir + SysDirLen,
                    szValue,
                    ARRAYSIZE(szSystemDir) - SysDirLen);
                g_lpLayout[g_iLayoutBuff].bInstalled = (Locale_FileExists(szSystemDir));

                //
                //  Get the name of the layout.
                //
                szValue[0] = TEXT('\0');
                cb = sizeof(szValue);
                g_lpLayout[g_iLayoutBuff].iSpecialID = 0;
                bLoadedLayout = FALSE;

                if (pfnSHLoadRegUIString &&
                    pfnSHLoadRegUIString(hkey1,
                        c_szDisplayLayoutText,
                        szValue,
                        ARRAYSIZE(szValue)) == S_OK)
                {
                    g_lpLayout[g_iLayoutBuff].atmLayoutText = AddAtom(szValue);
                    bLoadedLayout = TRUE;
                }
                else
                {
                    //
                    //  Get the name of the layout.
                    //
                    szValue[0] = TEXT('\0');
                    cb = sizeof(szValue);
                    if (RegQueryValueEx(hkey1,
                        c_szLayoutText,
                        NULL,
                        NULL,
                        (LPBYTE)szValue,
                        &cb) == ERROR_SUCCESS)
                    {
                        g_lpLayout[g_iLayoutBuff].atmLayoutText = AddAtom(szValue);
                        bLoadedLayout = TRUE;
                    }
                }

                if (bLoadedLayout)
                {

                    //
                    //  See if it's an IME or a special id.
                    //
                    szValue[0] = TEXT('\0');
                    cb = sizeof(szValue);
                    if ((HIWORD(g_lpLayout[g_iLayoutBuff].dwID) & 0xf000) == 0xe000)
                    {
                        //
                        //  Get the name of the IME file.
                        //
                        if (RegQueryValueEx(hkey1,
                            c_szIMEFile,
                            NULL,
                            NULL,
                            (LPBYTE)szValue,
                            &cb) == ERROR_SUCCESS)
                        {
                            g_lpLayout[g_iLayoutBuff].atmIMEFile = AddAtom(szValue);
                            szValue[0] = TEXT('\0');
                            cb = sizeof(szValue);
                            g_iLayoutBuff++;
                            g_iLayoutIME++;   // increment number of IME layouts.
                        }
                    }
                    else
                    {
                        //
                        //  See if this is a special id.
                        //
                        if (RegQueryValueEx(hkey1,
                            c_szLayoutID,
                            NULL,
                            NULL,
                            (LPBYTE)szValue,
                            &cb) == ERROR_SUCCESS)
                        {
                            //
                            //  This may not exist.
                            //
                            g_lpLayout[g_iLayoutBuff].iSpecialID =
                                (UINT)TransNum(szValue);
                        }
                        g_iLayoutBuff++;
                    }
                }
            }

            RegCloseKey(hkey1);
        }

        dwIndex++;
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumKey(hKey,
            dwIndex,
            szValue,
            sizeof(szValue) / sizeof(TCHAR));

    } while (dwRetVal == ERROR_SUCCESS);

    cb = sizeof(DWORD);
    g_dwAttributes = 0;
    if (RegQueryValueEx(hKey,
        c_szAttributes,
        NULL,
        NULL,
        (LPBYTE)&g_dwAttributes,
        &cb) != ERROR_SUCCESS)
    {
        g_dwAttributes &= 0x00FF0000;
    }

    RegCloseKey(hKey);

    if (g_hShlwapi)
    {
        FreeLibrary(g_hShlwapi);
        g_hShlwapi = NULL;
        pfnSHLoadRegUIString = NULL;
    }
    return (g_iLayoutBuff);
}

BOOL Locale_GetActiveLocales(
    HWND hwnd)
{
    HKL* pLangs;
    UINT nLangs, ctr1, ctr2, ctr3, id;
    HWND hwndList = GetDlgItem(hwnd, IDC_INPUT_LIST);
    HKL hklSystem = 0;
    int idxListBox;
    BOOL bReturn = FALSE;
    BOOL bTipSubhkl = FALSE;
    DWORD langLay;
    HANDLE hLangNode;
    LPLANGNODE pLangNode;
    HICON hIcon = NULL;

    TCHAR szLangText[DESC_MAX];
    TCHAR szLayoutName[DESC_MAX];
    LPINPUTLANG pInpLang;
    HTREEITEM hTVItem = NULL;
    TV_ITEM tvItem;


    //
    //  Initialize US layout option.
    //
    g_iUsLayout = -1;

    //
    //  Get the active keyboard layout list from the system.
    //
    if (!SystemParametersInfo(SPI_GETDEFAULTINPUTLANG,
        0,
        &hklSystem,
        0))
    {
        hklSystem = GetKeyboardLayout(0);
    }

    nLangs = GetKeyboardLayoutList(0, NULL);
    if (nLangs == 0)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return bReturn;
    }
    pLangs = (HKL*)LocalAlloc(LPTR, sizeof(DWORD_PTR) * nLangs);

    if (pLangs == NULL)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return bReturn;
    }

    GetKeyboardLayoutList(nLangs, (HKL*)pLangs);

    //
    //  Find the position of the US layout to use as a default.
    //
    for (ctr1 = 0; ctr1 < g_iLayoutBuff; ctr1++)
    {
        if (g_lpLayout[ctr1].dwID == US_LOCALE)
        {
            g_iUsLayout = ctr1;
            break;
        }
    }
    if (ctr1 == g_iLayoutBuff)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        goto Error;
    }

    //
    //  Get the active keyboard information and put it in the internal
    //  language structure.
    //
    for (ctr2 = 0; ctr2 < nLangs; ctr2++)
    {
        //
        //  Filter out TIP substitute HKL from TreeView.
        //
        bTipSubhkl = IsTipSubstituteHKL(pLangs[ctr2]);

        if (hklSystem != pLangs[ctr2] &&
            IsUnregisteredFEDummyHKL(pLangs[ctr2]))
        {
            continue;
        }

        for (ctr1 = 0; ctr1 < g_iLangBuff; ctr1++)
        {
            //
            //  See if there's a match.
            //
            if (LOWORD(pLangs[ctr2]) == LOWORD(g_lpLang[ctr1].dwID))
            {
                LPTVITEMNODE pTVItemNode;

                //
                //  Found a match.
                //  Create a node for this language.
                //
                pLangNode = Locale_AddToLinkedList(ctr1, pLangs[ctr2]);
                if (!pLangNode)
                {
                    Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
                    goto Error;
                }

                //
                //  Get language name to add it to treeview
                //
                pInpLang = &g_lpLang[pLangNode->iLang];
                GetAtomName(pInpLang->atmLanguageName, szLangText, ARRAYSIZE(szLangText));

                if ((HIWORD(pLangs[ctr2]) & 0xf000) == 0xe000)
                {
                    pLangNode->wStatus |= LANG_IME;
                }
                pLangNode->wStatus |= (LANG_ORIGACTIVE | LANG_ACTIVE);
                pLangNode->hkl = pLangs[ctr2];
                pLangNode->hklUnload = pLangs[ctr2];
                Locale_FetchIndicator(pLangNode);

                //
                //  Match the language to the layout.
                //
                pLangNode->iLayout = 0;
                langLay = (DWORD)HIWORD(pLangs[ctr2]);

                if ((HIWORD(pLangs[ctr2]) == 0xffff) ||
                    (HIWORD(pLangs[ctr2]) == 0xfffe))
                {
                    //
                    //  Mark default or previous error as US - this
                    //  means that the layout will be that of the basic
                    //  keyboard driver (the US one).
                    //
                    pLangNode->wStatus |= LANG_CHANGED;
                    pLangNode->iLayout = g_iUsLayout;
                    langLay = 0;
                }
                else if ((HIWORD(pLangs[ctr2]) & 0xf000) == 0xf000)
                {
                    //
                    //  Layout is special, need to search for the ID
                    //  number.
                    //
                    id = HIWORD(pLangs[ctr2]) & 0x0fff;
                    for (ctr3 = 0; ctr3 < g_iLayoutBuff; ctr3++)
                    {
                        if (id == g_lpLayout[ctr3].iSpecialID)
                        {
                            pLangNode->iLayout = ctr3;
                            langLay = 0;
                            break;
                        }
                    }
                    if (langLay)
                    {
                        //
                        //  Didn't find the id, so reset to basic for
                        //  the language.
                        //
                        langLay = (DWORD)LOWORD(pLangs[ctr2]);
                    }
                }

                if (langLay)
                {
                    //
                    //  Search for the id.
                    //
                    for (ctr3 = 0; ctr3 < g_iLayoutBuff; ctr3++)
                    {
                        if (((LOWORD(langLay) & 0xf000) == 0xe000) &&
                            (g_lpLayout[ctr3].dwID) == (DWORD)((DWORD_PTR)(pLangs[ctr2])))
                        {
                            pLangNode->iLayout = ctr3;
                            break;
                        }
                        else
                        {
                            if (langLay == (DWORD)LOWORD(g_lpLayout[ctr3].dwID))
                            {
                                pLangNode->iLayout = ctr3;
                                break;
                            }
                        }
                    }

                    if (ctr3 == g_iLayoutBuff)
                    {
                        //
                        //  Something went wrong or didn't load from
                        //  the registry correctly.
                        //
                        MessageBeep(MB_ICONEXCLAMATION);
                        pLangNode->wStatus |= LANG_CHANGED;
                        pLangNode->iLayout = g_iUsLayout;
                    }
                }

                //
                //  If this is the current language, then it's the default
                //  one.
                //
                if ((DWORD)((DWORD_PTR)pLangNode->hkl) == (DWORD)((DWORD_PTR)hklSystem))
                {
                    TCHAR sz[DESC_MAX];

                    pInpLang = &g_lpLang[ctr1];

                    //
                    //  Found the default.  Set the Default input locale
                    //  text in the property sheet.
                    //
                    if (pLangNode->wStatus & LANG_IME)
                    {
                        GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                            sz,
                            ARRAYSIZE(sz));
                    }
                    else
                    {
                        GetAtomName(pInpLang->atmLanguageName, sz, ARRAYSIZE(sz));
                    }
                    pLangNode->wStatus |= LANG_DEFAULT;
                }

                // Get layout name and add it to treeview
                GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                    szLayoutName,
                    ARRAYSIZE(szLayoutName));

                if (bTipSubhkl)
                {
                    AddKbdLayoutOnKbdTip((HKL)((DWORD_PTR)(pLangs[ctr2])), pLangNode->iLayout);
                }

                if (bTipSubhkl &&
                    (hTVItem = FindTVLangItem(pInpLang->dwID, NULL)))
                {
                    TV_ITEM tvTipItem;
                    HTREEITEM hGroupItem;
                    HTREEITEM hTipItem;
                    LPTVITEMNODE pTVTipItemNode;

                    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
                    tvItem.hItem = hTVItem;

                    //GetAtomName(pTVItemNode->atmDefTipName, szLayoutName, ARRAYSIZE(szLayoutName));

                    tvTipItem.mask = TVIF_HANDLE | TVIF_PARAM;
                    tvTipItem.hItem = hTVItem;

                    //
                    //  Adding the default keyboard layout info into each TIPs.
                    //
                    for (hGroupItem = TreeView_GetChild(g_hwndTV, hTVItem);
                        hGroupItem != NULL;
                        hGroupItem = TreeView_GetNextSibling(g_hwndTV, hGroupItem))
                    {
                        for (hTipItem = TreeView_GetChild(g_hwndTV, hGroupItem);
                            hTipItem != NULL;
                            hTipItem = TreeView_GetNextSibling(g_hwndTV, hTipItem))
                        {
                            tvTipItem.hItem = hTipItem;
                            if (TreeView_GetItem(g_hwndTV, &tvTipItem) && tvTipItem.lParam)
                            {
                                pTVTipItemNode = (LPTVITEMNODE)tvTipItem.lParam;

                                if (pTVTipItemNode->hklSub == pLangNode->hkl)
                                {
                                    pTVTipItemNode->lParam = (LPARAM)pLangNode;
                                }
                            }
                        }

                    }

                    if (pLangNode->wStatus & LANG_DEFAULT)
                    {
                        UINT ctr;
                        TCHAR szDefItem[MAX_PATH];

                        //
                        //  Set the default locale selection.
                        //
                        HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);
                        int idxSel = -1;


                        //
                        //  Search substitute HKL of Tips.
                        //
                        for (ctr = 0; ctr < g_iTipsBuff; ctr++)
                        {
                            if (pLangs[ctr2] == g_lpTips[ctr].hklSub &&
                                g_lpTips[ctr].bDefault)
                            {
                                GetAtomName(g_lpTips[ctr].atmTipText,
                                    szLayoutName,
                                    ARRAYSIZE(szLayoutName));
                                break;
                            }
                        }

                        lstrcpynW(szDefItem, szLangText, ARRAYSIZE(szDefItem));
                        lstrcatW(szDefItem, TEXT(" - "));
                        lstrcatW(szDefItem, szLayoutName);

                        // if ((idxSel = ComboBox_FindStringExact(hwndDefList, 0, szDefItem)) != CB_ERR)
                        //     ComboBox_SetCurSel(hwndDefList, idxSel);

                        // Locale_CommandSetDefault(hwnd);
                    }
                }
                else
                {

                    //
                    //
                    //
                    if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
                        goto Error;

                    pTVItemNode->lParam = (LPARAM)pLangNode;

                    if (!pTVItemNode->atmDefTipName)
                        pTVItemNode->atmDefTipName = AddAtom(szLayoutName);

                    //
                    //  Add language node into treeview
                    //
                    // AddTreeViewItems(TV_ITEM_TYPE_LANG,
                    //     szLangText, NULL, NULL, &pTVItemNode);

                    if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
                        goto Error;

                    pTVItemNode->lParam = (LPARAM)pLangNode;

                    //
                    //  Add keyboard layout item into treeview
                    //
                    // hTVItem = AddTreeViewItems(TV_ITEM_TYPE_KBD,
                    //     szLangText,
                    //     szInputTypeKbd,
                    //     szLayoutName,
                    //     &pTVItemNode);vs
                }

                //
                //  Check Thai layout.
                //
                if (g_dwPrimLangID == LANG_THAI && hTVItem)
                {
                    if (PRIMARYLANGID(LOWORD(g_lpLayout[pLangNode->iLayout].dwID)) == LANG_THAI)
                        g_iThaiLayout++;
                }

                //
                //  Break out of inner loop - we've found it.
                //
                break;

            }
        }
    }

    bReturn = TRUE;

Error:
    if (pLangs)
        LocalFree((HANDLE)pLangs);
    return (bReturn);
}


VOID parpareTSF(HWND hWnd)
{
    ULONG ul;
    ULONG ulCnt;
    HRESULT hr;
    LPTIPS pTips;
    LANGID* plangid;
    UINT uInputType;
    BOOL bReturn = TRUE;
    BOOL bEnabledTip = FALSE;
    TCHAR szTipName[MAX_PATH];
    TCHAR szTipTypeName[MAX_PATH];
    IEnumTfLanguageProfiles* pEnum;
    ITfInputProcessorProfiles* pProfiles = NULL;
    ITfFnLangProfileUtil* pLangUtil = NULL;
    ITfCategoryMgr* pCategory = NULL;

    CoInitialize(NULL);
    const CLSID CLSID_SapiLayr = { 0xdcbd6fa8, 0x032f, 0x11d3, {0xb5, 0xb1, 0x00, 0xc0, 0x4f, 0xc3, 0x24, 0xa1} };
    const IID IID_ITfFnLangProfileUtil = { 0xA87A8574,0xA6C1,0x4E15,0x99,0xF0,0x3D,0x39,0x65,0xF5,0x48,0xEB };
    const GUID GUID_TFCAT_TIP_SMARTTAG = {
    0x7ae86bb7,
    0x262c,
    0x431e,
    {0x91, 0x11, 0xc9, 0x74, 0xb6, 0xb7, 0xca, 0xc3}
    };
    //   
    //  检查SAPI TIP注册。 
    //   
    // hr = CoCreateInstance(CLSID_SapiLayr,
    //     NULL,
    //     CLSCTX_INPROC_SERVER,
    //     IID_ITfFnLangProfileUtil,
    //     (LPVOID*)&pLangUtil);
    // if (SUCCEEDED(hr))
    // {
    //     pLangUtil->RegisterActiveProfiles();
    // }

    //   
    //  加载部件列表。 
    //   
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles,
        (LPVOID*)&pProfiles);

    if (FAILED(hr))
        return;

    //   
    //  创建新节点。 
    //   
    if (!(g_hTips = (LPTIPS)LocalAlloc(LHND, 3 * sizeof(TIPS))))
    {
        return ;
    }

    int g_nTipsBuffSize = 3;
    int g_iTipsBuff = 0;
    g_lpTips = (LPTIPS)LocalLock(g_hTips);


    //   
    //  枚举所有可用语言。 
    //   
    LANGID* pLangId;
    ULONG ulCount;
    hr = pProfiles->GetLanguageList(&pLangId, &ulCount);
    if (SUCCEEDED(S_OK))
    {
        for (int i = 0; i < ulCount; i++)
        {
            if (SUCCEEDED(pProfiles->EnumLanguageProfiles(pLangId[i], &pEnum)))
            {
                TF_LANGUAGEPROFILE tflp;
                CLSID clsid;
                GUID guidProfile;

                while (pEnum->Next(1, &tflp, NULL) == S_OK)
                {
                    BSTR bstr = NULL;
                    BSTR bstr2 = NULL;
                    LANGID langid = tflp.langid;
                    BOOL bNoCategory = FALSE;

                    hr = pProfiles->GetLanguageProfileDescription(
                        tflp.clsid,
                        tflp.langid,
                        tflp.guidProfile,
                        &bstr);

                    if (FAILED(hr))
                        continue;

                    lstrcpynW(szTipName, bstr, ARRAYSIZE(szTipName));

                    if (IsEqualGUID(tflp.catid, GUID_TFCAT_TIP_KEYBOARD))
                    {
                        lstrcpynW(szTipTypeName, szInputTypeKbd, ARRAYSIZE(szTipTypeName));
                        uInputType = 0x0010;
                    }
                    else
                    {
                        g_bExtraTip = TRUE;
                        uInputType = 0x0200;

                        if (pCategory == NULL)
                        {
                            hr = CoCreateInstance(CLSID_TF_CategoryMgr,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_ITfCategoryMgr,
                                (LPVOID*)&pCategory);

                            if (FAILED(hr))
                                return;
                        }

                        if (pCategory->GetGUIDDescription(
                            tflp.catid,
                            &bstr2) == S_OK)
                        {
                            lstrcpynW(szTipTypeName, bstr2, ARRAYSIZE(szTipTypeName));
                        }
                        else
                        {
                            lstrcpynW(szTipTypeName, szInputTypeExternal, ARRAYSIZE(szTipTypeName));
                        }

                        if (IsEqualGUID(tflp.catid, GUID_TFCAT_TIP_SMARTTAG))
                        {
                            bNoCategory = TRUE;
                            uInputType |= 0x0400;
                        }
                    }

                    uInputType |= 0x0100;

                    if (g_iTipsBuff + 1 == g_nTipsBuffSize)
                    {
                        HANDLE hTemp;

                        LocalUnlock(g_hTips);
                        g_nTipsBuffSize += 3;
                        hTemp = LocalReAlloc(g_hTips,
                            g_nTipsBuffSize * sizeof(TIPS),
                            LHND);
                        if (hTemp == NULL)
                            return;
                        g_hTips = hTemp;
                        g_lpTips = (LPTIPS)LocalLock(g_hTips);
                    }

                    g_lpTips[g_iTipsBuff].dwLangID = (DWORD)langid;
                    g_lpTips[g_iTipsBuff].uInputType = uInputType;
                    g_lpTips[g_iTipsBuff].atmTipText = AddAtom(szTipName);
                    g_lpTips[g_iTipsBuff].clsid = tflp.clsid;
                    g_lpTips[g_iTipsBuff].guidProfile = tflp.guidProfile;
                    g_lpTips[g_iTipsBuff].bNoAddCat = bNoCategory;

                    if (pProfiles->GetDefaultLanguageProfile(
                        langid,
                        tflp.catid,
                        &clsid,
                        &guidProfile) == S_OK)
                    {
                        if (IsEqualGUID(tflp.guidProfile, guidProfile))
                            g_lpTips[g_iTipsBuff].bDefault = TRUE;
                    }

                    if (uInputType & INPUT_TYPE_KBD)
                    {
                        g_lpTips[g_iTipsBuff].hklSub = GetSubstituteHKL(tflp.clsid,
                            tflp.langid,
                            tflp.guidProfile);
                    }

                    pProfiles->IsEnabledLanguageProfile(
                        tflp.clsid,
                        tflp.langid,
                        tflp.guidProfile,
                        &bEnabledTip);

                    if (bEnabledTip && langid)
                    {
                        TCHAR szLangName[MAX_PATH];
                        LPTVITEMNODE pTVItemNode;
                        LPTVITEMNODE pTVLangItemNode;

                        GetLanguageName(MAKELCID(langid, SORT_DEFAULT),
                            szLangName,
                            ARRAYSIZE(szLangName));

                        if (!(pTVLangItemNode = CreateTVItemNode(langid)))
                        {
                            bReturn = FALSE;
                            break;
                        }

                        if (pTVLangItemNode->hklSub)
                            pTVLangItemNode->atmDefTipName = AddAtom(szTipName);

                        // AddTreeViewItems(TV_ITEM_TYPE_LANG,
                        //     szLangName,
                        //     NULL,
                        //     NULL,
                        //     &pTVLangItemNode);

                        if (!(pTVItemNode = CreateTVItemNode(langid)))
                        {
                            bReturn = FALSE;
                            break;
                        }

                        pTVItemNode->uInputType = uInputType;
                        pTVItemNode->iIdxTips = g_iTipsBuff;
                        pTVItemNode->clsid = tflp.clsid;
                        pTVItemNode->guidProfile = tflp.guidProfile;
                        pTVItemNode->hklSub = g_lpTips[g_iTipsBuff].hklSub;
                        pTVItemNode->bNoAddCat = g_lpTips[g_iTipsBuff].bNoAddCat;

                        //   
                        //  确保装货尖端取代了hkl。 
                        //   
                        if (pTVItemNode->hklSub)
                        {
                            TCHAR szSubhkl[10];
                            HKL hklNew;

                            swprintf(szSubhkl, ARRAYSIZE(szSubhkl), TEXT("%08x"), (DWORD)(UINT_PTR)pTVItemNode->hklSub);
                            hklNew = LoadKeyboardLayout(szSubhkl,
                                KLF_SUBSTITUTE_OK |
                                KLF_REPLACELANG |
                                KLF_NOTELLSHELL);
                            if (hklNew != pTVItemNode->hklSub)
                            {
                                pTVItemNode->hklSub = 0;
                                g_lpTips[g_iTipsBuff].hklSub = 0;
                            }
                        }

                        //AddTreeViewItems(uInputType,
                        //    szLangName,
                        //    szTipTypeName,
                        //    szTipName,
                        //    &pTVItemNode);

                        g_lpTips[g_iTipsBuff].bEnabled = TRUE;
                        g_iEnabledTips++;

                        if (uInputType & INPUT_TYPE_KBD)
                            g_iEnabledKbdTips++;
                    }

                    g_iTipsBuff++;

                    if (bstr)
                        SysFreeString(bstr);

                    if (bstr2)
                        SysFreeString(bstr2);
                }
                pEnum->Release();
            }
        }
    }
    if (pCategory)
        pCategory->Release();

    if (pLangUtil)
        pLangUtil->Release();

    if (pProfiles)
        pProfiles->Release();

    return ;
}

VOID parpareIME(HWND hWnd) {
    HKEY hKey;
    DWORD cchValue, cbData;
    DWORD dwIndex;
    DWORD dwLocale, dwLayout;
    DWORD dwLocale2, dwLayout2;
    LONG dwRetVal;
    UINT ctr1, ctr2 = 0;
    TCHAR szValue[MAX_PATH];           // language id (number)
    TCHAR szData[MAX_PATH];            // language name
    HINF hIntlInf;
    BOOL bRet;

    if (!(g_hLang = LocalAlloc(LHND, 3 * sizeof(INPUTLANG))))
    {
        Locale_EnablePane(hWnd, FALSE, IDC_KBDL_DISABLED);
        return;
    }

    g_nLangBuffSize = 3;
    g_iLangBuff = 0;
    g_lpLang = (LPINPUTLANG)LocalLock(g_hLang);

    //
    //  Now read all of the locales from the registry.
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLocaleInfo, &hKey) != ERROR_SUCCESS)
    {
        Locale_EnablePane(hWnd, FALSE, IDC_KBDL_DISABLED);
        return;
    }

    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    cbData = sizeof(szData);
    dwRetVal = RegEnumValue(hKey,
        dwIndex,
        szValue,
        &cchValue,
        NULL,
        NULL,
        (LPBYTE)szData,
        &cbData);


    if (dwRetVal != ERROR_SUCCESS)
    {
        Locale_EnablePane(hWnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return;
    }

    //
    //  Open the INF file.
    //
    bRet = Region_OpenIntlInfFile(&hIntlInf);

    do
    {
        //
        //  Check for cchValue > 1 - an empty string will be enumerated,
        //  and will come back with cchValue == 1 for the null terminator.
        //  Also, check for cbData > 2 - an empty string will be 2, since
        //  this is the count of bytes.
        //
        if ((cchValue > 1) && (cchValue < HKL_LEN) && (cbData > 2))
        {
            //
            //  New language - get the language name and the language id.
            //
            if ((g_iLangBuff + 1) == g_nLangBuffSize)
            {
                HANDLE hTemp;

                LocalUnlock(g_hLang);

                g_nLangBuffSize += ALLOCBLOCK;
                hTemp = LocalReAlloc(g_hLang,
                    g_nLangBuffSize * sizeof(INPUTLANG),
                    LHND);
                if (hTemp == NULL)
                {
                    break;
                }

                g_hLang = hTemp;
                g_lpLang = (LPINPUTLANG)LocalLock(g_hLang);
            }

            g_lpLang[g_iLangBuff].dwID = TransNum(szValue);
            g_lpLang[g_iLangBuff].iUseCount = 0;
            g_lpLang[g_iLangBuff].iNumCount = 0;
            g_lpLang[g_iLangBuff].pNext = NULL;

            //
            //  Get the default keyboard layout for the language.
            //
            if (bRet && Region_ReadDefaultLayoutFromInf(szValue,
                &dwLocale,
                &dwLayout,
                &dwLocale2,
                &dwLayout2,
                hIntlInf) == TRUE)
            {
                //
                // The default layout is either the first layout in the inf file line
                // or it's the first layout in the line that has the same language
                // is the locale.
                g_lpLang[g_iLangBuff].dwDefaultLayout = dwLayout2 ? dwLayout2 : dwLayout;
            }

            //
            //  Get the full localized name of the language.
            //
            if (GetLanguageName(LOWORD(g_lpLang[g_iLangBuff].dwID), szData, ARRAYSIZE(szData)))
            {
                g_lpLang[g_iLangBuff].atmLanguageName = AddAtom(szData);
                g_iLangBuff++;
            }
        }

        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        dwRetVal = RegEnumValue(hKey,
            dwIndex,
            szValue,
            &cchValue,
            NULL,
            NULL,
            (LPBYTE)szData,
            &cbData);

    } while (dwRetVal == ERROR_SUCCESS);

    //
    //  If we succeeded in opening the INF file, close it.
    //
    if (bRet)
    {
        Region_CloseInfFile(&hIntlInf);
    }

    RegCloseKey(hKey);
    return;
}

VOID parpareIME2(HWND hWnd) {
    Locale_LoadLayouts(hWnd);
    Locale_GetActiveLocales(hWnd);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   parpareIME(hWnd);
   parpareIME2(hWnd);
   parpareTSF(hWnd);


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
            {
               
                break;
            }
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
