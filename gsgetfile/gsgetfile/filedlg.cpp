#include "resourceppc.h"
#include <windows.h>
#include <tchar.h>
#include <commdlg.h>
#include "filedlg.h"


#define CLB_GETCURSEL		(m_fDlgType == DLG_TYPE_PPC3 ? CB_GETCURSEL : LB_GETCURSEL)
#define CLB_ADDSTRING		(m_fDlgType == DLG_TYPE_PPC3 ? CB_ADDSTRING : LB_ADDSTRING)
#define CLB_SETITEMDATA		(m_fDlgType == DLG_TYPE_PPC3 ? CB_SETITEMDATA : LB_SETITEMDATA)
#define CLB_SETCURSEL		(m_fDlgType == DLG_TYPE_PPC3 ? CB_SETCURSEL : LB_SETCURSEL)
#define CLB_GETCOUNT		(m_fDlgType == DLG_TYPE_PPC3 ? CB_GETCOUNT : LB_GETCOUNT)
#define CLB_GETITEMDATA		(m_fDlgType == DLG_TYPE_PPC3 ? CB_GETITEMDATA : LB_GETITEMDATA)
#define CLB_RESETCONTENT	(m_fDlgType == DLG_TYPE_PPC3 ? CB_RESETCONTENT : LB_RESETCONTENT)


static const int s_nListColumnWidth[] = 
{
	145, 65, 75, 120
};

// public members
CFileDialog::CFileDialog(LPOPENFILENAME pofn)
{
	m_pofn = pofn;
	m_hwndDlg = NULL;
	m_hFnt = NULL;

	m_nListSort = LIST_SORT_NAME;
	m_fSortOrder = TRUE;
	m_fShowExt = FALSE;
	
	m_pszFilter = NULL;
	m_pszDefExt = NULL;

	m_fSelMode = FALSE;
}

CFileDialog::~CFileDialog()
{
	if (m_hFnt)
		DeleteObject(m_hFnt);
}

int CFileDialog::DoModal(BOOL fSave)
{
	if (!m_pofn)
		return IDCANCEL;

	m_fSave = fSave;
	int nResource = GetDlgResourceID();
	if (!nResource)
		return FALSE;
	
	return DialogBoxParam(g_hInst, MAKEINTRESOURCE(nResource), 
							m_pofn->hwndOwner, FileDlgProc, (LPARAM)this);
}

LRESULT CALLBACK ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC p = (WNDPROC)GetWindowLong(hwnd, GWL_USERDATA);
	if (uMsg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;
	return CallWindowProc(p, hwnd, uMsg, wParam, lParam);
}

// protected members
BOOL CFileDialog::OnInitDialog(HWND hwndDlg)
{
	m_hwndDlg = hwndDlg;
	m_hwndLV = GetDlgItem(m_hwndDlg, IDC_LIST_FILE);
	m_helper.SHInitDialog(m_hwndDlg);

	INITCOMMONCONTROLSEX iccex;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;
    InitCommonControlsEx(&iccex);

	GetShellSettings();
	CreateToolBar();
	CheckWindowSize();
	ParseFilter();
	CreateExtList();
	if (m_fDlgType == DLG_TYPE_PPC3) {
		SendMessage(GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER), CB_SETEXTENDEDUI, 1, 0);
	}

	InitListView();

	SetDlgItemText(m_hwndDlg, IDC_EDIT_NAME, m_pofn->lpstrFile);

	LPTSTR psz;
	TCHAR sz[MAX_LOADSTRING];
	if (m_pofn->lpstrInitialDir &&
		_tcslen(m_pofn->lpstrInitialDir) &&
		LoadFolderItem(m_pofn->lpstrInitialDir))
		goto done;
	
	_tcscpy(sz, m_pofn->lpstrFile);
	if (_tcslen(sz) && LoadFolderItem(sz))
		goto done;

	psz = _tcsrchr(sz, _T('\\'));
	if (psz) {
		SetDlgItemText(m_hwndDlg, IDC_EDIT_NAME, psz + 1);
		*psz = NULL;
	}
	if (_tcslen(sz) && LoadFolderItem(sz))
		goto done;

	LoadString(g_hInst, IDS_DEF_DIR, sz, MAX_LOADSTRING);
	if (LoadFolderItem(sz))
		goto done;
	
	LoadFolderItem(_T("\\"));

done:
	if (m_fSave)
		SetFocus(GetDlgItem(m_hwndDlg, IDC_EDIT_NAME));
	else
		SetFocus(GetDlgItem(m_hwndDlg, IDC_LIST_FILE));
	return FALSE;
}

