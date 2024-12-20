#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <time.h>
#include <iostream>
#include <conio.h>

using namespace std;
using namespace Gdiplus;

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
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

void takeScreenshot(const int &Height, const int &Width, int targetHeight, int targetWidth)
{
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

		// Create the bitmap to store the resized image
		Bitmap originalBitmap(membit, NULL);
		Bitmap scaledBitmap(targetWidth, targetHeight, PixelFormat24bppRGB);
		Graphics graphics(&scaledBitmap);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic); // Better quality

		// Draw the scaled image
		graphics.DrawImage(&originalBitmap, 0, 0, targetWidth, targetHeight);

		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);

        scaledBitmap.Save(L"uploads\\screen.png", &clsid, NULL);


        DeleteObject(memdc);
        DeleteObject(membit);
		::ReleaseDC(0, scrdc);
    }
    GdiplusShutdown(gdiplusToken);
}

int main()
{
	const int height = 1880, width = 2880;
    takeScreenshot(height, width, height*60/100, width*60/100);
    return 0;
}