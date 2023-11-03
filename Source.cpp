#include "windows.h"
#include "ScreenCapture.h"
#include "Infer.h"
#include "vm.h"
#include "pid.h"
#include <onnxruntime_cxx_api.h>
//#include <gdiplus.h>
#include <mmsystem.h>
#include "voice.h"
#include <strsafe.h>
#include <thread>

/*
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
*/

//#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "winmm.lib ")

using namespace std;
using namespace cv;
using namespace Ort;
//using namespace Gdiplus;


using s_clock = std::chrono::system_clock;

std::condition_variable cond_var;
std::mutex mtx;

static Voice voice;
static std::atomic<bool>idle = true;

inline wchar_t* dec_func(LPCWSTR input, int key) {
	int n = wcslen(input);
	wchar_t* output = new wchar_t[n + 1];
	for (int i = 0; i < n; i++) {
		wchar_t c = input[i];
		if (iswalpha(c)) {
			wchar_t offset = iswupper(c) ? L'A' : L'a';
			c = (c - offset + key % 26 + 26) % 26 + offset;
		}
		output[i] = c;
	}
	output[n] = L'\0';
	return output;
}

static const wchar_t* ttl = dec_func(L"Tixq Exzxgwl", 111);

/*
static bool prev_phase_fast = false;

void PID_synchronized_phase_change(Pointer & pt,
	PID & pidX,
	PID & pidY,
	bool cur_phase_is_fast,
	const float delta_x,
	const float delta_y) {

	if (cur_phase_is_fast) {
		if (prev_phase_fast) pt.move(pidX.step(delta_x), pidY.step(delta_y));
		else{
			//pidX.clear_stats(); //cleared internally anyways
			//pidY.clear_stats(); //cleared internally anyways
			pidX.clear_time();
			pidY.clear_time();
			pt.move(pidX.step(delta_x), pidY.step(delta_y)); 
			//this will not have any effect, only to reset variables for the upcoming
			prev_phase_fast = true;
		}
	}
	else {
		pt.move(long(delta_x * 0.20F), long(delta_y * 0.20F));
		if (prev_phase_fast) prev_phase_fast = false;
	}
}
*/
/*
void slider_callback(Fl_Widget* widget, void* data) {
	Fl_Box* box = (Fl_Box*)data;
	Fl_Slider* slider = (Fl_Slider*)widget;
	std::string label = std::to_string(slider->value());
	box->copy_label(label.c_str());
}

struct ButtonCallbackParams {
	Fl_Float_Input* input_fields;
	PID& pidX;
	PID& pidY;

	Fl_Slider* slider;
	Infer& net;
};


void ui_thread(PID & pidX, PID & pidY, Infer & net) {
	
	
	Fl_Window* window = new Fl_Window(350, 300, "MIXER");
	Fl_Float_Input inputs[3] = {
		Fl_Float_Input(50, 20, 260, 20, "PRO"),
		Fl_Float_Input(50, 60, 260, 20, "INT"),
		Fl_Float_Input(50, 100, 260, 20, "DEV")
	};
	inputs[0].value(std::to_string(pidX.kp).c_str());
	inputs[1].value(std::to_string(pidX.ki).c_str());
	inputs[2].value(std::to_string(pidX.kd).c_str());

	Fl_Slider* slider = new Fl_Slider(50, 140, 260, 20);
	slider->type(FL_HORIZONTAL);
	slider->bounds(0.5, 0.95);
	slider->step(0.005);
	slider->value(net.confThreshold);
	Fl_Box* box = new Fl_Box(50, 170, 260, 20);
	box->label(std::to_string(net.confThreshold).c_str());	
	slider->callback(slider_callback, box);

	ButtonCallbackParams cb_params = {inputs,pidX,pidY,slider,net};

	Fl_Button* button = new Fl_Button(150, 200, 50, 20, "Enter");
	
	button->callback([](Fl_Widget* widget, void* data) {
			ButtonCallbackParams* callbackData = static_cast<ButtonCallbackParams*>(data);

			callbackData->net.confThreshold = static_cast<float>(callbackData->slider->value());

			Fl_Float_Input* input_kp = &(callbackData->input_fields[0]);
			Fl_Float_Input* input_ki = &(callbackData->input_fields[1]);
			Fl_Float_Input* input_kd = &(callbackData->input_fields[2]);
			// Similarly, retrieve other input fields if needed

			// Extract float values from input fields
			double kp = atof(input_kp->value());
			double ki = atof(input_ki->value());
			double kd = atof(input_kd->value());

			callbackData->pidX.change_parameter(kp, ki, kd);
			callbackData->pidY.change_parameter(kp, ki, kd);

			callbackData->pidX.clear_stats();
			callbackData->pidY.clear_stats();
			callbackData->pidX.clear_time();
			callbackData->pidY.clear_time();

			Sleep(1000);
		}, &cb_params);
		
	
	window->end();
	window->show();
	Fl::run();
	
}
*/

