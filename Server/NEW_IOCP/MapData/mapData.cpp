#include "../stdafx.h"
#include "mapData.h"

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, Vector3 xmf3Scale)
{
    m_nWidth = nWidth;
    m_nLength = nLength;
    m_xmf3Scale = xmf3Scale;

    auto pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

    HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);
    DWORD dwBytesRead;
    ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, nullptr);
    CloseHandle(hFile);

    m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

    for (int y = 0; y < m_nLength; y++)
    {
        for (int x = 0; x < m_nWidth; x++)
        {
            m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
        }
    }
    if (pHeightMapPixels)
        delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
    if (m_pHeightMapPixels)
        delete[] m_pHeightMapPixels;
    m_pHeightMapPixels = nullptr;
}

Vector3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
    if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength))
        return (Vector3(0.0f, 1.0f, 0.0f));

    const int height_map_index = x + (z * m_nWidth);
    const int x_height_map_add = (x < (m_nWidth - 1)) ? 1 : -1;
    const int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
    const float y1 = static_cast<float>(m_pHeightMapPixels[height_map_index]) * m_xmf3Scale.y;
    const float y2 = static_cast<float>(m_pHeightMapPixels[height_map_index + x_height_map_add]) * m_xmf3Scale.y;
    const float y3 = static_cast<float>(m_pHeightMapPixels[height_map_index + zHeightMapAdd]) * m_xmf3Scale.y;
    auto xmf3Edge1 = Vector3(0.0f, y3 - y1, m_xmf3Scale.z);
    auto xmf3Edge2 = Vector3(m_xmf3Scale.x, y2 - y1, 0.0f);
    Vector3 xmf3Normal = Vector3::CrossNormal(xmf3Edge1, xmf3Edge2);

    return (xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
    fx = fx / m_xmf3Scale.x;
    fz = fz / m_xmf3Scale.z;
    if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength))
        return (0.0f);

    int x = static_cast<int>(fx);
    int z = static_cast<int>(fz);
    float fxPercent = fx - x;
    float fzPercent = fz - z;

    float fBottomLeft = m_pHeightMapPixels[x + z * m_nWidth];
    float fBottomRight = m_pHeightMapPixels[x + 1 + z * m_nWidth];
    float fTopLeft = m_pHeightMapPixels[x + (z + 1) * m_nWidth];
    float fTopRight = m_pHeightMapPixels[x + 1 + (z + 1) * m_nWidth];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
    if (bReverseQuad)
    {
        if (fzPercent >= fxPercent)
            fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
        else
            fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
    }
    else
    {
        if (fzPercent < (1.0f - fxPercent))
            fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        else
            fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
    }
#endif
    float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
    float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
    float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

    return (fHeight);
}

float CHeightMapImage::OnGetHeight(int x, int z)
{
    auto pHeightMapImage = this;
    BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
    Vector3 xmf3Scale = pHeightMapImage->GetScale();
    int nWidth = pHeightMapImage->GetHeightMapWidth();
    float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
    return (fHeight);
}
