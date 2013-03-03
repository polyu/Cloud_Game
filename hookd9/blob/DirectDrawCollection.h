#pragma once

typedef stdext::hash_map<IDirect3DDevice9*, CDirectDraw*> CDirectDrawCollectionBase;
typedef stdext::hash_map<IDirect3DDevice9*, CDirectDraw*>::const_iterator CDirectDrawCollectionConstIterator;

class CDirectDrawCollection :
	private CDirectDrawCollectionBase
{
public:
	CDirectDrawCollection(void);
	~CDirectDrawCollection(void);

public:
	void Add(IDirect3DDevice9* pDevice, CDirectDraw* pDDraw);
	CDirectDraw* Find(IDirect3DDevice9* pDevice);
	void Remove(IDirect3DDevice9* pDevice);
};
