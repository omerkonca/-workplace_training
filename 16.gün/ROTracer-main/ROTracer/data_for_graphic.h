#ifndef DFG_H   // bu k�t�phaneye birden fazla yerden �ag�rmak i�in  �simlendirmenini k�saltmas� �eklinde yazd�k burdaki ismi
#define DFG_H    

#include <iostream>
#include <zmq.h>

class TakeDatasString {
public:
    int x;           //x konum
    int y;           // y konum
    float a;           // a��
    int ws;          // yaz�lan h�z
    int rs;          // okunan h�z 
    float wwa;         // yaz�lan teker a��s� 
    float rwa;         // okunan teker a��s� 

};

void Demo_RealtimePlots();  // cpp dosyas�nda bulunan foksiyonu buraya �ag�r�yoruz.  sonras�nda da main taraf�ndan �a��racag�z 
                           
	

#endif