void timer(ScreenCapture& sc) {
	
	s_clock::time_point last = s_clock::time_point{};
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	bool debug = false;
	while (true) {
		
		
		if (GetAsyncKeyState(VK_RBUTTON) < 0) {
			last = s_clock::now();
		}
		else if (GetAsyncKeyState(VK_LBUTTON) < 0) {
			last += chrono::seconds(1);
		}
		else if (GetAsyncKeyState(VK_INSERT)) {
			idle = true;
			voice.speak(L"System paused");
			while (idle) {
				if (GetAsyncKeyState(VK_DELETE)) {
					
					if (::FindWindow(NULL, ttl) == NULL) voice.speak(L"Unavailable");
					else {
						debug = false;
						cv::destroyAllWindows();
						::ShowWindow(::GetConsoleWindow(), SW_HIDE);
						sc.Reset(::FindWindow(NULL, ttl));
						std::unique_lock<std::mutex> lock(mtx);
						idle = false;
						cond_var.notify_one();
						voice.speak(L"System resumed");
						break;
					}
				}
				else if (GetAsyncKeyState(VK_END)) {
					//PlaySound(TEXT("asset/OFF.wav"), NULL, SND_FILENAME | SND_SYNC);
					voice.speak(L"System Offline");
					exit(0);
				}
				else if (GetAsyncKeyState(VK_HOME)) {
					debug = true;
					::ShowWindow(::GetConsoleWindow(), SW_SHOW); 
				}
				else if (debug == true) {
					auto per_last = s_clock::now();
					sc.Capture();
					cout << chrono::duration_cast<std::chrono::microseconds>(s_clock::now() - per_last).count() << endl;
					imshow("frame", sc.src);
					waitKey(1);
				}
				else Sleep(256);
			}
		} 

		if (chrono::duration_cast<std::chrono::milliseconds>(s_clock::now() - last) < chrono::milliseconds(4000)) {
			std::unique_lock<std::mutex> lock(mtx);
			idle = false;
			cond_var.notify_one();
		}
		else {
			idle = true;
		}
		Sleep(32);
	}
}



