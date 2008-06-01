#include "GSPlayer2.h"
#include "EffectDlgPPC.h"
#include "SmallSlider.h"

#ifndef _WIN32_WCE_PPC
#error  _WIN32_WCE_PPC is not defined.
#endif

typedef struct _EFFECT_LIST_SPIN_PAIR {
	UINT uEdit;
	UINT uSpin;
}EFFECT_LIST_SPIN_PAIR, *PEFFECT_LIST_SPIN_PAIR;

const EFFECT_LIST_SPIN_PAIR c_EffectLists[] = {
	{IDC_LIST_BASSBOOST, IDC_SPIN_BASSBOOST},
	{IDC_LIST_SURROUND, IDC_SPIN_SURROUND},
	{IDC_LIST_3DCHORUS, IDC_SPIN_3DCHORUS},
	{IDC_LIST_REVERB, IDC_SPIN_REVERB},
	{IDC_LIST_ECHO, IDC_SPIN_ECHO},
	{IDC_LIST_PREAMP, IDC_SPIN_PREAMP}
};


CEffectDlg::CEffectDlg()
{
	m_hMap = NULL;
	m_hwndMB = NULL;
}

CEffectDlg::~CEffectDlg()
{
}

void CEffectDlg::OnInitDialog(HWND hwndDlg)
{
	EFFECT effect;
	EQUALIZER eq;
	ShellInitDialog(hwndDlg);
	SetWindowLong(hwndDlg, DWL_USER, (LONG)ShellInitDlgMenu(hwndDlg, IDR_DONE));

	// BASSBOOST
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_BASSBOOST), 0, 20, MAP_GetBassBoostLevel(m_hMap));
	
	// SURROUND
	MAP_GetEffect(m_hMap, EFFECT_SURROUND, &effect);
	SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_SURROUND), BM_SETCHECK, effect.fEnable ? BST_CHECKED : BST_UNCHECKED, 0);
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_SURROUND), 0, 100, effect.nRate);

	// 3DEFFECT
	MAP_GetEffect(m_hMap, EFFECT_3DCHORUS, &effect);
	SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_3DCHORUS), BM_SETCHECK, effect.fEnable ? BST_CHECKED : BST_UNCHECKED, 0);
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_3DCHORUS), 0, 100, effect.nRate);

	// REVERB
	MAP_GetEffect(m_hMap, EFFECT_REVERB, &effect);
	SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_REVERB), BM_SETCHECK, effect.fEnable ? BST_CHECKED : BST_UNCHECKED, 0);
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_REVERB), 0, 100, effect.nRate);

	// ECHO
	MAP_GetEffect(m_hMap, EFFECT_ECHO, &effect);
	SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_ECHO), BM_SETCHECK, effect.fEnable ? BST_CHECKED : BST_UNCHECKED, 0);
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_ECHO), 0, 100, effect.nRate);

	// PREAMP
	MAP_GetEqualizer(m_hMap, &eq);
	SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_PREAMP), BM_SETCHECK, eq.fEnable ? BST_CHECKED : BST_UNCHECKED, 0);
	InitEffectList(GetDlgItem(hwndDlg, IDC_LIST_PREAMP), -30, +30, 61-eq.preamp);
}

void CEffectDlg::ShowEffectDlg(HWND hwndParent, HANDLE hMap)
{
	m_hMap = hMap;
	DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_EFFECT_DLG), hwndParent, EffectDlgProc, (LPARAM)this);
	m_hMap = NULL;
}