void CFileDialog::OnOK()
{
	DWORD dwAttr;
	LPTSTR pszSrc = NULL, pszPath = NULL, p;
	TCHAR szMsg[MAX_LOADSTRING];
	TCHAR szTitle[MAX_LOADSTRING];

	if (m_pofn->lpstrTitle)	
		_tcsncpy(szTitle, m_pofn->lpstrTitle, MAX_LOADSTRING);
	else
		LoadString(g_hInst, m_fSave ? IDS_SAVE_TITLE : IDS_OPEN_TITLE, szTitle, MAX_LOADSTRING);

	int n = GetWindowTextLength(GetDlgItem(m_hwndDlg, IDC_EDIT_NAME));
	if (!n)	return;

	SendMessage(GetDlgItem(m_hwndDlg, IDC_EDIT_NAME), EM_SETSEL, 0, -1);

	pszSrc = new TCHAR[n + 1];
	GetDlgItemText(m_hwndDlg, IDC_EDIT_NAME, pszSrc, n + 1);

	if (_tcschr(pszSrc, _T('/')) ||
		_tcschr(pszSrc, _T(':')) ||
		_tcschr(pszSrc, _T(';')) ||
		_tcschr(pszSrc, _T('?')) ||
		_tcschr(pszSrc, _T(':')) ||
		_tcschr(pszSrc, _T('<')) ||
		_tcschr(pszSrc, _T('>')) ||
		_tcschr(pszSrc, _T('|')))
		goto done;

	if (_tcschr(pszSrc, _T('*'))) {
		// search
		if (m_pszFilter) {
			delete [] m_pszFilter;
			m_pszFilter = NULL;
		}
		m_pszFilter = pszSrc; pszSrc = NULL;
		LoadFolderItem(m_szCurrent, FALSE);
		goto done;
	}
	else if (_tcschr(pszSrc, _T('\"'))) {
		// multi
		if (m_fSave || !(m_pofn->Flags & OFN_ALLOWMULTISELECT))
			goto done;

		n = m_pofn->nMaxFile;
		pszPath = m_pofn->lpstrFile;
		p = _tcslen(m_szCurrent) ? m_szCurrent : _T("\\");
		_tcsncpy(pszPath, p, n);
		n -= _tcslen(pszPath) + 2;
		pszPath += _tcslen(pszPath);
		*pszPath++ = NULL;
		
		p = pszSrc;
		while (n > 0 && *p != NULL) {
			LPTSTR psz;
			if (*p == _T('\"'))
				p++;

			psz = _tcschr(p, _T('\"'));
			if (m_pofn->Flags & OFN_FILEMUSTEXIST) {
				int nLen = MAX_PATH;
				TCHAR szPath[MAX_PATH + 1] = _T("");
				
				_tcscpy(szPath, m_szCurrent);
				_tcscat(szPath, _T("\\"));
				nLen -= _tcslen(m_szCurrent) + 1;
				if (psz)
					_tcsncat(szPath, p, min(psz - p, nLen));
				else
					_tcsncat(szPath, p, nLen);
				dwAttr = GetFileAttributes(szPath);
				if (dwAttr == 0xFFFFFFFF ||
					(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
					LoadString(g_hInst, IDS_MSG_FILE_MUST_EXIST, szMsg, MAX_LOADSTRING);
					p = new TCHAR[_tcslen(szPath) + _tcslen(szMsg) + 1];
					wsprintf(p, _T("%s%s"), szPath, szMsg);
					MessageBox(m_hwndDlg, p, szTitle, MB_ICONEXCLAMATION | MB_OK);
					delete [] p;

					pszPath = NULL;
					goto done;
				}
			}

			if (psz) {
				_tcsncpy(pszPath, p, min(psz - p, n));
				n -= psz - p;
				pszPath += psz - p;
			}
			else {
				_tcsncpy(pszPath, p, n);
				n -= _tcslen(p);
				pszPath += _tcslen(p);
			}
			if (n > 0)
				*pszPath++ = NULL;

			if (!psz)
				break;

			p = psz;
			while (*p == _T(' ') || *p == _T('\"'))
				p++;
		}
		pszPath = NULL;

		n = (n > 1) ? IDOK : IDCANCEL;
		EndDialog(n);
	}
	else {
		// single
		n = (*pszSrc != _T('\\')) ? _tcslen(m_szCurrent) + _tcslen(pszSrc) + 1 : _tcslen(pszSrc);
		p = _tcsrchr(pszSrc, _T('.'));
		if (!p || _tcschr(p, _T('\\'))) {
			if (m_pszDefExt)
				n += _tcslen(m_pszDefExt);
			else if (m_pofn->lpstrDefExt) {
				n += _tcslen(m_pofn->lpstrDefExt) + 1;
			}
			else {
				p = _tcschr((LPTSTR)m_listExt.GetAt(0), _T('.'));
				if (p && !_tcschr(p, _T('*')))
					n += _tcslen(p);
			}
		}
		else {
			if (m_pofn->lpstrDefExt && m_pszDefExt)
				n += max(_tcslen(m_pszDefExt), _tcslen(m_pofn->lpstrDefExt) + 1);
			else if (m_pszDefExt)
				n += _tcslen(m_pszDefExt);
			else if (m_pofn->lpstrDefExt)
				n += _tcslen(m_pofn->lpstrDefExt) + 1;
		}
		pszPath = new TCHAR[n + 1];
		*pszPath = NULL;
		
		if (*pszSrc != _T('\\')) {
			_tcscpy(pszPath, m_szCurrent);
			_tcsncat(pszPath, _T("\\"), n);
		}
		_tcscat(pszPath, pszSrc);

		p = _tcsrchr(pszSrc, _T('.'));
		if (!p || _tcschr(p, _T('\\'))) {
			if (m_pszDefExt) 
				_tcscat(pszPath, m_pszDefExt);
			else {
				dwAttr = GetFileAttributes(pszPath);
				if (dwAttr != 0xFFFFFFFF &&
					(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
					if (LoadFolderItem(pszPath, FALSE)) {
						goto done;
					}
				}
				else if (m_pofn->lpstrDefExt) {
					_tcscat(pszPath, _T("."));
					_tcscat(pszPath, m_pofn->lpstrDefExt);
				}
				else {
					p = _tcschr((LPTSTR)m_listExt.GetAt(0), _T('.'));
					if (p && !_tcschr(p, _T('*')))
						_tcscat(pszPath, p);
				}
			}
		}
		else {
			dwAttr = GetFileAttributes(pszPath);
			if (dwAttr == 0xFFFFFFFF ||
				(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
				if (m_pszDefExt)
					_tcscat(pszPath, m_pszDefExt);
				//else if (m_pofn->lpstrDefExt)
				//	_tcscat(pszPath, m_pofn->lpstrDefExt);
			}
		}
		if (!m_fSave && (m_pofn->Flags & OFN_FILEMUSTEXIST)) {
			dwAttr = GetFileAttributes(pszPath);
			if (dwAttr == 0xFFFFFFFF ||
				(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
				LoadString(g_hInst, IDS_MSG_FILE_MUST_EXIST, szMsg, MAX_LOADSTRING);
				p = new TCHAR[_tcslen(pszPath) + _tcslen(szMsg) + 1];
				wsprintf(p, _T("%s%s"), pszPath, szMsg);
				MessageBox(m_hwndDlg, p, szTitle, MB_ICONEXCLAMATION | MB_OK);
				delete [] p;
				goto done;
			}
		}
		if (!m_fSave && (m_pofn->Flags & OFN_CREATEPROMPT)) {
			dwAttr = GetFileAttributes(pszPath);
			if (dwAttr == 0xFFFFFFFF || 
				(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
				LoadString(g_hInst, IDS_MSG_CREATE_PROMPT, szMsg, MAX_LOADSTRING);
				p = new TCHAR[_tcslen(pszPath) + _tcslen(szMsg) + 1];
				wsprintf(p, _T("%s%s"), pszPath, szMsg);
				n = MessageBox(m_hwndDlg, p, szTitle, MB_ICONQUESTION | MB_YESNO);
				delete [] p;
				if (n == IDNO)
					goto done;
			}
		}
		if (m_fSave && (m_pofn->Flags & OFN_PATHMUSTEXIST)) {
			p = _tcsrchr(pszPath, _T('\\'));
			if (p) *p = NULL;
			dwAttr = GetFileAttributes(pszPath);
			if (p) *p = _T('\\');

			if (dwAttr == 0xFFFFFFFF ||
				!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
				LoadString(g_hInst, IDS_MSG_PATH_MUST_EXIST, szMsg, MAX_LOADSTRING);
				p = new TCHAR[_tcslen(pszPath) + _tcslen(szMsg) + 1];
				wsprintf(p, _T("%s%s"), pszPath, szMsg);
				MessageBox(m_hwndDlg, p, szTitle, MB_ICONEXCLAMATION | MB_OK);
				delete [] p;
				goto done;
			}
		}
		if (m_fSave && (m_pofn->Flags & OFN_OVERWRITEPROMPT)) {
			dwAttr = GetFileAttributes(pszPath);
			if (dwAttr != 0xFFFFFFFF && 
				!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
				LoadString(g_hInst, IDS_MSG_OVERWRITE_PROMPT, szMsg, MAX_LOADSTRING);
				p = new TCHAR[_tcslen(pszPath) + _tcslen(szMsg) + 1];
				wsprintf(p, _T("%s%s"), pszPath, szMsg);
				n = MessageBox(m_hwndDlg, p, szTitle, MB_ICONEXCLAMATION | MB_YESNO);
				delete [] p;
				if (n == IDNO)
					goto done;
			}
		}
		_tcsncpy(m_pofn->lpstrFile, pszPath, m_pofn->nMaxFile - 1);
		m_pofn->lpstrFile[m_pofn->nMaxFile - 1] = NULL;
		if (m_pofn->lpstrFileTitle && m_pofn->nMaxFileTitle) {
			p = _tcsrchr(pszPath, _T('\\'));
			if (p) 
				_tcsncpy(m_pofn->lpstrFileTitle, p + 1, m_pofn->nMaxFileTitle - 1);
			else
				_tcsncpy(m_pofn->lpstrFileTitle, pszPath, m_pofn->nMaxFileTitle - 1);
			m_pofn->lpstrFileTitle[m_pofn->nMaxFileTitle - 1] = NULL;
		}

		n = (_tcslen(pszPath) + 1 <= m_pofn->nMaxFile) ? IDOK : IDCANCEL;
		EndDialog(n);
	}

done:
	if (pszPath)
		delete [] pszPath;
	if (pszSrc)
		delete [] pszSrc;
}

void CFileDialog::EndDialog(int nResult)
{
	if (nResult == IDOK) {
		int n = SendMessage(GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER), CLB_GETCURSEL, 0, 0);
		if (n != CB_ERR && m_pofn->lpstrFilter)
			m_pofn->nFilterIndex = n + 1;
	}	
	if (m_pszFilter) {
		delete [] m_pszFilter;
		m_pszFilter = NULL;
	}
	if (m_pszDefExt) {
		delete [] m_pszDefExt;
		m_pszDefExt = NULL;
	}

	ClearFilter();
	DeleteExtList();
	DestroyListView();
	CommandBar_Destroy(m_hwndCB);
	::EndDialog(m_hwndDlg, nResult);
}

BOOL CALLBACK CFileDialog::FileDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CFileDialog* pDlg = (CFileDialog*)GetWindowLong(hwndDlg, DWL_USER);
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CFileDialog*)lParam;
		SetWindowLong(hwndDlg, DWL_USER, (DWORD)lParam);
		return pDlg->OnInitDialog(hwndDlg);
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDOK:
				if (GetFocus() == GetDlgItem(hwndDlg, IDC_LIST_FILE))
					pDlg->OnListDblClk();
				else
					pDlg->OnOK();
				return TRUE;
			case IDCANCEL:
				pDlg->EndDialog(LOWORD(wParam));
				return TRUE;
			case ID_LIST_STYLE_LIST:
				pDlg->ChangeListStyle(LVS_LIST);
				return TRUE;
			case ID_LIST_STYLE_REPORT:
				pDlg->ChangeListStyle(LVS_REPORT);
				return TRUE;
			case ID_LIST_SELECTALL:
				pDlg->SelectAllItems();
				return TRUE;
			case ID_UP: 
				pDlg->OnUp();
				return TRUE;
			case IDC_COMBO_FILTER:
				if (HIWORD(wParam) == LBN_SELCHANGE)
					pDlg->OnCBSelChange();
				return TRUE;
			case ID_KEY_CTRL:
				pDlg->OnSelMode();				
				return TRUE;
		}
		break;
	case WM_NOTIFY:
	{
		NMHDR* pnmh = (NMHDR*)lParam;
		switch (pnmh->code) {
		case LVN_GETDISPINFO:
			pDlg->OnGetDispInfo((NMLVDISPINFO*)lParam);
			return TRUE;
		case NM_CLICK:
			pDlg->OnListClick();
			return TRUE;
		case NM_DBLCLK:
		case NM_RETURN:
			pDlg->OnListDblClk();
			return TRUE;
		case LVN_KEYDOWN:
			pDlg->OnListKeyDown((NMLVKEYDOWN*)lParam);
			return TRUE;
		case LVN_COLUMNCLICK:
			pDlg->OnListColumnClick((NMLISTVIEW*)lParam);
			return TRUE;
		case LVN_ITEMCHANGED:
			pDlg->OnListItemChanged((NMLISTVIEW*)lParam);
			return TRUE;
		case LVN_ITEMCHANGING:
			SetWindowLong(hwndDlg, DWL_MSGRESULT, pDlg->OnListItemChanging((NMLISTVIEW*)lParam));
			return TRUE;
		case UDN_DELTAPOS:
			pDlg->OnSpinDeltaPos((NMUPDOWN*)lParam);
			return TRUE;
		}
		break;
	}
	case WM_WININICHANGE:
		pDlg->CheckListHeight();
		return TRUE;
	case WM_PAINT:
		return pDlg->m_helper.DefDlgPaintProc(hwndDlg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
		return pDlg->m_helper.DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
	case WM_INITMENUPOPUP:
		pDlg->OnInitMenuPopup((HMENU)wParam);
		return TRUE;
	case WM_SIZE:
		pDlg->CheckWindowSize();
		return TRUE;
	}
	return FALSE;
}

int CFileDialog::GetDlgResourceID()
{
	if (m_helper.IsPocketPC()) {
		m_fDlgType = DLG_TYPE_PPC3;
		return IDD_OPENFILEDLG_PPC;
	}
	else if (m_helper.IsSmartPhone()) {
		m_fDlgType = DLG_TYPE_SP;
		return IDD_OPENFILEDLG_SP;
	}
	return 0;
}

void CFileDialog::CreateToolBar()
{
	BOOL bMultiSel = FALSE;
	if ((m_pofn->Flags & OFN_ALLOWMULTISELECT) && !m_fSave)
		bMultiSel = TRUE;

	m_hwndCB = m_helper.SHCreateMenuBar(m_hwndDlg, bMultiSel ? IDR_MAIN_MULTI : IDR_MAIN_SINGLE);

	if (m_hFnt) {
		DeleteObject(m_hFnt);
	}
	m_hFnt = CreatePointFont(90, _T(""), TRUE);
	SendMessage(GetDlgItem(m_hwndDlg, IDC_STATIC_CURRENT_TEXT), WM_SETFONT, (WPARAM)m_hFnt, 0);
}

void CFileDialog::CheckWindowSize()
{
	RECT rcParent;
	GetClientRect(m_hwndDlg, &rcParent);
	int nRight = rcParent.right - 5;

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwndDlg, IDCANCEL), &rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)&rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)(&rc) + 1);
	MoveWindow(GetDlgItem(m_hwndDlg, IDCANCEL), nRight - (rc.right - rc.left), rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	GetWindowRect(GetDlgItem(m_hwndDlg, IDC_EDIT_NAME), &rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)&rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)(&rc) + 1);
	MoveWindow(GetDlgItem(m_hwndDlg, IDC_EDIT_NAME), rc.left, rc.top, nRight - rc.left, rc.bottom - rc.top, TRUE);

	GetWindowRect(GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER), &rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)&rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)(&rc) + 1);
	MoveWindow(GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER), rc.left, rc.top, nRight - rc.left, rc.bottom - rc.top, TRUE);

	if (m_fDlgType == DLG_TYPE_SP) {
		SendMessage(GetDlgItem(m_hwndDlg, IDC_SPIN_FILTER), UDM_SETBUDDY, (WPARAM)GetDlgItem(m_hwndDlg, IDC_LIST_FILTER), 0);
	}

	nRight += 5;
	GetWindowRect(GetDlgItem(m_hwndDlg, IDC_LIST_FILE), &rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)&rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)(&rc) + 1);
	MoveWindow(GetDlgItem(m_hwndDlg, IDC_LIST_FILE), rc.left, rc.top, nRight - rc.left, rc.bottom - rc.top, TRUE);

	CheckListHeight();
}

