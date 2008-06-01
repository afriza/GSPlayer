#ifndef __MULTIBUFF_H__
#define __MULTIBUFF_H__

typedef struct _tagMultiBuffData
{
	struct _tagMultiBuffData* prev;
	DWORD dwData;
	struct _tagMultiBuffData* next;
	
	_tagMultiBuffData()
	{
		prev = NULL;
		dwData = 0;
		next = NULL;
	}
}MULTIBUFFDATA;

typedef int (CALLBACK *LPCOMPAREFUNC)(LPARAM, LPARAM, LPARAM);
typedef BOOL (CALLBACK *LPFINDFUNC)(LPARAM, LPARAM);

class CMultiBuff
{
protected:
	MULTIBUFFDATA* m_Head;
	MULTIBUFFDATA* GetBuff(int iPos);
	
public:
	CMultiBuff();
	~CMultiBuff();

	DWORD GetAt(int iPos);
	void SetAt(int iPos, DWORD dwValue);
	DWORD RemoveAt(int iPos); //At系はデータ数を超えたiPosを指定すると落ちるので注意
	void RemoveAll();
	
	void Insert(DWORD dwValue);
	int Add(DWORD dwValue);

	int GetCount();
	BOOL IsEmpty();
	BOOL Sort(LPCOMPAREFUNC pfnCompare, DWORD dwParam);
	int Find(LPFINDFUNC pfnFind, int iStart, DWORD dwParam);
};

#endif //!__MULTIBUFF_H__