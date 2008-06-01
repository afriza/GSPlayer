#include "GSPlayer2.h"
#include "PpcOptions.h"

#define MAX_KEYMAP			99
#define NAME_KEY_FVIRT		_T("KeyVirt%d")
#define NAME_KEY_CODE		_T("KeyCode%d")
#define NAME_KEY_COMMAND	_T("KeyCmd%d")
#define NAME_RELEASEKEYMAP	_T("ReleaseKeyMap")
#define NAME_DISPAUTOOFF	_T("DispAutoOff")
#define NAME_DISPAUTOON		_T("DispAutoOn")
#define NAME_DISPBATTERY	_T("DispEnableBattery")
#define KEY_KEYMAP			_T("Software\\GreenSoftware\\GSPlayer\\Settings\\Key")

static const int s_nDispSecs[] = {0, 5, 10, 15, 20, 30, 60, 90, 120, 180, 300}; // 5の倍数

CPpcOptions::CPpcOptions()
{
	m_fReleaseKeyMap = TRUE;
	m_nDispAutoOff = 30;
	m_fDispAutoOn = TRUE;
	m_fDispEnableBattery = TRUE;
}

CPpcOptions::~CPpcOptions()
{
}

void CPpcOptions::Save(HANDLE hMap)
{
	COptions::Save(hMap);

	// レジストリキーを消す
	RegDeleteKey(HKEY_CURRENT_USER, KEY_KEYMAP);

	// キー割り当て
	HKEY hKey = 0;
	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, KEY_KEYMAP, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {

		DWORD dwBuf;
		TCHAR szName[MAX_PATH];
		for (int i = 0; i < m_listKeyMap.GetCount(); i++) {
			ACCEL* p = (ACCEL*)m_listKeyMap.GetAt(i);
			dwBuf = p->fVirt;
			wsprintf(szName, NAME_KEY_FVIRT, i);
			RegSetValueEx(hKey, szName, 0, REG_DWORD, (LPBYTE)&dwBuf, sizeof(DWORD));
			dwBuf = p->key;
			wsprintf(szName, NAME_KEY_CODE, i);
			RegSetValueEx(hKey, szName, 0, REG_DWORD, (LPBYTE)&dwBuf, sizeof(DWORD));
			dwBuf = p->cmd;
			wsprintf(szName, NAME_KEY_COMMAND, i);
			RegSetValueEx(hKey, szName, 0, REG_DWORD, (LPBYTE)&dwBuf, sizeof(DWORD));
		}
		RegSetValueEx(hKey, NAME_RELEASEKEYMAP, 0, REG_DWORD, (LPBYTE)&m_fReleaseKeyMap, sizeof(DWORD));
		RegCloseKey(hKey);
	}

	// 画面制御
	if (RegCreateKeyEx(HKEY_CURRENT_USER, KEY_SETTINGS, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
		RegSetValueEx(hKey, NAME_DISPAUTOOFF, 0, REG_DWORD, (LPBYTE)&m_nDispAutoOff, sizeof(DWORD));
		RegSetValueEx(hKey, NAME_DISPAUTOON, 0, REG_DWORD, (LPBYTE)&m_fDispAutoOn, sizeof(DWORD));
		RegSetValueEx(hKey, NAME_DISPBATTERY, 0, REG_DWORD, (LPBYTE)&m_fDispEnableBattery, sizeof(DWORD));
	}
}

void CPpcOptions::Load(HANDLE hMap)
{
	COptions::Load(hMap);

	// キー割り当て
	HKEY hKey = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_KEYMAP, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		TCHAR szName[MAX_PATH];
		DWORD dwType, dwSize;
		for (int i = 0; i < MAX_KEYMAP; i++) {
			int fVirt = -1, key = -1, cmd = -1;
			dwSize = sizeof(DWORD);

			wsprintf(szName, NAME_KEY_FVIRT, i);
			RegQueryValueEx(hKey, szName, 0, &dwType, (LPBYTE)&fVirt, &dwSize);
			wsprintf(szName, NAME_KEY_CODE, i);
			RegQueryValueEx(hKey, szName, 0, &dwType, (LPBYTE)&key, &dwSize);
			wsprintf(szName, NAME_KEY_COMMAND, i);
			RegQueryValueEx(hKey, szName, 0, &dwType, (LPBYTE)&cmd, &dwSize);

			if (fVirt != -1 && key != -1 && cmd != -1) {
				ACCEL* p = new ACCEL;
				p->fVirt = fVirt;
				p->key = key;
				p->cmd = cmd;
				m_listKeyMap.Add((DWORD)p);
			}
		}
		if (RegQueryValueEx(hKey, NAME_RELEASEKEYMAP, 0, &dwType, (LPBYTE)&m_fReleaseKeyMap, &dwSize) != ERROR_SUCCESS)
			m_fReleaseKeyMap = TRUE;
		RegCloseKey(hKey);
	}

	// 画面制御
	if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_SETTINGS, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwType, dwSize;
		dwSize = sizeof(DWORD);
		if (RegQueryValueEx(hKey, NAME_DISPAUTOOFF, 0, &dwType, (LPBYTE)&m_nDispAutoOff, &dwSize) != ERROR_SUCCESS)
			m_nDispAutoOff = 0;
		if (RegQueryValueEx(hKey, NAME_DISPAUTOON, 0, &dwType, (LPBYTE)&m_fDispAutoOn, &dwSize) != ERROR_SUCCESS)
			m_fDispAutoOn = FALSE;
		if (RegQueryValueEx(hKey, NAME_DISPBATTERY, 0, &dwType, (LPBYTE)&m_fDispEnableBattery, &dwSize) != ERROR_SUCCESS)
			m_fDispEnableBattery = FALSE;
		RegCloseKey(hKey);
	}
	m_fTrayIcon = FALSE; // 最小化時のトレイアイコンは常にオン
}