void CFileDialog::CheckListHeight()
{
	RECT rc, rcParent;
	GetClientRect(m_hwndDlg, &rcParent);
	GetWindowRect(GetDlgItem(m_hwndDlg, IDC_LIST_FILE), &rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)&rc);
	ScreenToClient(m_hwndDlg, (LPPOINT)(&rc) + 1);

	int nHeight = m_helper.IsSipPanelVisible() ? 
		rcParent.bottom - rc.top - 80 : rcParent.bottom - rc.top;

	MoveWindow(GetDlgItem(m_hwndDlg, IDC_LIST_FILE), rc.left, rc.top, 
										rc.right - rc.left, nHeight, TRUE);
}

void CFileDialog::ParseFilter()
{
	int nIndex;
	TCHAR szFilter[MAX_LOADSTRING];
	TCHAR szExt[MAX_LOADSTRING];
	LPTSTR pszSrc, p;
	HWND hwndCombo = GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER);

	if (!m_pofn->lpstrFilter)
		goto fail;

	pszSrc = (LPTSTR)m_pofn->lpstrFilter;
	while (TRUE) {
		// filter
		p = szFilter;
		while (*pszSrc != NULL) {
			*p++ = *pszSrc;
			pszSrc++;
		}
		*p = NULL;
		if (*pszSrc == NULL && *(pszSrc + 1) == NULL)
			break;
		pszSrc++;

		// ext
		p = szExt;
		while (*pszSrc != NULL) {
			*p++ = *pszSrc;
			pszSrc++;
		}
		*p = NULL;
		if (_tcslen(szFilter) && _tcslen(szExt)) {
			int nIndex = SendMessage(hwndCombo, CLB_ADDSTRING, 0, (LPARAM)szFilter);
			if (nIndex == CB_ERR)
				break;
			
			p = new TCHAR[_tcslen(szExt) + 1];
			_tcscpy(p, szExt);
			SendMessage(hwndCombo, CLB_SETITEMDATA, nIndex, (LPARAM)p);
		}
		if (*pszSrc == NULL && *(pszSrc + 1) == NULL)
			break;
		pszSrc++;
	}
	
	nIndex = m_pofn->nFilterIndex > 0 ? m_pofn->nFilterIndex - 1 : 0;
	SendMessage(hwndCombo, CLB_SETCURSEL, nIndex, 0);
	return;
