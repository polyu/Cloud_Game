#include "iris-int.h"
#include "DirectDrawCollection.h"

CDirectDrawCollection::CDirectDrawCollection(void)
{
}

CDirectDrawCollection::~CDirectDrawCollection(void)
{
}

void CDirectDrawCollection::Add(IDirect3DDevice9* pDevice, CDirectDraw* pDDraw)
{
	auto e = this->find(pDevice);
	if(e != this->end())
	{
		delete e->second;
		this->erase(e);
	}
	else
	{
		(*this)[pDevice] = pDDraw;
	}
}

CDirectDraw* CDirectDrawCollection::Find(IDirect3DDevice9* pDevice)
{
	auto e = this->find(pDevice);
	if(e != this->end())
	{
		return e->second;
	}
	else
	{
		return NULL;
	}
}

void CDirectDrawCollection::Remove(IDirect3DDevice9* pDevice)
{
	auto e = this->find(pDevice);
	if(e != this->end())
	{
		delete e->second;
		this->erase(e);
	}
}
