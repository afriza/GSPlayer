//PpcFolderDlg.cpp
#include "GSPlayer2.h"
#include "PpcFolderDlg.h"

CPpcFolderDlg::CPpcFolderDlg()
{
	m_hwndMB = NULL;
	m_pOrgProc = NULL;
}

CPpcFolderDlg::~CPpcFolderDlg()
{
}

void CPpcFolderDlg::OnInitDialog(HWND hDlg)
{
	CFolderDlg::OnInitDialog(hDlg);

	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hDlg;
	mbi.nToolBarId = IDR_FOLDERDLG;
	mbi.hInstRes = GetInst();
	SHCreateMenuBar(&mbi);
	m_hwndMB = mbi.hwndMB;

	m_pOrgProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_TREE_FOLDER), 
										GWL_WNDPROC, (LONG)FolderDlgTreeHookProc);
}

void CPpcFolderDlg::OnOK(HWND hDlg)
{
	HWND hTV = GetDlgItem(hDlg, IDC_TREE_FOLDER);
	HTREEITEM hTreeItem = TreeView_GetSelection(hTV);
	GetTree(hTV, hTreeItem, m_pszPath);
	EndDialog(hDlg, IDOK);

	if (m_hwndMB) {
		CommandBar_Destroy(m_hwndMB);
		m_hwndMB = NULL;
	}
	if (m_pOrgProc) {
		SetWindowLong(GetDlgItem(hDlg, IDC_TREE_FOLDER), GWL_WNDPROC, (LONG)m_pOrgProc);
		m_pOrgProc = NULL;
	}
}

void CPpcFolderDlg::OnCancel(HWND hDlg)
{
	CFolderDlg::OnCancel(hDlg);
	if (m_hwndMB) {
		CommandBar_Destroy(m_hwndMB);
		m_hwndMB = NULL;
	}
	if (m_pOrgProc) {
		SetWindowLong(GetDlgItem(hDlg, IDC_TREE_FOLDER), GWL_WNDPROC, (LONG)m_pOrgProc);
		m_pOrgProc = NULL;
	}
}

BOOL CPpcFolderDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (CFolderDlg::OnCommand(hDlg, wParam, lParam))
		return TRUE;

	switch (LOWORD(wParam)) {
	case ID_BTNDOWN:
		OnBtnDown(hDlg, lParam);
		return TRUE;
	case IDM_SUBDIR:
		m_bSubFolder = !m_bSubFolder;
		return TRUE;
	}
	return FALSE;
}

void CPpcFolderDlg::OnInitMenuPopup(HMENU hMenu)
{
	CheckMenuItem(hMenu, IDM_SUBDIR, 
		m_bSubFolder ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);
}

LRESULT CPpcFolderDlg::FolderDlgTreeHookProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CPpcFolderDlg* pDlg = (CPpcFolderDlg*)GetWindowLong(GetParent(hWnd), DWL_USER);

	if (uMsg != WM_KEYUP) 
		return CallWindowProc(pDlg->m_pOrgProc, hWnd, uMsg, wParam, lParam);

	if (wParam != VK_LEFT && wParam != VK_RIGHT) 
		return CallWindowProc(pDlg->m_pOrgProc, hWnd, uMsg, wParam, lParam);

	PostMessage(GetParent(hWnd), WM_COMMAND, ID_BTNDOWN, wParam);
	return CallWindowProc(pDlg->m_pOrgProc, hWnd, uMsg, wParam, lParam);
}

void CPpcFolderDlg::OnBtnDown(HWND hDlg, WPARAM wParam)
{
	TV_ITEM tvi;
	HWND hWnd = GetDlgItem(hDlg, IDC_TREE_FOLDER);
	HTREEITEM hItem = TreeView_GetSelection(hWnd);
	if (!hItem)
		return;

	memset(&tvi, 0, sizeof(tvi));
	tvi.mask = TVIF_CHILDREN | TVIF_STATE;
	tvi.hItem = hItem;
	TreeView_GetItem(hWnd, &tvi);

	if (tvi.cChildren) {
		if (wParam == VK_RIGHT) {
			if (!(tvi.state & TVIS_EXPANDED))
				TreeView_Expand(hWnd, hItem, TVE_EXPAND);
			else {
				hItem = TreeView_GetChild(hWnd, hItem);
				TreeView_SelectItem(hWnd, hItem);
			}
		}
		else {
			if (tvi.state & TVIS_EXPANDED)
				TreeView_Expand(hWnd, hItem, TVE_COLLAPSE);
			else {
				hItem = TreeView_GetParent(hWnd, hItem);
				TreeView_SelectItem(hWnd, hItem);
			}
		}
	}
	else {
		if (wParam == VK_LEFT) {
			hItem = TreeView_GetParent(hWnd, hItem);
			TreeView_SelectItem(hWnd, hItem);
		}
	}
}