fail:
	ClearFilter();
	LoadString(g_hInst, IDS_DEF_FILTER, szFilter, MAX_LOADSTRING);
	LoadString(g_hInst, IDS_DEF_EXT, szExt, MAX_LOADSTRING);
	p = new TCHAR[_tcslen(szExt) + 1];
	_tcscpy(p, szExt);
	SendMessage(hwndCombo, CLB_ADDSTRING, 0, (LPARAM)szFilter);
	SendMessage(hwndCombo, CLB_SETITEMDATA, 0, (LPARAM)p);
	SendMessage(hwndCombo, CLB_SETCURSEL, 0, 0);
}

void CFileDialog::ClearFilter()
{
	HWND hwndCombo = GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER);
	int n = SendMessage(hwndCombo, CLB_GETCOUNT, 0, 0);
	for (int i = 0; i < n; i++) {
		LPTSTR p = (LPTSTR)SendMessage(hwndCombo, CLB_GETITEMDATA, i, 0);
		delete p;
	}
	SendMessage(hwndCombo, CLB_RESETCONTENT, 0, 0);
}

void CFileDialog::GetShellSettings()
{
	LoadString(g_hInst, IDS_DEF_ROOT_NAME, m_szRootName, MAX_LOADSTRING);

#if 0
	if (m_fDlgType == DLG_TYPE_PPC3)
		return;

	HKEY hKey = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("{000214A0-0000-0000-C000-000000000046}"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwType, dwSize = sizeof(m_szRootName);
		RegQueryValueEx(hKey, _T("DisplayName"), 0, &dwType, (LPBYTE)m_szRootName, &dwSize);
		RegCloseKey(hKey);
	}

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Explorer"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD dwType, dwSize = sizeof(m_fShowExt);
		RegQueryValueEx(hKey, _T("ShowExt"), 0, &dwType, (LPBYTE)&m_fShowExt, &dwSize);
		RegCloseKey(hKey);
	}
#endif
}

