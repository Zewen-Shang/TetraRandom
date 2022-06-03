#include "Model.h"

using namespace std;


Model::Model(string fileName,int setSize):fileName(fileName),setSize(setSize)
{

	init();

	readDiv(fileName,0);

	validVertexNum = vertexBuffer.size();
	maxValidPos = tetraBuffer.size() - 1;
	setScale();

	return;
}

Model::Model(string fileName, vector<string> divNames, int setSize) :fileName(fileName), setSize(setSize)
{

	init();

	for (string divName : divNames) {
		readDiv(divName,1);
	}

	validVertexNum = vertexBuffer.size();
	maxValidPos = tetraBuffer.size() - 1;
	setScale();

	return;
}

void Model::outputVtk(string fileName)
{
	string outputPath = outputRoot + fileName + ".vtk";
	vtkNew<vtkUnstructuredGrid>unGrid;
	vtkNew<vtkPoints>points;
	vtkNew<vtkCellArray>cellArray;
	vtkNew<vtkDoubleArray>doubleArray,border;
	doubleArray->SetNumberOfComponents(1);
	border->SetNumberOfComponents(1);
	doubleArray->SetName("attr");
	border->SetName("border");
	for (Vertex v : vertexBuffer) {
		double pos[3];
		if (v.tetras.size() == 0) {
			v.Q.p = Vector4d::Zero();
		}
		//else {
		//	maxErr = max(maxErr, v.Q.e);
		//}
		for (int i = 0; i < 3; i++) {
			pos[i] = v.Q.p(i);
			if (isnan(v.Q.p(i))) {
				cout << "fuck" << endl;
			}
		}
		points->InsertNextPoint(pos);
		doubleArray->InsertNextTuple1(v.Q.p(3));
		switch (v.state)
		{
		case Cut:
			border->InsertNextTuple1(2);
			break;
		case Border:
			border->InsertNextTuple1(0);
			break;
		default:
			border->InsertNextTuple1(1);
			break;
		}
	}


	for (int i = 0; i < tetraBuffer.size(); i++) {
		Tetra t = tetraBuffer[i];
		if (t.state == Invalid) {
			continue;
		}
		vtkNew<vtkTetra> vt;
		for (int j = 0; j < 4; j++) {
			vt->GetPointIds()->SetId(j, t.vertexs[j]);
		}
		cellArray->InsertNextCell(vt);
	}

	unGrid->SetPoints(points);
	unGrid->SetCells(10, cellArray);
	unGrid->GetPointData()->AddArray(doubleArray);
	unGrid->GetPointData()->AddArray(border);
	vtkNew<vtkUnstructuredGridWriter>writer;
	writer->SetInputData(unGrid);
	writer->SetFileName(outputPath.c_str());
	writer->Write();
	
}


void Model::init()
{
	killedTetra = 0;
	avgErr = maxErr = 0;
	for (int i = 0; i < 4; i++) {
		lowBound[i] = 10000;
		upBound[i] = 10000;
	}
}

void Model::outputDiv(string fileName)
{
	ofstream outputFs("./tmpdiv/" + fileName + ".div", ios::binary);
	int type;
	float buffer[7];
	for (int i = 0; i < vertexBuffer.size(); i++) {
		Vertex v = vertexBuffer[i];
		if (v.state != Invalid ) {
			type = 0;
			outputFs.write((char*)&type, sizeof(type));
			*((int*)buffer) = v.id;
			for (int j = 0; j < 4; j++) {
				buffer[j + 1] = v.Q.p[j];
			}
			buffer[5] = buffer[6] = 0;
			buffer[4] /= rate;
			outputFs.write((char*)buffer, sizeof(buffer));
		}
	}
	for (int i = 0; i < tetraBuffer.size(); i++) {
		if (tetraBuffer[i].state == Invalid)continue;
		type = 1;
		outputFs.write((char*)&type, sizeof(type));
		Tetra t = tetraBuffer[i];
		int vs[4];
		for (int i = 0; i < 4; i++) {
			vs[i] = vertexBuffer[t.vertexs[i]].id;
		}
		outputFs.write((char*)vs, sizeof(vs));
	}
}

