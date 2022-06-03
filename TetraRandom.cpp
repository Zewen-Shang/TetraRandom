// TetraRandom.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkUnstructuredGrid.h>
#include <Eigen/Dense>
#include <iostream>
#include <time.h>
#include <chrono>
#include<mpi.h>

#include "Model.h"


#define START_TIME startTime = chrono::system_clock::now();

#define END_TIME(str) endTime = chrono::system_clock::now();\
dur = endTime - startTime;\
second = chrono::duration<double>(dur);\
cout << str <<"Use time : " << second.count() << endl



using namespace std;
using namespace Eigen;
using namespace chrono;

string fileNames[2] = {
	"big0","big1"
};

void testError(){
	double maxTotal = 0, avgTotal = 0;
	for (int i = 0; i < 5; i++) {
		Model m("cylinder1m", 15);
		m.selectBorder();
		m.simplification(0.1);
		//m.outputVtk("box100k0.1");
		m.setErr();
		cout << "Max Err: " << m.maxErr << endl;
		cout << "Avg Err: " << m.avgErr << endl;
		maxTotal += m.maxErr;
		avgTotal += m.avgErr;
	}
	cout << "Max Err: " << maxTotal / 5 << endl;
	cout << "Avg Err: " << avgTotal / 5 << endl;
};


//void testTime(string fileName) {
//	double inputTime = 0, simplyTime = 0, outputTime = 0;
//	const int SIZE = 5;
//	for (int i = 0; i < SIZE; i++) {
//		system_clock::time_point startTime = chrono::system_clock::now();
//		Model m(fileName.c_str(), 15);
//		m.selectBorder();
//		system_clock::time_point endTime = chrono::system_clock::now();
//		auto dur = endTime - startTime;
//		chrono::duration<double> second(dur);
//		inputTime += second.count();
//
//		startTime = system_clock::now();
//		m.simplification(0.1);
//		endTime = system_clock::now();
//		dur = endTime - startTime;
//		second = duration<double>(dur);
//		simplyTime += second.count();
//
//		startTime = system_clock::now();
//		m.outputDiv(fileName + "0");
//		endTime = system_clock::now();
//		dur = endTime - startTime;
//		second = duration<double>(dur);
//		outputTime += second.count();
//	}
//	cout << "input use time : " << inputTime / SIZE << endl;
//	cout << "simply use time : " << simplyTime / SIZE << endl;
//	cout << "output use time : " << outputTime / SIZE << endl;
//
//}


void testSerial(string fileName) {
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point endTime = chrono::system_clock::now();
	auto dur = endTime - startTime;
	chrono::duration<double> second(dur);

	START_TIME;
	Model m(fileName, 10);

	END_TIME("Input");

	START_TIME;
	m.simplification(0.1);
	END_TIME("Simplify");

	START_TIME;
	m.outputDiv(fileName);
	END_TIME("Output");
}


void testParallel(vector<string>divNames) {
	double start, end;
	MPI_Init(NULL, NULL);
	int world_rank = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	cout << "Process " << world_rank << endl;
	start = MPI_Wtime();
	Model m(divNames[world_rank],10);
	m.selectBorder();
	end = MPI_Wtime();
	cout << "Input " << end - start << endl;
	m.simplification(0.5);
	start = end;
	end = MPI_Wtime();
	cout << "Simplification " << end - start << endl;
	m.outputDiv(divNames[world_rank]);
	start = end;
	end = MPI_Wtime();
	cout << "Output " << end - start << endl;
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}


void testMerge(string fileName,vector<string>divNames) {
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point endTime = chrono::system_clock::now();
	auto dur = endTime - startTime;
	chrono::duration<double> second(dur);

	START_TIME;
	Model m(fileName, divNames, 5);
	m.selectBorder();
	END_TIME("Input");

	START_TIME;
	m.simplification(0.2);
	END_TIME("Simplification");

	START_TIME;
	m.outputDiv(fileName);
	END_TIME("Output");
}

int main()
{
	vector<string>divNames;
	divNames.push_back("box100k0");
	divNames.push_back("box100k1");
	divNames.push_back("box100k2");
	divNames.push_back("box100k3");
	testParallel(divNames);

	//testMerge("box100k", divNames);

	//testSerial("box100k");

	//Model m("box1m2", 5);
	//m.selectBorder();
	return 0;
}