void CFileDialog::InitListView()
{
	TCHAR sz[MAX_LOADSTRING];

	if (!(m_pofn->Flags & OFN_ALLOWMULTISELECT) || m_fSave)
		SetWindowLong(m_hwndLV, GWL_STYLE, GetWindowLong(m_hwndLV, GWL_STYLE) | LVS_SINGLESEL);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = sz;

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_nListColumnWidth[0];
	lvc.iSubItem = 0;
	LoadString(g_hInst, IDS_COLUMN_NAME, sz, MAX_LOADSTRING);
	ListView_InsertColumn(m_hwndLV, 0, &lvc);

	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = s_nListColumnWidth[1];
	lvc.iSubItem = 1;
	LoadString(g_hInst, IDS_COLUMN_SIZE, sz, MAX_LOADSTRING);
	ListView_InsertColumn(m_hwndLV, 1, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_nListColumnWidth[2];
	lvc.iSubItem = 2;
	LoadString(g_hInst, IDS_COLUMN_TYPE, sz, MAX_LOADSTRING);
	ListView_InsertColumn(m_hwndLV, 2, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_nListColumnWidth[3];
	lvc.iSubItem = 3;
	LoadString(g_hInst, IDS_COLUMN_DATE, sz, MAX_LOADSTRING);
	ListView_InsertColumn(m_hwndLV, 3, &lvc);

	m_ImageList.Init();
	ListView_SetImageList(m_hwndLV, (HIMAGELIST)m_ImageList, LVSIL_SMALL);
}

void CFileDialog::DestroyListView()
{
	DeleteAllListItem();
	m_ImageList.Destroy();
}

void CFileDialog::DeleteAllListItem()
{
	for (int i = 0; i < ListView_GetItemCount(m_hwndLV); i++) {
		LIST_ITEM_INFO* pDel = GetListItemInfo(i);
		if (pDel->pszDispName) delete[] pDel->pszDispName;
		if (pDel->pszDispSize) delete[] pDel->pszDispSize;
		if (pDel->pszDispType) delete[] pDel->pszDispType;
		if (pDel->pszDispTime) delete[] pDel->pszDispTime;
		delete[] pDel->pszName; 
		delete pDel;
	}
	ListView_DeleteAllItems(m_hwndLV);
}

BOOL CFileDialog::LoadFolderItem(LPCTSTR pszPath, BOOL fFocus)
{
	if (!pszPath)
		return FALSE;

	DWORD dwAttr = GetFileAttributes(pszPath);
	if (dwAttr == 0xFFFFFFFF ||
		!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	ShowWindow(m_hwndLV, SW_HIDE);

	m_fSelMode = FALSE;
	DeleteAllListItem();

	TCHAR sz[MAX_PATH];
	TCHAR szPath[MAX_PATH];
	WIN32_FIND_DATA wfd;

	_tcscpy(szPath, pszPath);
	LPTSTR psz = _tcsrchr(szPath, _T('\\'));
	if (psz && *(psz + 1) == NULL)
		*psz = NULL;

	// folder
	wsprintf(sz, _T("%s\\*.*"), szPath);
	HANDLE hFind = FindFirstFile(sz, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				AddListItem(szPath, &wfd);
			else if (m_listExt.IsEmpty())
				AddListItem(szPath, &wfd);
			else if (IsFolderShortcut(szPath, wfd.cFileName))
				AddListItem(szPath, &wfd);
		}
		while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}

	// files
	if (m_pszFilter) {
		wsprintf(sz, _T("%s\\%s"), szPath, m_pszFilter);
		hFind = FindFirstFile(sz, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					!IsFolderShortcut(szPath, wfd.cFileName))
					AddListItem(szPath, &wfd);
			}
			while (FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}
	else {
		int n = m_listExt.GetCount();
		for (int i = 0; i < n; i++) {
			LPTSTR p = (LPTSTR)m_listExt.GetAt(i);
			wsprintf(sz, _T("%s\\%s"), szPath, p);
			hFind = FindFirstFile(sz, &wfd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						!IsFolderShortcut(szPath, wfd.cFileName))
						AddListItem(szPath, &wfd);
				}
				while (FindNextFile(hFind, &wfd));
				FindClose(hFind);
			}
		}
	}
	
	if (ListView_GetItemCount(m_hwndLV)) {
		SortList();
		ListView_SetItemState(m_hwndLV, 0, LVIS_FOCUSED, LVIS_FOCUSED);
	}
	
	_tcscpy(m_szCurrent, szPath);
	SetDlgItemText(m_hwndDlg, IDC_STATIC_CURRENT_TEXT, GetDisplayName(m_szCurrent));

	BOOL fUp = (_tcslen(szPath) && _tcscmp(szPath, _T("\\")) != 0);
	SendMessage(m_hwndCB, TB_SETSTATE, ID_UP, fUp ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE);

	SetCursor(hCursor);
	ShowWindow(m_hwndLV, SW_SHOW);
	UpdateWindow(m_hwndLV);

	if (fFocus)
		SetFocus(m_hwndLV);
	return TRUE;
}

void CFileDialog::SortList()
{
	m_fSelMode = FALSE;
	switch (m_nListSort)
	{
		case LIST_SORT_NAME:
			ListView_SortItems(m_hwndLV, ListSortCompareFuncByName, m_fSortOrder);
			break;
		case LIST_SORT_EXT:
			ListView_SortItems(m_hwndLV, ListSortCompareFuncByExt, m_fSortOrder);
			break;
		case LIST_SORT_SIZE:
			ListView_SortItems(m_hwndLV, ListSortCompareFuncBySize, m_fSortOrder);
			break;
		case LIST_SORT_TIME:
			ListView_SortItems(m_hwndLV, ListSortCompareFuncByTime, m_fSortOrder);
			break;
		default:
			ListView_SortItems(m_hwndLV, ListSortCompareFuncByName, m_fSortOrder);
			break;
	}
}

void CFileDialog::AddListItem(LPCTSTR pszPath, WIN32_FIND_DATA* pwfd)
{
	LVITEM li;
	LIST_ITEM_INFO* p;

	if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		return;

	if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		p = new LIST_ITEM_INFO;
		p->type = ITEM_TYPE_DIR;
		p->pszName = new TCHAR[_tcslen(pwfd->cFileName) + 1];
		memset(p->pszName, 0, sizeof(TCHAR) * (_tcslen(pwfd->cFileName) + 1));
		_tcscpy(p->pszName, pwfd->cFileName);
		p->llSize = 0;
		p->ft = pwfd->ftCreationTime;
		
		li.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		li.iItem = 0;
		li.iSubItem = 0;
		li.iImage = I_IMAGECALLBACK;
		li.lParam = (DWORD)p;
		li.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(m_hwndLV, &li);
		ListView_SetItemText(m_hwndLV, 0, 2, LPSTR_TEXTCALLBACK);
	}
	else {
		p = new LIST_ITEM_INFO;
		p->type = ITEM_TYPE_FILE;
		p->pszName = new TCHAR[_tcslen(pwfd->cFileName) + 1];
		memset(p->pszName, 0, sizeof(TCHAR) * (_tcslen(pwfd->cFileName) + 1));
		_tcscpy(p->pszName, pwfd->cFileName);
		p->llSize = ((ULONGLONG)pwfd->nFileSizeHigh << 32) | pwfd->nFileSizeLow;
		p->ft = pwfd->ftLastWriteTime;
		FileTimeToLocalFileTime(&p->ft, &p->ft);
		li.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		li.iItem = 0;
		li.iSubItem = 0;
		li.iImage = I_IMAGECALLBACK;
		li.lParam = (DWORD)p;
		li.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(m_hwndLV, &li);
		ListView_SetItemText(m_hwndLV, 0, 1, LPSTR_TEXTCALLBACK);
		ListView_SetItemText(m_hwndLV, 0, 2, LPSTR_TEXTCALLBACK);
		ListView_SetItemText(m_hwndLV, 0, 3, LPSTR_TEXTCALLBACK);
	}
}

