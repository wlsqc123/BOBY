#pragma once

class CHeightMapImage
{
private:
	BYTE* m_pHeightMapPixels;

	int							m_nWidth;
	int							m_nLength;
	Vector3						m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, Vector3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	float OnGetHeight(int x, int z);
	Vector3 GetHeightMapNormal(int x, int z);
	Vector3 GetScale() { return(m_xmf3Scale); }

	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};