BOOL CALLBACK CPpcOptions::DisplayPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->DisplayPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->DisplayPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;	
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	case WM_SIZE:
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_ENABLE_BATTERY));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_AUTOON));
		return TRUE;
	default:
		return FALSE;
	}
}

void CPpcOptions::DisplayPageOnInitDialog(HWND hwndDlg)
{
	HWND hwndCmb = GetDlgItem(hwndDlg, IDC_CMB_AUTOOFF);
	for (int i = 0; i < sizeof(s_nDispSecs) / sizeof(int); i++) {
		if (i == 0) {
			CTempStr str(IDS_FMT_DISPLAY_AUTOOFF_NONE);
			SendMessage(hwndCmb, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
		}
		else {
			TCHAR sz[32];
			CTempStr str(IDS_FMT_DISPLAY_AUTOOFF);
			wsprintf(sz, str, s_nDispSecs[i]);
			SendMessage(hwndCmb, CB_ADDSTRING, 0, (LPARAM)sz);
		}
		if (m_nDispAutoOff == s_nDispSecs[i])
			SendMessage(hwndCmb, CB_SETCURSEL, i, 0);
	}

	if (m_fDispAutoOn)
		SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_AUTOON), BM_SETCHECK, 1, 0);
	if (m_fDispEnableBattery)
		SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_ENABLE_BATTERY), BM_SETCHECK, 1, 0);
}

void CPpcOptions::DisplayPageOnOK(HWND hwndDlg)
{
	HWND hwndCmb = GetDlgItem(hwndDlg, IDC_CMB_AUTOOFF);
	int nSel = SendMessage(hwndCmb, CB_GETCURSEL, 0, 0);
	if (nSel == CB_ERR)
		return;

	m_nDispAutoOff = s_nDispSecs[nSel];
	m_fDispAutoOn = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_AUTOON), BM_GETCHECK, 0, 0);
	m_fDispEnableBattery = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_ENABLE_BATTERY), BM_GETCHECK, 0, 0);
}

void CPpcOptions::LocationDlgOnInitDialog(HWND hwndDlg)
{
	ShellInitDialog(hwndDlg);
	ShellInitDlgMenu(hwndDlg);
}

void CPpcOptions::LocationDlgOnOK(HWND hwndDlg)
{
	// 入力されたものを取得する
	TCHAR szLocation[MAX_URL];
	GetDlgItemText(hwndDlg, IDC_EDIT_LOCATION, szLocation, MAX_URL);
	if (!_tcslen(szLocation))
		return;

	if (_tcsncmp(szLocation, HTTP_PREFIX, _tcslen(HTTP_PREFIX)) == 0)
		_tcscpy(m_pszLocation, szLocation);
	else
		wsprintf(m_pszLocation, _T("%s%s"), HTTP_PREFIX, szLocation);

	EndDialog(hwndDlg, IDOK);
}

BOOL CALLBACK CPpcOptions::OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		pOptions->OptionsDlgOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->OptionsDlgOnOK(hwndDlg);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_SIZE:
		pOptions->OptionsDlgOnSize(hwndDlg);
		return TRUE;
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	}
	return FALSE;
}

void CPpcOptions::ShowOptionDlg(HWND hwndParent, HANDLE hMap)
{
	m_hMap = hMap;
	DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_OPTIONS_DLG), hwndParent, OptionsDlgProc, (LPARAM)this);
	m_hMap = NULL;
}