LIST_ITEM_INFO* CFileDialog::GetListItemInfo(int nIndex)
{
	LVITEM li;
	li.iItem = nIndex;
	li.iSubItem = 0;
	li.mask = LVIF_PARAM;
	ListView_GetItem(m_hwndLV, &li);
	return (LIST_ITEM_INFO*)li.lParam;
}

int CFileDialog::GetSelectedItemIndex(int nStart)
{
	int nIndex = -1;
	int nItemCount = ListView_GetItemCount(m_hwndLV);
	for (int i = nStart; i < nItemCount; i++)	{
		if (ListView_GetItemState(m_hwndLV, i, LVIS_SELECTED) == LVIS_SELECTED)
			return i;
	}

	return -1;
}

void CFileDialog::OnGetDispInfo(NMLVDISPINFO* pnmdi)
{
	LIST_ITEM_INFO* p = GetListItemInfo(pnmdi->item.iItem);
	if (pnmdi->item.mask & LVIF_IMAGE) {
		if (p->nIcon == -1)
			p->nIcon = m_ImageList.GetImageListIndex(p->pszName, m_szCurrent);
		pnmdi->item.iImage = p->nIcon;
	}
	if ((pnmdi->item.mask & LVIF_TEXT)) {
		if (p->type == ITEM_TYPE_DIR) {
			switch (pnmdi->item.iSubItem) {
				case 0: //Name
					pnmdi->item.pszText = p->pszName;
					break;
				case 2: //Type
				{
					if (!p->pszDispType) {
						TCHAR sz[MAX_PATH];
						SHFILEINFO shfi;
						p->pszDispType = new TCHAR[80]; // ???
						wsprintf(sz, _T("%s\\%s"), m_szCurrent, p->pszName);
						SHGetFileInfo(sz, NULL, &shfi, sizeof(shfi), SHGFI_TYPENAME);
						_tcscpy(p->pszDispType, shfi.szTypeName);
					}
					pnmdi->item.pszText = p->pszDispType;
					break;
				}
				default: break;
			}
		}
		else
		{
			switch (pnmdi->item.iSubItem) {
				case 0: //Name
				{
					if (!p->pszDispName) {
						if (m_fShowExt) {
							pnmdi->item.pszText = p->pszName;
							break;
						}
						else {
							p->pszDispName = new TCHAR[_tcslen(p->pszName) + 1];
							_tcscpy(p->pszDispName, p->pszName);
							LPTSTR psz = _tcsrchr(p->pszDispName, _T('.'));
							if (psz)
								*psz = NULL;
						}
					}
					pnmdi->item.pszText = p->pszDispName;
					break;
				}
				case 1: //Size
				{
					if (!p->pszDispSize) {
						p->pszDispSize = new TCHAR[64];
						SetFormatSize((DWORD)p->llSize, p->pszDispSize, _T("%s KB"), _T("%s MB"));
						// todo : 2^32 bytesˆÈã‚Ìƒtƒ@ƒCƒ‹‘Î‰ž
					}
					pnmdi->item.pszText = p->pszDispSize;
					break;
				}
				case 2: //Type
				{
					if (!p->pszDispType) {
						TCHAR sz[MAX_PATH];
						SHFILEINFO shfi;
						p->pszDispType = new TCHAR[80]; // ???
						wsprintf(sz, _T("%s\\%s"), m_szCurrent, p->pszName);
						SHGetFileInfo(sz, NULL, &shfi, sizeof(shfi), SHGFI_TYPENAME);
						_tcscpy(p->pszDispType, shfi.szTypeName);
					}
					pnmdi->item.pszText = p->pszDispType;
					break;
				}
				case 3: //Date
				{
					if (!p->pszDispTime)
					{
						p->pszDispTime = new TCHAR[32];
						SYSTEMTIME st;
						FileTimeToSystemTime(&p->ft, &st);
						SetFormatDateTime(&st, p->pszDispTime, 32);
					}
					pnmdi->item.pszText = p->pszDispTime;
					break;
				}
				default: break;
			}
		}
	}
}

void CFileDialog::CreateExtList()
{
	DeleteExtList();

	HWND hwndCombo = GetDlgItem(m_hwndDlg, IDC_COMBO_FILTER);
	int nIndex = SendMessage(hwndCombo, CLB_GETCURSEL, 0, 0);
	if (nIndex == CB_ERR)
		return;

	TCHAR sz[MAX_LOADSTRING];
	LPTSTR p;
	LPTSTR pszSrc = (LPTSTR)SendMessage(hwndCombo, CLB_GETITEMDATA, nIndex, 0);
	while (TRUE) {
		p = sz;
		while (*pszSrc != _T(';') && *pszSrc != NULL) {
			*p++ = *pszSrc;
			pszSrc++;
		}
		*p = NULL;
		p = new TCHAR[_tcslen(sz) + 1];
		_tcscpy(p, sz);
		m_listExt.Add((DWORD)p);

		if (*pszSrc == NULL)
			break;
		pszSrc++;
	}
}

void CFileDialog::DeleteExtList()
{
	LPTSTR p;
	while (!m_listExt.IsEmpty()) {
		p = (LPTSTR)m_listExt.RemoveAt(0);
		delete [] p;
	}
}

void CFileDialog::OnCBSelChange()
{
	if (m_pszFilter) {
		delete [] m_pszFilter;
		m_pszFilter = NULL;
	}

	CreateExtList();
	LoadFolderItem(m_szCurrent, FALSE);
}

LPTSTR CFileDialog::GetDisplayName(LPTSTR pszPath)
{
	if (!_tcslen(pszPath) || _tcscmp(pszPath, _T("\\")) == 0)
		return m_szRootName;
	else
		return pszPath;
}

