#include "Primes.h"

Vertex::Vertex(int id, Vector4d pos)
{
	this->id = id;
	this->Q.p = pos;
}

Tetra::Tetra(int id,int vs[4]):id(id)
{
	for (int i = 0; i < 4; i++) {
		vertexs[i] = vs[i];
	}
}

void Tetra::changeVertex(int from, int to)
{
	int index = -1;
	for (int i = 0; i < 4; i++) {
		if (vertexs[i] == from) {
			index = i;
		}
	}
	assert(index != -1);
	vertexs[index] = to;
}

bool Tetra::haveVertex(int vIndex)
{
	for (int i = 0; i < 4; i++) {
		if (vertexs[i] == vIndex)return true;
	}
	return false;
}

//vector<Vector6d> smtOrth(vector<Vector6d>A) {
//	vector<Vector6d>B;
//	for (int i = 0; i < A.size(); i++) {
//		Vector6d t = A[i];
//		for (int j = 0; j < i; j++) {
//			t -= A[i].dot(B[j]) / B[j].dot(B[j]) * B[j];
//		}
//		B.push_back(t);
//	}
//	return B;
//}


QEF::QEF(Matrix4d AA, Vector4d pp, double ee):A(AA),p(pp),e(ee)
{

}

QEF::QEF(QEF& Q1, QEF& Q2, Vector4d x):A(Q1.A+Q2.A),p(x)
{
	ConjugateR(Q1.A, Q2.A, Q1.p, p);
	e = Q1.e + Q2.e + square(Q1.A,p - Q1.p) + square(Q2.A,p - Q2.p);
}

void QEF::Sum(QEF& Q1, QEF& Q2)
{
	A = Q1.A + Q2.A;
	p = (Q1.p + Q2.p) * 0.5;
	ConjugateR(Q1.A, Q2.A, Q1.p, p);
	e = Q1.e + Q2.e + square(Q1.A,p - Q1.p) + square(Q2.A,p - Q2.p);
}

void QEF::Scale(double f)
{
	A *= f;
	p *= f;
	e *= f;
}

double QEF::getErr(Vector4d x)
{
	return (x-p).transpose() * A * (x-p) + e;
}



bool ConjugateR(Matrix4d A1, Matrix4d A2, Vector4d p1, Vector4d& x)
{
	Matrix4d A = A1 + A2;
	double e = 0;
	for (int i = 0; i < 4; i++) {
		e += A(i,i);
	}
	e = e * 1e-3 / 4;
	Vector4d r((A1 - A2) * (p1 - x));
	Vector4d p;
	for (int k = 0; k < 4; k++)
	{
		double s(r.dot(r));
		//本来是s<=0
		if (s <= 0)
		{
			break;
		}
		p += (r / s);
		Vector4d q = (A * p);
		double t(p.dot(q));
		if (s * t <= e)
		{
			break;
		}
		//后来加上的
		//if (t < 1e-3) {
		//	break;
		//}
		r -= (q / t);
		x += (p / t);
	}

	return false;
}

double square(Matrix4d m,Vector4d v) {
	return v.transpose() * m * v;
}