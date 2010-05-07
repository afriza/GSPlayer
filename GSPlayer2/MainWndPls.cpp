#include "GSPlayer2.h"
#include "MainWnd.h"

void RemoveReturn(LPSTR psz)
{
	// ������̍Ō��'\n'���폜
	LPSTR p = psz + strlen(psz) - 1;
	if (*p == '\n') *p = NULL;

	// ������̍Ō��' '���폜
	while (psz[strlen(psz) - 1] == ' ')
		psz[strlen(psz) - 1] = NULL;
}

BOOL CMainWnd::IsPlayList(LPTSTR pszFile)
{
	TCHAR szExt[4];
	LPTSTR psz = _tcsrchr(pszFile, _T('.'));
	if (!psz)
		return FALSE;
	
	_tcsncpy(szExt, ++psz, 4);
	szExt[3] = NULL;
	_tcslwr(szExt);
	return _tcscmp(szExt, M3U_FILE_EXT) == 0 || _tcscmp(szExt, PLS_FILE_EXT) == 0;
}

void CMainWnd::SavePlayList(LPTSTR pszFile)
{
	::DeleteFile(pszFile);
	if (!m_pListFile->GetCount())
		return;

	SaveM3uPlayList(pszFile); // M3U�̂ݑΉ�
}

void CMainWnd::LoadPlayList(LPTSTR pszFile)
{
	CWaitCursor wc;

	TCHAR szExt[4];
	LPTSTR psz = _tcsrchr(pszFile, _T('.'));
	if (!psz)
		return;
	
	_tcsncpy(szExt, ++psz, 4);
	szExt[3] = NULL;
	_tcslwr(szExt);
	if (_tcscmp(szExt, M3U_FILE_EXT) == 0)
		LoadM3uPlayList(pszFile);
	else if (_tcscmp(szExt, PLS_FILE_EXT) == 0)
		LoadPlsPlayList(pszFile);
}

void CMainWnd::SaveM3uPlayList(LPTSTR pszFile)
{
	// M3U�̌`��
	// #EXTM3U
	// #EXTINF:<���t����(�b)>,<�^�C�g��>
	// <�t�@�C���̃p�X>
	// ...

#ifdef _UNICODE
	char szFile[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, NULL, pszFile, -1, szFile, MAX_PATH, NULL, NULL);
	FILE* fp = fopen(szFile, "w");
#else
	FILE* fp = fopen(pszFile, "w");
#endif
	if (!fp) return;

	TCHAR szPath[MAX_PATH];
	_tcscpy(szPath, pszFile);
	LPTSTR psz = _tcsrchr(szPath, _T('\\'));
	if (psz) *(psz + 1) = NULL;

	// �w�b�_���L�q
	fprintf(fp, "#EXTM3U\n");

	// ���X�g���L�q
	for (int i = 0; i < m_pListFile->GetCount(); i++) {
		FILEINFO* p = (FILEINFO*)m_pListFile->GetAt(i);
		if (p) {
			TCHAR szTitle[MAX_PATH];
			GetTitle(i, szTitle);
#ifdef _UNICODE
			TCHAR szBuff[MAX_PATH * 2]; // ...�z���邱�ƂȂ��ł���H

			// #EXTINF:<���t����(�b)>,<�^�C�g��>
			wsprintf(szBuff, _T("#EXTINF:%d,%s\n"), 
				p->info.nDuration == 0 ? -1 : p->info.nDuration / 1000, szTitle);
			WideCharToMultiByte(CP_ACP, NULL, szBuff, -1, szFile, MAX_PATH * 2, NULL, NULL);
			fprintf(fp, szFile);
			
			// <�t�@�C���̃p�X>
			if (_tcsncmp(szPath, p->szPath, _tcslen(szPath)) == 0)
				WideCharToMultiByte(CP_ACP, NULL, p->szPath + _tcslen(szPath), -1, szFile, MAX_PATH * 2, NULL, NULL);
			else
				WideCharToMultiByte(CP_ACP, NULL, p->szPath, -1, szFile, MAX_PATH * 2, NULL, NULL);
			fprintf(fp, szFile);
			fprintf(fp, "\n");
#else
			// #EXTINF:<���t����(�b)>,<�^�C�g��>
			fprintf(fp, "#EXTINF:%d", 
				p->info.nDuration == 0 ? -1 : p->info.nDuration / 1000);
			fprintf(fp, ",%s\n", szTitle);

			// <�t�@�C���̃p�X>
			if (_tcsncmp(szPath, p->szPath, _tcslen(szPath)) == 0)
				fprintf(fp, p->szPath + _tcslen(szPath)); // ���΃p�X
			else
				fprintf(fp, p->szPath); // ��΃p�X
			fprintf(fp, "\n");
#endif
		}
	}
	fclose(fp);
}

