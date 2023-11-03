#include "ScreenCapture.h"
ScreenCapture::ScreenCapture(HWND hwndDesktop) {
	GetWindowRect(hwndDesktop, &windowRect);
	this->x1 = (windowRect.right - windowRect.left - fwidth) / 2;
	this->y1 = (windowRect.bottom - windowRect.top - fheight) / 2;

	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = fwidth;
	bi.bmiHeader.biHeight = -fheight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;

	this->hwnd = hwndDesktop;
	this->hwindowDC = GetDCEx(hwndDesktop,NULL, DCX_WINDOW);
	//this->hwindowDC = GetDC(hwndDesktop);
	this->hmemoryDC = CreateCompatibleDC(hwindowDC);
	this->bitmap = CreateDIBSection(hwindowDC, &bi, DIB_RGB_COLORS, &ptrBitmapPixels, NULL, 0);
	//Note: always create a bitmap that is compatible with hwindowDC, not hmemDC because default hmemory DC 
	//is only initialized with a mono-chromatic 1x1
	SelectObject(this->hmemoryDC, this->bitmap);
		
	this->src = cv::Mat(fheight, fwidth, CV_8UC3, ptrBitmapPixels, 0);
}


void ScreenCapture::Capture() {
	
	
	BitBlt(this->hmemoryDC,
		0, 0,
		fwidth, fheight,
		this->hwindowDC,
		this->x1, this->y1,
		SRCCOPY );

	// SRCCOPY | BITBLT?
	//GetDIBits(hmemoryDC, bitmap, 0, fheight, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
}

ScreenCapture::~ScreenCapture() {
	DeleteDC(this->hmemoryDC);
	DeleteObject(this->bitmap);
	ReleaseDC(this->hwnd,this->hwindowDC);
	src.release();
}

void ScreenCapture::Reset(HWND hwndDesktop) {
	//SelectObject(NULL, this->bitmap); // dont do this, a bitmap can only have one DC at a time
	//DeleteObject(this->bitmap);  // dont do this, have to delete memDC first
	DeleteDC(this->hmemoryDC);
	DeleteObject(this->bitmap);
	ReleaseDC(this->hwnd, this->hwindowDC);
	src.release();

	GetWindowRect(hwndDesktop, &windowRect);
	this->x1 = (windowRect.right - windowRect.left - fwidth) / 2;
	this->y1 = (windowRect.bottom - windowRect.top - fheight) / 2;

	this->hwnd = hwndDesktop;
	this->hwindowDC = GetDCEx(hwndDesktop, NULL, DCX_WINDOW);
	this->hmemoryDC = CreateCompatibleDC(this->hwindowDC);
	this->bitmap = CreateDIBSection(this->hwindowDC, &bi, DIB_RGB_COLORS, &ptrBitmapPixels, NULL, 0);
	SelectObject(this->hmemoryDC, this->bitmap);
	this->src = cv::Mat(fheight, fwidth, CV_8UC3, ptrBitmapPixels, 0);
}