void CEffectDlg::OnSelChanged(HWND hwndDlg, UINT uId)
{
	EFFECT effect;
	EQUALIZER eq;

	switch (uId) {
	case IDC_LIST_BASSBOOST:
		MAP_SetBassBoostLevel(m_hMap, SendMessage(GetDlgItem(hwndDlg, IDC_LIST_BASSBOOST), LB_GETCURSEL, 0, 0));
		break;
	case IDC_LIST_SURROUND:
	case IDC_CHECK_SURROUND:
		MAP_GetEffect(m_hMap, EFFECT_SURROUND, &effect);
		effect.nRate = SendMessage(GetDlgItem(hwndDlg, IDC_LIST_SURROUND), LB_GETCURSEL, 0, 0);
		effect.fEnable = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_SURROUND), BM_GETCHECK, 0, 0) ? TRUE : FALSE;
		MAP_SetEffect(m_hMap, EFFECT_SURROUND, &effect);
		break;
	case IDC_LIST_3DCHORUS:
	case IDC_CHECK_3DCHORUS:
		MAP_GetEffect(m_hMap, EFFECT_3DCHORUS, &effect);
		effect.nRate = SendMessage(GetDlgItem(hwndDlg, IDC_LIST_3DCHORUS), LB_GETCURSEL, 0, 0);
		effect.fEnable = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_3DCHORUS), BM_GETCHECK, 0, 0) ? TRUE : FALSE;
		MAP_SetEffect(m_hMap, EFFECT_3DCHORUS, &effect);
		break;
	case IDC_LIST_REVERB:
	case IDC_CHECK_REVERB:
		MAP_GetEffect(m_hMap, EFFECT_REVERB, &effect);
		effect.nRate = SendMessage(GetDlgItem(hwndDlg, IDC_LIST_REVERB), LB_GETCURSEL, 0, 0);
		effect.fEnable = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_REVERB), BM_GETCHECK, 0, 0) ? TRUE : FALSE;
		MAP_SetEffect(m_hMap, EFFECT_REVERB, &effect);
		break;
	case IDC_LIST_ECHO:
	case IDC_CHECK_ECHO:
		MAP_GetEffect(m_hMap, EFFECT_ECHO, &effect);
		effect.nRate = SendMessage(GetDlgItem(hwndDlg, IDC_LIST_ECHO), LB_GETCURSEL, 0, 0);
		effect.fEnable = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_ECHO), BM_GETCHECK, 0, 0) ? TRUE : FALSE;
		MAP_SetEffect(m_hMap, EFFECT_ECHO, &effect);
		break;
	case IDC_LIST_PREAMP:
	case IDC_CHECK_PREAMP:
		MAP_GetEqualizer(m_hMap, &eq);
		eq.preamp = 61 - SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PREAMP), LB_GETCURSEL, 0, 0);
		eq.fEnable = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_PREAMP), BM_GETCHECK, 0, 0) ? TRUE : FALSE;
		MAP_SetEqualizer(m_hMap, &eq);
		break;
	}
}

void CEffectDlg::OnUpDown(HWND hwndDlg, NMUPDOWN* pnmud)
{
	int nCount, nSel;
	HWND hwndList;
	for (int i = 0; i < sizeof(c_EffectLists) / sizeof(EFFECT_LIST_SPIN_PAIR); i++) {
		if (pnmud->hdr.idFrom == c_EffectLists[i].uSpin) {
			hwndList = GetDlgItem(hwndDlg, c_EffectLists[i].uEdit);
			nSel = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
			nCount = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

			if (pnmud->iDelta > 0) {
				nSel = min(nSel + 1, nCount - 1);
			}
			else if (pnmud->iDelta < 0) {
				nSel = max(nSel - 1, 0);
			}
			SendMessage(hwndList, LB_SETCURSEL, nSel, 0);
			OnSelChanged(hwndDlg, c_EffectLists[i].uEdit);
			break;
		}
	}
}

BOOL CALLBACK CEffectDlg::EffectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CEffectDlg* pDlg;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CEffectDlg*)lParam;
		pDlg->OnInitDialog(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		if (HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == BN_CLICKED) {
			pDlg->OnSelChanged(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		return FALSE;
	case WM_DESTROY:
		ShellDestroyDlgMenu((HWND)GetWindowLong(hwndDlg, DWL_USER));
		return TRUE;
	case WM_SIZE:
		for (int i = 0; i < sizeof(c_EffectLists) / sizeof(EFFECT_LIST_SPIN_PAIR); i++) {
			ShellResizeEditCtrlWidth(GetDlgItem(hwndDlg, c_EffectLists[i].uEdit));
			SendMessage(GetDlgItem(hwndDlg, c_EffectLists[i].uSpin), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, c_EffectLists[i].uEdit), 0);
		}
		return TRUE;
	case WM_NOTIFY:
		if (((NMHDR*)lParam)->code == UDN_DELTAPOS) {
			pDlg->OnUpDown(hwndDlg, (NMUPDOWN*)lParam);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

void CEffectDlg::InitEffectList(HWND hwndList, int nMin, int nMax, int nVal)
{
	TCHAR szTemp[MAX_PATH];
	for (int i = nMin; i <= nMax; i++) {
		wsprintf(szTemp, _T("%d"), i);
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szTemp);
	}
	SendMessage(hwndList, LB_SETCURSEL, nVal, 0);
}