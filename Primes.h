#pragma once

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkTetra.h>
#include <vtkPolyDataWriter.h>

#include <array>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iostream>
#include <Eigen/Dense>

#include "Utils.h"

#define N 70000

using namespace std;
using namespace Eigen;

//typedef Matrix<double, 6, 1> Vector6d;
//typedef Matrix<double, 6, 6> Matrix6d;

class IOVertex {
public:
	int id;
	float pa[6];
};

class IOTetra {
public:
	int vs[4];
};

enum State {
	Valid,
	Invalid,
	Border,
	Cut
};

class Prime {
public:

	State state = Valid;
	void kill() {
		state = Invalid;
	}
	bool valid() {
		return state == Valid;
	}
};

class Vertex;
class Tetra;

class QEF {
public:
	QEF() {
		A = Matrix4d::Zero();
		p = Vector4d::Zero();
		e = 0;
	};
	QEF(Matrix4d AA, Vector4d pp, double ee);
	QEF(QEF& Q1, QEF& Q2, Vector4d x);
	void Sum(QEF& Q1, QEF& Q2);
	void Scale(double f);
	double getErr(Vector4d x);

	Matrix4d A;
	Vector4d p;
	double e;


};

class Vertex:public Prime
{
public:
	int id;
	vector<int> tetras;
	QEF Q;
	Vertex() {};
	Vertex(int id, Vector4d pos);

};

class Edge {
public:
	int index[2];
	QEF Q;
	bool operator<(const Edge& other) {
		return Q.e < other.Q.e;
	}
};

class Tetra :public Prime {
public:
	int id;
	int vertexs[4];
	Tetra() {};
	Tetra(int id,int vs[4]);
	void changeVertex(int from, int to);
	bool haveVertex(int vIndex);
};


//vector<Vector6d> smtOrth(vector<Vector6d>A);

bool ConjugateR(Matrix4d A1, Matrix4d A2, Vector4d p1, Vector4d& x);
double square(Matrix4d m, Vector4d v);