void CPpcOptions::OptionsDlgOnInitDialog(HWND hwndDlg)
{
	RECT rc;
	HWND hwndLV;
	LVCOLUMN lvc = {0};
	LVITEM lvi = {0};
	CTempStr str;

	const UINT c_uStrIds[] = {
		IDS_OPTION_PLAYER, IDS_OPTION_DECODER, IDS_OPTION_STREAMING,
		IDS_OPTION_ASSOCIATE, IDS_OPTION_DISPLAY, IDS_OPTION_SKIN, IDS_OPTION_PLUGIN
	};

	ShellInitDialog(hwndDlg);
	SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg, IDR_DONE));

	hwndLV = GetDlgItem(hwndDlg, IDC_LIST_PAGES);	
	ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);

	GetClientRect(hwndLV, &rc);
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT; 
	lvc.pszText = NULL;
	lvc.iSubItem = 0;
	lvc.cx = RECT_WIDTH(&rc) - GetSystemMetrics(SM_CXVSCROLL) - 1;
	ListView_InsertColumn(hwndLV, 0, &lvc);

	for (int i = 0; i < sizeof(c_uStrIds) / sizeof(UINT); i++) {
		str.Load(c_uStrIds[i]);
		lvi.mask = LVIF_TEXT;
		lvi.pszText = str;
		lvi.iItem = ListView_GetItemCount(hwndLV);
		ListView_InsertItem(hwndLV, &lvi);
	}
	ListView_SetItemState(hwndLV, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void CPpcOptions::OptionsDlgOnSize(HWND hwndDlg)
{
	RECT rc;
	GetClientRect(hwndDlg, &rc);
	MoveWindow(GetDlgItem(hwndDlg, IDC_LIST_PAGES), rc.left, rc.top, RECT_WIDTH(&rc), RECT_HEIGHT(&rc), TRUE);
}

void CPpcOptions::OptionsDlgOnOK(HWND hwndDlg)
{
	HWND hwndLV = GetDlgItem(hwndDlg, IDC_LIST_PAGES);
	int nSel = -1;
	int nCount = ListView_GetItemCount(hwndLV);

	for (int i = 0; i < nCount; i++) {
		if (ListView_GetItemState(hwndLV, i, LVIS_SELECTED) == LVIS_SELECTED) {
			nSel = i;
			break;
		}
	}

	switch (nSel) {
	case 0:	// Player
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_PLAYER), hwndDlg, PlayerPageProc, (LPARAM)this);
		break;
	case 1: // Decoder
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_DECODER), hwndDlg, DecoderPageProc, (LPARAM)this);
		break;
	case 2: // Streaming
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_STREAMING), hwndDlg, StreamingPageProc, (LPARAM)this);
		break;
	case 3: // Association
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_ASSOCIATE), hwndDlg, AssociatePageProc, (LPARAM)this);
		break;
	case 4: // Display
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_DISPLAY), hwndDlg, DisplayPageProc, (LPARAM)this);
		break;
	case 5: // Skin
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_SKIN), hwndDlg, SkinPageProc, (LPARAM)this);
		break;
	case 6: // Plug-in
		DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_PAGE_PLUGIN), hwndDlg, PlugInPageProc, (LPARAM)this);
		break;
	}
}