void CMainWnd::LoadM3uPlayList(LPTSTR pszFile)
{
	char szBuff[MAX_PATH * 2];
	TCHAR szTitle[MAX_PATH] = {0};
#ifdef _UNICODE
	TCHAR szFile[MAX_PATH];
	WideCharToMultiByte(CP_ACP, NULL, pszFile, -1, szBuff, MAX_PATH, NULL, NULL);
	FILE* fp = fopen(szBuff, "r");
#else
	FILE* fp = fopen(pszFile, "r");
#endif
	if (!fp) return;

	TCHAR szPath[MAX_PATH];
	_tcscpy(szPath, pszFile);
	LPTSTR psz = _tcsrchr(szPath, _T('\\'));
	if (psz) *psz = NULL;

#define EXTINF	"#EXTINF"
	while (fgets(szBuff, MAX_PATH * 2, fp)) {
		RemoveReturn(szBuff);
		if (szBuff[0] == '#') {
			if (strncmp(szBuff, EXTINF, strlen(EXTINF)) == 0) {
				LPSTR p = strchr(szBuff, ',');
				if (p++) {
					while (*p == ' ') p++;
					if (strlen(p)) {
#ifdef _UNICODE
						MultiByteToWideChar(CP_ACP, 0, p, -1, szTitle, MAX_PATH);
#else
						strncpy(szTitle, p, MAX_PATH);
						szTitle[MAX_PATH - 1] = NULL;
#endif
					}
				}
			}
			continue;
		}

		if (!strlen(szBuff))
			continue;
#ifdef _UNICODE
		MultiByteToWideChar(CP_ACP, 0, szBuff, -1, szFile, MAX_PATH);
		AddFile2(szPath, szFile, _tcslen(szTitle) ? szTitle : NULL);
#else
		AddFile2(szPath, szBuff, _tcslen(szTitle) ? szTitle : NULL);
#endif
		szTitle[0] = NULL;
	}
	fclose(fp);
}

void CMainWnd::LoadPlsPlayList(LPTSTR pszFile)
{
	TCHAR sz[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	TCHAR szTitle[MAX_PATH] = {0};
#ifdef _UNICODE
	char szBuff[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, NULL, pszFile, -1, szBuff, MAX_PATH, NULL, NULL);
	FILE* fp = fopen(szBuff, "r");
#else
	FILE* fp = fopen(pszFile, "r");
#endif
	if (!fp) return;

	TCHAR szPath[MAX_PATH];
	_tcscpy(szPath, pszFile);
	LPTSTR psz = _tcsrchr(szPath, _T('\\'));
	if (psz) *psz = NULL;

#define SECTION_PLAYLIST	_T("playlist")
#define KEY_FILE			_T("File%d")
#define KEY_TITLE			_T("Title%d")

	int nIndex = 1;
	while (TRUE) {
		// File
		wsprintf(sz, KEY_FILE, nIndex);
		if (!GetKeyString(fp, SECTION_PLAYLIST, sz, szFile))
			break;

		// Title
		wsprintf(sz, KEY_TITLE, nIndex);
		GetKeyString(fp, SECTION_PLAYLIST, sz, szTitle);
		AddFile2(szPath, szFile, _tcslen(szTitle) ? szTitle: NULL);
		szTitle[0] = NULL;

		nIndex++;
	}

	fclose(fp);
}

void CMainWnd::AddFile2(LPTSTR pszPath, LPTSTR pszFile, LPTSTR pszTitle)
{
	if (IsValidStream(pszFile)) {
		AddFile(pszFile, pszTitle);
		return;
	}

	TCHAR szFile[MAX_PATH];
	wsprintf(szFile, _T("%s\\%s"), pszPath, pszFile);
	AddFile(szFile, pszTitle);
}