void CFileDialog::ChangeListStyle(DWORD dwNewStyle)
{
	DWORD dwStyle = GetWindowLong(m_hwndLV, GWL_STYLE);
	switch (dwStyle & LVS_TYPEMASK) {
		case LVS_ICON: dwStyle ^= LVS_ICON; break;
		case LVS_SMALLICON: dwStyle ^= LVS_SMALLICON; break;
		case LVS_LIST: dwStyle ^= LVS_LIST; break;
		case LVS_REPORT: dwStyle ^= LVS_REPORT; break;
	}

	dwStyle |= dwNewStyle;
	SetWindowLong(m_hwndLV, GWL_STYLE, dwStyle);

	if (dwNewStyle == LVS_REPORT) {
		ListView_SetExtendedListViewStyle(m_hwndLV, ListView_GetExtendedListViewStyle(m_hwndLV) | 
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT | LVS_EX_TRACKSELECT);
	}
	else {
		ListView_SetExtendedListViewStyle(m_hwndLV, 0);
	}
}

void CFileDialog::OnUp()
{
	TCHAR szPath[MAX_PATH] = _T("");
	if (!_tcslen(m_szCurrent) || _tcscmp(m_szCurrent, _T("\\")) == 0)
		return;

	_tcscpy(szPath, m_szCurrent);
	LPTSTR psz = _tcsrchr(szPath, _T('\\'));
	if (psz) *psz = NULL;
	LoadFolderItem(szPath);
}

void CFileDialog::OnListClick()
{
	if (m_fSelMode)
		SelectToggle();
	else if (!GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(VK_SHIFT) && GetListSelectedCount() < 2)
		OnListDblClk();
}