void Model::readDiv(string divName,bool tmp)
{
	string inputPath;
	if (tmp) {
		inputPath = "./tmpdiv/" + divName + ".div";
	}
	else {
		inputPath = "./div/" + divName + ".div";
	}
	ifstream inputFile(inputPath.c_str(), ios::binary);

	int type;
	int cnt = 0;
	while (inputFile.peek() != EOF) {
		cnt++;
		inputFile.read((char*)&type, sizeof(type));
		Vector4d pos;
		switch (type)
		{
		case 0:
			IOVertex iov;
			inputFile.read((char*)&iov, sizeof(iov));
			if (vMap[iov.id]) {
				continue;
			}
			pos << iov.pa[0], iov.pa[1], iov.pa[2], iov.pa[5];
			for (int i = 0; i < 4; i++) {
				lowBound[i] = min(lowBound[i], (double)iov.pa[i]);
				upBound[i] = max(lowBound[i], (double)iov.pa[i]);
			}
			addVertex(iov.id, pos);
			break;
		case 1:
			IOTetra iot;
			inputFile.read((char*)&iot, sizeof(iot));
			addTetra(iot.vs);
			break;
		case 3:
			int id;
			inputFile.read((char*)&id, sizeof(id));
			vertexBuffer[vMap[id]].state = Cut;
			break;
		default:
			assert(-1);
			break;
		}
	}

}

void Model::setScale()
{
	float posScale = 1;
	for (int i = 0; i < 3; i++) {
		posScale *= upBound[i] - lowBound[i];
	}
	posScale = pow(posScale, 1.0 / 3) / (upBound[3] - lowBound[3]);
	for (int i = 0; i < vertexBuffer.size(); i++) {
		vertexBuffer[i].Q.p[3] *= posScale;
	}
	rate = posScale;
}

int Model::addVertex(int id, Vector4d pos)
{
	Vertex v(id, pos);
	int index = vertexBuffer.size();
	vertexBuffer.push_back(v);
	vMap[id] = index;
	return index;
}

int Model::addTetra(int vs[4])
{

	int vertexs[4];
	int index = tetraBuffer.size();

	for (int i = 0; i < 4; i++) {
		//if (vMap[vs[i]] == 0) {
		//	assert(0);
		//}
		vertexs[i] = vMap[vs[i]];
		vertexBuffer[vertexs[i]].tetras.push_back(index);
	}
	Tetra t(index, vertexs);
	tetraBuffer.push_back(t);
	tetraPos.push_back(index);

	//更新Q
	//以第三个顶点为起点的三个方向向量
	Vector4d direct[3];
	Matrix<double, 3, 4> vM;
	Matrix4d I = Matrix4d::Identity();
	for (int i = 0; i < 3; i++){
		direct[i] = vertexBuffer[vertexs[i]].Q.p - vertexBuffer[vertexs[3]].Q.p;
		vM.row(i) = (direct[i].transpose());
	}
	Vector4d n = Vector4d::Zero();
	for (int i = 0; i < 4; i++) {
		int cnt = 0;
		Matrix3d tmpM = Matrix3d::Zero();
		for (int j = 0; j < 4; j++) {
			if (j == i)continue;
			tmpM.col(cnt++) = vM.col(j);
		}
		n += I.col(i) * pow(-1, i) * tmpM.determinant();
	}

	Vector3d poss[4];
	for (int i = 0; i < 4; i++) {
		poss[i] = vertexBuffer[vertexs[i]].Q.p.block(0, 0, 3, 1);
	}
	double deter = abs(getDetermination(poss));

	Matrix4d A = n * n.transpose();
	A *= 1.5;// / abs(deter);

	for (int i = 0; i < 4; i++) {
		vertexBuffer[vertexs[i]].Q.A += A;
	}

	
	sort(vertexs,vertexs+4);
	tuple<int,int,int> tups[4] = {
		tuple<int,int,int>(vertexs[0],vertexs[1],vertexs[2]),
		tuple<int,int,int>(vertexs[0],vertexs[1],vertexs[3]),
		tuple<int,int,int>(vertexs[0],vertexs[2],vertexs[3]),
		tuple<int,int,int>(vertexs[1],vertexs[2],vertexs[3]),
	};

	for (int i = 0; i < 4; i++) {
		if (borderMap.count(tups[i]) == 1) {
			borderMap.erase(tups[i]);
		}
		else {
			borderMap[tups[i]] = 1;
		}
	}

	return index;
}

