#pragma once
#include <windows.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#define fwidth 416
#define fheight 416


class ScreenCapture
{
private:
   
    HWND hwnd;
    
    HDC hmemoryDC;
    void* ptrBitmapPixels;
    RECT windowRect;// get the height and width of the screen

    HBITMAP bitmap; // create a bitmap
    BITMAPINFO bi;
    /*
    BITMAPINFOHEADER bi{
        sizeof(BITMAPINFOHEADER),
        fwidth,
        -fheight,
        1,
        24,
        BI_RGB
    };
    */
    int x1;
    int y1;
    //cv::Mat src = cv::Mat(fheight, fwidth, CV_8UC3);
    

public:
    ScreenCapture(HWND hwndDesktop);
    ~ScreenCapture();
    void Capture();
    void Reset(HWND hwndDesktop);
    HDC hwindowDC;
    cv::Mat src;
   
};

