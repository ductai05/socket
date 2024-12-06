#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <time.h>
#include <iostream>
#include <conio.h>
using namespace std;

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
	using namespace Gdiplus;
	UINT num = 0;
	UINT size = 0;

	ImageCodecInfo *pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return 0;
}

void gdiscreen(const int &Height, const int &Width)
{
	using namespace Gdiplus;
	IStream *istream;
	HRESULT res = CreateStreamOnHGlobal(NULL, true, &istream);
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	{
		HDC scrdc, memdc;
		HBITMAP membit;
		scrdc = ::GetDC(0);

		memdc = CreateCompatibleDC(scrdc);
		membit = CreateCompatibleBitmap(scrdc, Width, Height);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);
		BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

		Gdiplus::Bitmap bitmap(membit, NULL);
		CLSID clsid;
		GetEncoderClsid(L"image/jpeg", &clsid);
		bitmap.Save(L"uploads\\screen.jpeg", &clsid, NULL);
		bitmap.Save(istream, &clsid, NULL);
		DeleteObject(memdc);
		DeleteObject(membit);
		::ReleaseDC(0, scrdc);
	}
	GdiplusShutdown(gdiplusToken);
}

int main()
{
	gdiscreen(1880, 2880);
	return 0;
}