void Model::selectBorder()
{
	set<int>cutSet,borderSet;
	for (auto i : borderMap) {
		int vPos[3];
		vPos[0] = std::get<0>(i.first);
		vPos[1] = std::get<1>(i.first);
		vPos[2] = std::get<2>(i.first);
		 

		Vector4d e0 = vertexBuffer[vPos[1]].Q.p - vertexBuffer[vPos[0]].Q.p, e1 = vertexBuffer[vPos[2]].Q.p - vertexBuffer[vPos[0]].Q.p;
		Vector3d a0 = e0.block(0, 0, 3, 1), a1 = e1.block(0, 0, 3, 1);
		double deter = a0.cross(a1).norm();
		e0.normalize();
		e1 = e1 - e0 * (e0.dot(e1));
		e1.normalize();
		Matrix4d A = Matrix4d::Identity();
		A -= (e0 * e0.transpose() + e1 * e1.transpose());
		A *= abs(deter) / 6.0 * 2;

		for (int j = 0; j < 3; j++) {
			if (vertexBuffer[vPos[j]].state != Cut) {
				vertexBuffer[vPos[j]].state = Border;
				vertexBuffer[vPos[j]].Q.A += A;
				borderSet.insert(vPos[j]);
			}
			else {
				cutSet.insert(vPos[j]);
			}
		}
	}

	for (int index = 0; index < tetraPos.size(); index++) {
		int pos = tetraPos[index];
		if (tetraHaveCut(index)) {
			swapTetra(index, tetraBuffer[maxValidPos].id);
			tetraBuffer[maxValidPos].state = Cut;
			maxValidPos--;
		}
	}
}


//rate:要剩下多少
void Model::simplification(double rate)
{
	srand(time(NULL));
	int targerNumber = tetraBuffer.size() * (1-rate);
	int next = 10000;
	int run = 0;
	while (killedTetra < targerNumber && run <= 3000) {
		if (killedTetra >= next) {
			cout << killedTetra << endl;
			next += 10000;
		}
		Edge e = getMin();
		while (isnan(e.Q.p(0)))
		{
			e = getMin();
		}
		if (contractable(e)) {
			directContract(e.index, e.Q);
			run = 0;
		}else{
			run++;
		}
	}
	if (run >= 3000) {
		cout << "run : " << run << endl;
	}
}

bool Model::contractable(Edge e)
{
	vector<int>tIndexs;
	vector<int>inWhich;
	for (int i = 0; i < 2; i++) {
		for (int tIndex : vertexBuffer[e.index[i]].tetras) {
			int tPos = tetraPos[tIndex];
			if (!(tetraBuffer[tPos].haveVertex(e.index[0]) && (tetraBuffer[tPos].haveVertex(e.index[1])))) {
				tIndexs.push_back(tIndex);
				inWhich.push_back(i);
			}
		}
	}

	for (int i = 0; i < tIndexs.size();i++) {
		if (!changable(tIndexs[i], e.index[inWhich[i]], e.Q.p)) {
			return false;
		}
	}

	return true;
}

bool Model::changable(int tIndex, int vIndex, Vector4d newPos)
{
	Tetra t = tetraBuffer[tetraPos[tIndex]];
	Vector3d before[4], after[4];
	int change = -1;
	for (int i = 0; i < 4; i++) {
		if (t.vertexs[i] == vIndex) {
			change = i;
		}
	}
	assert(change != -1);

	for (int i = 0; i < 4; i++) {
		before[i] = after[i] = vertexBuffer[t.vertexs[i]].Q.p.block(0,0,3,1);
		if (i == change) {
			after[i] = newPos.block(0,0,3,1);
		}
	}
	return (getDetermination(before) >= EPS) == (getDetermination(after) >= EPS) ;
}

double Model::getDetermination(Vector3d vectors[4])
{
	Matrix3d M;
	for (int i = 0; i < 3; i++) {
		Vector3d v = (vectors[i] - vectors[3]);
		M.col(i) = v;
	}
	return M.determinant();
}

void Model::directContract(int vIndex[2], QEF Q)
{
	vector<vector<int>> tetraVectors;
	vector<int>V;
	tetraVectors.push_back(V);
	tetraVectors.push_back(V);
	for (int i = 0; i < 2; i++) {
		for (int tIndex : vertexBuffer[vIndex[i]].tetras) {
			tetraVectors[i].push_back(tIndex);
		}
	}
	vector<int>interTetras = Intersction(tetraVectors);
	for (int tIndex : interTetras) {
		if (tetraBuffer[tetraPos[tIndex]].state == Valid) {
			swapTetra(tIndex, tetraBuffer[maxValidPos].id);
			maxValidPos--;
			assert(tetraBuffer[maxValidPos].state == Valid);
			killTetra(tIndex);
		}
		else{
			assert(-1);
		}
	}
	vertexBuffer[vIndex[0]].Q = Q;
	mapVertex(vIndex[1], vIndex[0]);
	validVertexNum--;
}



