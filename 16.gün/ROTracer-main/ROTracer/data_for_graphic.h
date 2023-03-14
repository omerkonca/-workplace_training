#ifndef ROTRACER_H   // bu k�t�phaneye birden fazla yerden �ag�rmak i�in  �simlendirmenini k�saltmas� �eklinde yazd�k burdaki ismi
#define ROTRACER_H    
// ----------------------------B�T�N TANIMLAMA ��LEMLER�N� BURDA YAPIYORUZ --------------------------
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable : 4996)

#include <iostream>
#include <imgui.h>               // imgui bile�enlerini kullnmak i�in 
#include <implot/implot.h>       // implot bile�enleri i�in 
#include <regex>
#include <string.h>
#include <zmq.h>
#include <thread>
#include <stdio.h>

class AgvData {
public:
	int x;           //x konum
	int y;           // y konum
	float a;           // a��
	int ws;          // yaz�lan h�z
	int rs;          // okunan h�z 
	float wwa;         // yaz�lan teker a��s� 
	float rwa;         // okunan teker a��s� 
};


struct RollingBuffer {  //grafik i�in eklendi 
	float Span;
	ImVector<ImVec2> Data;
	RollingBuffer() {
		Span = 10.0f;
		Data.reserve(2000);
	}
	void AddPoint(float x, float y) {
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
			Data.shrink(0);
		Data.push_back(ImVec2(xmod, y));
	}
};

struct ScrollingBuffer {
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;
	ScrollingBuffer(int max_size = 2000000) {
		MaxSize = max_size;
		Offset = 0;
		Data.reserve(MaxSize);
	}
	void AddPoint(float x, float y) {
		if (Data.size() < MaxSize)
			Data.push_back(ImVec2(x, y));
		else {
			Data[Offset] = ImVec2(x, y);
			Offset = (Offset + 1) % MaxSize;
		}
	}
	void Erase() {
		if (Data.size() > 0) {
			Data.shrink(0);
			Offset = 0;
		}
	}
};

class ROTracer
{
public:                   //constructor 
	ROTracer();
	~ROTracer();

	void SpeedPage();      //fonksiyonlar 
	void LoginPage();

	void StartStreamParser();
	void StopStreamParser();

	AgvData* Agv;          // nesne olu�turduk

	char IpAddress[16] = "192.168.2.125";
	bool _isRunning;


private:
	bool _loginPageVisibility;        //giri� sayfa g�r�n�rl���
	bool _speedPageVisibility;        //h�z grafik sayfas�n�n g�r�n�rl���

	bool _zmqLoopFlag;               //zmq data parse i�lemini yap�p yapmama  

	void ZMQDataStreamParser();       // parse i�lemi private onun i�in burda yoksa yukar� da yazabilirdik
};


#endif