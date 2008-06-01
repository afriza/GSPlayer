#ifndef __FILEIMAGELIST_H__
#define __FILEIMAGELIST_H__

typedef struct _tagImageListIndex
{
	TCHAR szExt[MAX_PATH];
	int nIndex;
}IMAGELISTINDEX;

class CFileImageList
{
public:
	HIMAGELIST m_himl;
	CMultiBuff m_listIndex;

public:
	CFileImageList();
	~CFileImageList();
	void Init();
	void Destroy();
	int GetImageListIndex(LPCTSTR pszFileName, LPCTSTR pszPath);
	int GetImageListIndex(LPCTSTR pszPath);

	operator HIMAGELIST() {return m_himl;}
};

#endif // __FILEIMAGELIST_H__