int main()
{
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	static constexpr float half_fwidth = fwidth / 2;
	static constexpr float half_fheight = fheight / 2;
	//static constexpr float outer_check = 0.0400f * 86528.0f; // very coupled, needs to be change when fwidth, fhieght is changed
	static constexpr float inner_check = 0.01750f * 86528.0f;

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	//::ShowWindow(::GetConsoleWindow(), SW_SHOW);
	voice.speak(L"Loading");
	
	CreateEvent(NULL, FALSE, FALSE, (LPCWSTR)L"Warnung!");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, TEXT("X!"), TEXT("!!!"), MB_OK);
		exit(0);
		return FALSE;

	}

	Net_config Infer_nets = { 0.740f, 0.4f, "asset/yolov7-tiny_reparam_agnostic.onnx" };
	//Net_config Infer_nets = { 0.740f, 0.4f, "asset/new_8_7_2023.onnx" };
	Infer net(Infer_nets);
	PlaySound(TEXT("asset/ON.wav"), NULL, SND_FILENAME | SND_SYNC);
	voice.speak(L"SYSTEM INITIALIZED");
	
	
	while (::FindWindow(NULL, ttl) == NULL)
	{
		if (GetAsyncKeyState(VK_END)) {
			voice.speak(L"System Offline");
			exit(0);
		}
		Sleep(1000);
	}
	

	HWND hwndDesktop = ::FindWindow(NULL, ttl);
	//HWND hwndDesktop = GetDesktopWindow();
	ScreenCapture sc(hwndDesktop);

	thread t1(timer,std::ref(sc));

	voice.speak(L"SC ON!");

	Pointer pt;
	
	//Gdiplus::Graphics graphics(sc.hwindowDC);
	/*
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	Graphics graphics(sc.hwindowDC);
	//graphics.SetCompositingMode(CompositingModeSourceOver);
	graphics.SetCompositingMode(CompositingModeSourceCopy);
	graphics.SetCompositingQuality(CompositingQualityHighSpeed);
	graphics.SetPixelOffsetMode(PixelOffsetModeNone);
	graphics.SetSmoothingMode(SmoothingModeNone);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	FontFamily  fontFamily(L"Times New Roman");
	Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	PointF      pointF1(1800.0f, 500.0f);
	PointF      pointF2(1800.0f, 550.0f);
	PointF      pointF3(1800.0f, 600.0f);
	
	SolidBrush  solidBrushGreen(Color(255, 0, 255, 0));
	SolidBrush  solidBrushRed(Color(255, 255, 0, 0));
	*/
	

	
	PID pidX(1.2500, 0.000024, 15000.0);
	PID pidY(1.2500, 0.000024, 15000.0);

	//t is large then p should be large, i should be low, and d should be large

	//thread another(ui_thread, ref(pidX), ref(pidY), ref(net));
	//cout << "start" << endl;
	
	
	srand(time(NULL));
	double delta_x = 0;
	double delta_y = 0;
	unsigned int at = 0;
	unsigned int n = 0;
	double m_temp = inner_check;
	for (;;) {
			
			while (!idle) {
				n = 0;
				m_temp = inner_check;

				
				sc.Capture();
				//auto per_last = s_clock::now();
				float* pdata = net.detect(sc.src);
				//cout << chrono::duration_cast<std::chrono::microseconds>(s_clock::now() - per_last).count() << endl;

				//cv::imshow("fr", sc.src);
				//cv::waitKey(1);

				if (pdata != nullptr && GetAsyncKeyState(VK_LBUTTON) < 0) {
					do {
						if (pdata[n + 6] > net.confThreshold and !pdata[n + 5]) {
							const double new_delta_x = ((pdata[n + 1] + pdata[n + 3]) * 0.5) - half_fwidth;
							const double new_delta_y = ((pdata[n + 2] + pdata[n + 4]) * 0.5) - half_fheight;
							const double pseudo_dist = (new_delta_x * new_delta_x) + (new_delta_y * new_delta_y);

							if (pseudo_dist < m_temp) {
								m_temp = pseudo_dist;
								delta_x = new_delta_x;
								delta_y = new_delta_y;
								at = n;
							}
						}

						n += 7;

					} while (pdata[n + 6]);

					if (m_temp != inner_check) {
						//PID_synchronized_phase_change(pt, pidX, pidY, m_temp < inner_check, delta_x, delta_y + (pdata[at + 2] - pdata[at + 4]) / 3);
						pt.move(pidX.step(delta_x), pidY.step(delta_y + (pdata[at + 2] - pdata[at + 4]) / (3 + rand() % 3)));
					}
					else {
						pidX.clear_time();
						pidY.clear_time();
					}
				}
				else {
					pidX.clear_time();
					pidY.clear_time();
					//Sleep(1);
				}
			}
			pidX.clear_time();
			pidY.clear_time();
			std::unique_lock<std::mutex> lock(mtx);
			cond_var.wait(lock, [] { return !idle;});
	}
}