BOOL CALLBACK CPpcOptions::PlayerPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->PlayerPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->PlayerPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	case WM_SIZE:
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_PEEK));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_SAVE_DEFLIST));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_ADD_EXISTING));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_RESUME));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_PLAYONSTART));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_SCROLLTITLE));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_TRAYICON));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_USESYSVOLUME));
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK CPpcOptions::DecoderPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->DecoderPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->DecoderPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam; 
			if (pnmh->code == UDN_DELTAPOS) {
				pOptions->DecoderPageOnDeltaPos(hwndDlg, (NM_UPDOWN*)pnmh);
				return TRUE;
			}
			return FALSE;
		}
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	case WM_SIZE:
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_OUTPUT_FADE));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_SCANCOMPLETELY));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_SUPZERO));
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_ALWAYSOPENDEV));
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK CPpcOptions::StreamingPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->StreamingPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->StreamingPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam; 
			if (pnmh->code == UDN_DELTAPOS) {
				pOptions->StreamingPageOnDeltaPos(hwndDlg, (NM_UPDOWN*)pnmh);
				return TRUE;
			}
			return FALSE;
		}
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	case WM_SIZE:
		ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, IDC_CHECK_PROXY));
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK CPpcOptions::SkinPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->SkinPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BROWSE:
			pOptions->SkinPageOnBrowse(hwndDlg);
			return TRUE;
		case IDOK:
			pOptions->StreamingPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK CPpcOptions::AssociatePageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndLV;
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->AssociatePageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->AssociatePageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDC_CHECK_ALL:
			hwndLV = GetDlgItem(hwndDlg, IDC_LIST_EXT);
			for (int i = 0; i < ListView_GetItemCount(hwndLV); i++) {
				ListView_SetCheckState(hwndLV, i , TRUE);
			}
			return TRUE;
		case IDC_CLEAR_ALL:
			hwndLV = GetDlgItem(hwndDlg, IDC_LIST_EXT);
			for (int i = 0; i < ListView_GetItemCount(hwndLV); i++) {
				ListView_SetCheckState(hwndLV, i , FALSE);
			}
			return TRUE;
		}
		return FALSE;
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
#ifdef _WIN32_WCE_PPC
	case WM_SIZE:
		{
			RECT rc;
			HWND hwnd;
			POINT pt;

#define BUTTON_HEIGHT	SCALEY(24)
#define BUTTON_MARGIN	SCALEX(2)
			hwnd = GetDlgItem(hwndDlg, IDC_LIST_EXT);
			GetWindowRect(hwnd, &rc);
			pt.x = rc.left; pt.y = rc.top;
			ScreenToClient(hwndDlg, &pt);
			MoveWindow(hwnd, pt.x, pt.y, LOWORD(lParam) - pt.x * 2, 
				HIWORD(lParam) - pt.y - (BUTTON_HEIGHT + BUTTON_MARGIN * 2), TRUE);

			GetWindowRect(hwnd, &rc);
			SendMessage(hwnd, LVM_SETCOLUMNWIDTH, 0, 
				MAKELPARAM(RECT_WIDTH(&rc) - GetSystemMetrics(SM_CXVSCROLL) - GetSystemMetrics(SM_CXBORDER) * 2, 0));

			hwnd = GetDlgItem(hwndDlg, IDC_CHECK_ALL);
			GetWindowRect(hwnd, &rc);
			pt.x = rc.left; pt.y = rc.top;
			ScreenToClient(hwndDlg, &pt);
			MoveWindow(hwnd, pt.x, HIWORD(lParam) - (BUTTON_HEIGHT + BUTTON_MARGIN), RECT_WIDTH(&rc), BUTTON_HEIGHT, TRUE);

			hwnd = GetDlgItem(hwndDlg, IDC_CLEAR_ALL);
			GetWindowRect(hwnd, &rc);
			pt.x = rc.left; pt.y = rc.top;
			ScreenToClient(hwndDlg, &pt);
			MoveWindow(hwnd, pt.x, HIWORD(lParam) - (BUTTON_HEIGHT + BUTTON_MARGIN), RECT_WIDTH(&rc), BUTTON_HEIGHT, TRUE);

			return FALSE;
		}
#endif
	default:
		return FALSE;
	}
}

BOOL CALLBACK CPpcOptions::PlugInPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcOptions* pOptions;
	switch (uMsg) {
	case WM_INITDIALOG:
		pOptions = (CPpcOptions*)lParam;
		ShellInitDialog(hwndDlg);
		SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg));
		pOptions->PlugInPageOnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pOptions->PlugInPageOnOK(hwndDlg);
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return pOptions->PlugInPageOnCommand(hwndDlg, wParam, lParam);;
	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam; 
			if (pnmh->code == NM_DBLCLK) {
				pOptions->PlugInPageOnCommand(hwndDlg, IDC_CONFIG, 0);
				return TRUE;
			}
			else if (pnmh->code == LVN_ITEMCHANGED) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIG), 
					ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_LIST_PLUGIN)) ? TRUE : FALSE);
			}
			return FALSE;
		}
	case WM_PAINT:
		return DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
#ifdef _WIN32_WCE_PPC
	case WM_SIZE:
		{
			RECT rc;
			HWND hwnd;
			POINT pt;

#define BUTTON_HEIGHT	SCALEY(24)
#define BUTTON_MARGIN	SCALEX(2)
			hwnd = GetDlgItem(hwndDlg, IDC_LIST_PLUGIN);
			GetWindowRect(hwnd, &rc);
			pt.x = rc.left; pt.y = rc.top;
			ScreenToClient(hwndDlg, &pt);
			MoveWindow(hwnd, pt.x, pt.y, LOWORD(lParam) - pt.x * 2, 
				HIWORD(lParam) - pt.y - (BUTTON_HEIGHT + BUTTON_MARGIN * 2), TRUE);

			GetWindowRect(hwnd, &rc);
			SendMessage(hwnd, LVM_SETCOLUMNWIDTH, 0, 
				MAKELPARAM(RECT_WIDTH(&rc) - GetSystemMetrics(SM_CXVSCROLL) - GetSystemMetrics(SM_CXBORDER) * 2, 0));

			hwnd = GetDlgItem(hwndDlg, IDC_CONFIG);
			GetWindowRect(hwnd, &rc);
			pt.x = rc.left; pt.y = rc.top;
			ScreenToClient(hwndDlg, &pt);
			MoveWindow(hwnd, pt.x, HIWORD(lParam) - (BUTTON_HEIGHT + BUTTON_MARGIN), RECT_WIDTH(&rc), BUTTON_HEIGHT, TRUE);

			return FALSE;
		}
#endif
	default:
		return FALSE;
	}
}