void CFileDialog::OnListDblClk()
{
	if (m_fSelMode) {
		SelectToggle();
		return;
	}

	int nIndex = GetSelectedItemIndex(0);
	if (nIndex != -1) {
		TCHAR szPath[MAX_PATH];
		LIST_ITEM_INFO* p = GetListItemInfo(nIndex);
		if (p->type == ITEM_TYPE_DIR) {
			wsprintf(szPath, _T("%s\\%s"), m_szCurrent, p->pszName);
			LoadFolderItem(szPath);
		}
		else if (IsFolderShortcut(m_szCurrent, p->pszName)) {
			TCHAR szTarget[MAX_PATH];
			wsprintf(szPath, _T("%s\\%s"), m_szCurrent, p->pszName);
			SHGetShortcutTarget(szPath, szTarget, MAX_PATH);
			LPTSTR p = _tcsrchr(szTarget, _T('\"'));
			if (p) *p = NULL;
			p = (szTarget[0] == _T('\"')) ? szTarget + 1 : szTarget;
			LoadFolderItem(p);
		}
		else if (p->type == ITEM_TYPE_FILE) {
			OnOK();
		}
	}
	else {
		if (ListView_GetItemCount(m_hwndLV) == 1) {
			if (!ListView_GetItemState(m_hwndLV, 0, LVIS_SELECTED)) {
				ListView_SetItemState(m_hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}
}

void CFileDialog::OnListKeyDown(NMLVKEYDOWN* pnmk)
{
	switch (pnmk->wVKey) {
	case VK_BACK:
		OnUp(); break;
	case VK_ESCAPE:
		EndDialog(IDCANCEL); break;
	case VK_TAB:
		SetFocus(GetDlgItem(m_hwndDlg, 
			(GetAsyncKeyState(VK_SHIFT)) ? IDC_COMBO_FILTER :IDC_EDIT_NAME)); break;
	case 'A':
		SelectAllItems();
		break;
	case VK_SPACE:
		if (m_fSelMode) {
			SelectToggle();
		}
		break;
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		if (ListView_GetItemCount(m_hwndLV) == 1) {
			if (!ListView_GetItemState(m_hwndLV, 0, LVIS_SELECTED)) {
				ListView_SetItemState(m_hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
		break;
	}
}

void CFileDialog::OnListItemChanged(NMLISTVIEW* pnmlv)
{
	TCHAR sz[MAX_PATH + 3];
	LIST_ITEM_INFO* p;

	if ((pnmlv->uNewState | pnmlv->uOldState) == LVIS_FOCUSED)
		return;

	HWND hwndEdit = GetDlgItem(m_hwndDlg, IDC_EDIT_NAME);
	int n = GetListSelectedCount();
	if (n == 0) {
		if (!(GetWindowLong(m_hwndLV, GWL_STYLE) & LVS_SINGLESEL))
			SetWindowText(hwndEdit, _T(""));
		return;
	}

	n = 0;
	int nIndex;
	nIndex = GetSelectedItemIndex(0);
	do {
		p = GetListItemInfo(nIndex);
		if (p->type == ITEM_TYPE_FILE && !IsFolderShortcut(m_szCurrent, p->pszName))
			n++;
	}
	while ((nIndex = GetSelectedItemIndex(nIndex + 1)) != -1);
	if (n == 0) {
		if (!(GetWindowLong(m_hwndLV, GWL_STYLE) & LVS_SINGLESEL))
			SetWindowText(hwndEdit, _T(""));
		return;
	}

	if (n > 1) {
		p = GetListItemInfo(pnmlv->iItem);
		if (p->type == ITEM_TYPE_FILE && !IsFolderShortcut(m_szCurrent, p->pszName)) {
			int nLen = GetWindowTextLength(hwndEdit);
			wsprintf(sz, _T("\"%s\" "), p->pszName);
			if (pnmlv->uNewState & LVIS_SELECTED) {
				// add
				if (n == 2) {
					TCHAR sz2[MAX_PATH + 3];
					nIndex = GetSelectedItemIndex(0);
					do {
						p = GetListItemInfo(nIndex);
						if (p->type == ITEM_TYPE_FILE && !IsFolderShortcut(m_szCurrent, p->pszName) &&
							nIndex != pnmlv->iItem)
							break;
					}
					while ((nIndex = GetSelectedItemIndex(nIndex + 1)) != -1);					
					wsprintf(sz2, _T("\"%s\" "), p->pszName);

					LPTSTR psz = new TCHAR[_tcslen(sz) + _tcslen(sz2) + 1];
					if (psz) {
						*psz = NULL;
						_tcscpy(psz, sz2);
						_tcscat(psz, sz);
						SetWindowText(hwndEdit, psz);
						SendMessage(hwndEdit, EM_SETSEL, -1, -1);
						delete [] psz;
					}
				}
				else {
					LPTSTR psz = new TCHAR[nLen + _tcslen(sz) + 1];
					if (psz) {
						GetWindowText(hwndEdit, psz, nLen + 1);
						_tcscat(psz, sz);
						SetWindowText(hwndEdit, psz);
						SendMessage(hwndEdit, EM_SETSEL, -1	, -1);
						delete [] psz;
					}
				}
			}
			else {
				// delete
				LPTSTR psz = new TCHAR[nLen + 1];
				if (psz) {
					GetWindowText(hwndEdit, psz, nLen + 1);
					LPTSTR p = _tcsstr(psz, sz);
					if (p) {
						memmove(p, p + _tcslen(sz), sizeof(TCHAR) * (_tcslen(p + _tcslen(sz)) + 1));
						SetWindowText(hwndEdit, psz);
						SendMessage(hwndEdit, EM_SETSEL, 0, 0);
					}
					delete [] psz;
				}
			}
		}
	}
	else if (n) {
		nIndex = GetSelectedItemIndex(0);
		do {
			p = GetListItemInfo(nIndex);
			if (p->type == ITEM_TYPE_FILE && !IsFolderShortcut(m_szCurrent, p->pszName))
				break;
		}
		while ((nIndex = GetSelectedItemIndex(nIndex + 1)) != -1);
		if (nIndex != -1) {
			p = GetListItemInfo(nIndex);
			if (p->type == ITEM_TYPE_FILE && !IsFolderShortcut(m_szCurrent, p->pszName)) {
				_tcscpy(sz, p->pszName);
				if (!m_fShowExt) {
					LPTSTR psz = _tcsrchr(sz, _T('.'));
					if (psz) {
						if (m_pszDefExt) {
							delete [] m_pszDefExt;
							m_pszDefExt = NULL;
						}
						m_pszDefExt = new TCHAR[_tcslen(psz) + 1];
						_tcscpy(m_pszDefExt, psz);
						*psz = NULL;
					}
				}
				SetWindowText(hwndEdit, sz);
			}
		}
	}
}

void CFileDialog::OnListColumnClick(NMLISTVIEW* pnmlv)
{
	LIST_SORT sort = LIST_SORT_NAME;
	switch (pnmlv->iSubItem) {
		case 0: sort = LIST_SORT_NAME; break;
		case 1: sort = LIST_SORT_SIZE; break;
		case 2: sort = LIST_SORT_EXT; break;
		case 3: sort = LIST_SORT_TIME; break;
	}

	if (sort == m_nListSort)
		m_fSortOrder = !m_fSortOrder;
	
	m_nListSort = sort;
	SortList();
}

void CFileDialog::SelectAllItems()
{
	if (!(GetWindowLong(m_hwndLV, GWL_STYLE) & LVS_SINGLESEL))
		ListView_SetItemState(m_hwndLV, -1, LVIS_SELECTED, LVIS_SELECTED);
}

BOOL CFileDialog::IsFolderShortcut(LPCTSTR pszPath, LPCTSTR pszName)
{
	TCHAR sz[MAX_PATH];
	wsprintf(sz, _T("%s\\%s"), pszPath, pszName);
	_tcsupr(sz);
	
	LPTSTR psz = _tcsrchr(sz, _T('.'));
	if (psz && _tcscmp(psz, _T(".LNK")) == 0) {
		TCHAR szTarget[MAX_PATH];
		wsprintf(sz, _T("%s\\%s"), pszPath, pszName);
		if (SHGetShortcutTarget(sz, szTarget, MAX_PATH)) {
			LPTSTR p = _tcsrchr(szTarget, _T('\"'));
			if (p) *p = NULL;
			p = (szTarget[0] == _T('\"')) ? szTarget + 1 : szTarget;
			DWORD dwAttr = GetFileAttributes(p);
			if (dwAttr != 0xFFFFFFFF && 
				(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
				return TRUE;
		}
	}
	return FALSE;
}

void CFileDialog::OnSelMode()
{
	m_fSelMode = !m_fSelMode;
}

void CFileDialog::SelectToggle()
{
	if (!m_fSelMode)
		return;

	int nCount = ListView_GetItemCount(m_hwndLV);
	for (int i = 0; i < nCount; i++) {
		if (ListView_GetItemState(m_hwndLV, i, LVIS_FOCUSED) == LVIS_FOCUSED) {
			m_fSelMode = FALSE;
			if (ListView_GetItemState(m_hwndLV, i, LVIS_SELECTED) == LVIS_SELECTED) {
				ListView_SetItemState(m_hwndLV, i, 0, LVIS_SELECTED);
			}
			else {
				ListView_SetItemState(m_hwndLV, i, LVIS_SELECTED, LVIS_SELECTED);
			}
			m_fSelMode = TRUE;
			break;
		}
	};
}

BOOL CFileDialog::OnListItemChanging(NM_LISTVIEW* pnmlv)
{
	if (!m_fSelMode)
		return FALSE;

	if ((pnmlv->uNewState & LVIS_SELECTED) != (pnmlv->uOldState & LVIS_SELECTED)) {
		if (pnmlv->uNewState & LVIS_FOCUSED) {
			m_fSelMode = FALSE;
			ListView_SetItemState(m_hwndLV, pnmlv->iItem, pnmlv->uNewState & ~LVIS_SELECTED, 0xFFFF);
			m_fSelMode = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

void CFileDialog::OnInitMenuPopup(HMENU hMenu)
{
	DWORD dwStyle;
	dwStyle = GetWindowLong(m_hwndLV, GWL_STYLE);

	CheckMenuItem(hMenu, ID_KEY_CTRL, MF_BYCOMMAND| (m_fSelMode ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuRadioItem(hMenu, ID_LIST_STYLE_LIST, ID_LIST_STYLE_REPORT, 
		(dwStyle & LVS_TYPEMASK) == LVS_LIST ? ID_LIST_STYLE_LIST : ID_LIST_STYLE_REPORT, MF_BYCOMMAND);
}

void CFileDialog::OnSpinDeltaPos(NMUPDOWN* pnmud)
{
	if (pnmud->hdr.idFrom == IDC_SPIN_FILTER) {
		HWND hwndList = GetDlgItem(m_hwndDlg, IDC_LIST_FILTER);
		int nSel = SendMessage(hwndList, CLB_GETCURSEL, 0, 0);
		int nCount = SendMessage(hwndList, CLB_GETCOUNT, 0, 0);

		if (pnmud->iDelta > 0) {
			nSel = min(nSel + 1, nCount - 1);
		}
		else if (pnmud->iDelta < 0) {
			nSel = max(nSel - 1, 0);
		}
		SendMessage(hwndList, CLB_SETCURSEL, nSel, 0);
		OnCBSelChange();
	}
}

int CFileDialog::GetListSelectedCount()
{
	int nCount = ListView_GetItemCount(m_hwndLV);
	int nSelected = 0;
	for (int i = 0; i < nCount; i++) {
		if (ListView_GetItemState(m_hwndLV, i, LVIS_SELECTED) == LVIS_SELECTED)
			nSelected++;
	}
	return nSelected;
}