//交换tetraBuffer里二者的位置，同时切换tetraPos里对应的pos
void Model::swapTetra(int index0, int index1)
{
	swap(tetraBuffer[tetraPos[index0]], tetraBuffer[tetraPos[index1]]);
	swap(tetraPos[index0], tetraPos[index1]);
}

Edge Model::getRandomEdge()
{
	Edge ans;
	int tetraPos = rand() % (maxValidPos);
	assert(tetraBuffer[tetraPos].state != Cut);
	int index[2] = {rand()%4,rand()%4};
	while (index[0] == index[1]) {
		index[1] = rand() % 4;
	}
	ans.index[0] = tetraBuffer[tetraPos].vertexs[index[0]];
	ans.index[1] = tetraBuffer[tetraPos].vertexs[index[1]];
	ans.Q.Sum(vertexBuffer[ans.index[0]].Q, vertexBuffer[ans.index[1]].Q);
	
	return ans;
}

Edge Model::getMin()
{
	static double lastErr = 0;
	bool stored = false;
	Edge minE;
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < setSize; i++) {
			Edge e = getRandomEdge();
			if (!stored) {
				minE = e;
				stored = true;
			}
			else {
				if (e.Q.e < minE.Q.e) {
					minE = e;
				}
			}
		}
		if (lastErr != 0 && ((minE.Q.e - lastErr) / lastErr <= 0.4)) {
			break;
		};
	}
	lastErr = minE.Q.e;
	return minE;
}



void Model::killTetra(int tIndex)
{
	assert(tetraBuffer[tetraPos[tIndex]].state != Invalid);
	int tPos = tetraPos[tIndex];
	tetraBuffer[tPos].state = Invalid;
	for (int i : tetraBuffer[tPos].vertexs) {
		bool erased = false;
		for (auto j = vertexBuffer[i].tetras.begin(); j != vertexBuffer[i].tetras.end(); j++) {
			if (*j == tIndex) {
				vertexBuffer[i].tetras.erase(j);
				erased = true;
				break;
			}
		}
		assert(erased);
	}
	killedTetra++;
}

void Model::mapVertex(int from, int to)
{
	for (int tIndex : vertexBuffer[from].tetras) {
		int tPos = tetraPos[tIndex];
		assert(tetraBuffer[tPos].state != Invalid);
		tetraBuffer[tPos].changeVertex(from, to);
		if (!count(vertexBuffer[to].tetras.begin(), vertexBuffer[to].tetras.begin(), tIndex)) {
			vertexBuffer[to].tetras.push_back(tIndex);
		}
		else {
			assert(-1);
		}
	}
	vertexBuffer[from].state = Invalid;
	vertexBuffer[from].tetras.clear();
}
void Model::print()
{
	
	for (int i = 0; i < vertexBuffer.size();i++) {
		Vertex v = vertexBuffer[i];
		cout << "vertex " << i <<"  :";
		if (v.state == Valid) {
			cout << "  valid  ";
		}
		else {
			cout << "  invalid  ";
		}
		for (auto tIndex : v.tetras) {
			cout << ",  " << tIndex;
		}
		cout << endl;
	}
	for (int index = 0; index < tetraPos.size(); index++) {
		int tPos = tetraPos[index];
		Tetra t = tetraBuffer[tPos];
		cout << "tetra  " << index <<"  :  ";
		if (t.state == Valid) {
			cout << "  valid  ";
		}
		else {
			cout << "  invalid  ";
		}
		for (int vIndex : t.vertexs) {
			cout << " , " << vIndex;
		}
		cout << "\n";
	}
	cout << "——————————————————\n";
}
bool Model::tetraHaveCut(int tIndex)
{
	int tPos = tetraPos[tIndex];
	Tetra t = tetraBuffer[tPos];
	for (int i = 0; i < 4; i++) {
		if (vertexBuffer[t.vertexs[i]].state == Cut) {
			return true;
		}
	}
	return false;
}
void Model::setErr()
{
	for (int i = 0; i < vertexBuffer.size(); i++) {
		if (vertexBuffer[i].state == Invalid)continue;
		maxErr = max(maxErr, vertexBuffer[i].Q.e);
		avgErr += vertexBuffer[i].Q.e;
	}
	avgErr /= validVertexNum;
}
vector<int> Intersction(vector<vector<int>> vectors)
{
	vector<int>ans;
	vector<int>A = vectors[0];
	for (int e : A) {
		bool all = true;
		for (int i = 1; i < vectors.size(); i++) {
			if (!count(vectors[i].begin(), vectors[i].end(), e)) {
				all = false;
				break;
			}
		}
		if (all) {
			ans.push_back(e);
		}
	}
	return ans;
}
