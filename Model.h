#pragma once

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTetra.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkCell.h>
#include <vtkDoubleArray.h>

#include "Primes.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <tuple>
#include <time.h>
#include <assert.h>

using namespace std;

vector<int> Intersction(vector<vector<int>>vectors);


const double EPS = 0;



class Model {
public:
	string fileName;
	string inputRoot = "./div/";
	string outputRoot = "./output/";
	int killedTetra = 0;
	int setSize;
	double avgErr = 0;
	double maxErr = 0;
	double lowBound[4], upBound[4];
	double rate = 1;
	double wGrad = 1;
	Model() {};
	Model(string fileName,int setSize,double wGrad);
	//Model(string fileName, vector<string> divNames, int setSize);
	~Model() {};

	void init();
	void readDiv(string divName,bool tmp);
	void setScale();

	void outputVtk(string fileName);
	void outputDiv(string fileName);
	vector<Vertex>vertexBuffer;
	//�Ӷ���idӳ�䵽����λ��
	map<int, int>vMap;
	vector<Tetra> tetraBuffer;
	vector<int>tetraPos;
	map<tuple<int,int,int>, bool>borderMap;
	int validVertexNum;
	int addVertex(int id, Vector4d pos);
	int addTetra(int vs[4]);
	//��¼rand�Ŀ�ѡ��Χ,���һ����ѡ����ĵ�index
	int maxValidPos = -1;

	void simplification(double rate);
	bool contractable(Edge e);
	bool changable(int tIndex, int vIndex, Vector4d newPos);
	double getDetermination(Vector3d vectors[4]);
	//����ȷ���������������ʱʹ��
	void directContract(int vIndex[2],QEF Q);
	void selectBorder(double borderWeight);
	void swapTetra(int t0, int t1);
	Edge getRandomEdge();
	Edge getMin();
	//��������ض����н�����������Ƴ����������л���vector���
	void killTetra(int tetraIndex);
	void mapVertex(int vIndex0, int vIndex1);
	void print();
	bool tetraHaveCut(int tIndex);

	vector<double>errArr;
	void getError();
	void addQ();
};