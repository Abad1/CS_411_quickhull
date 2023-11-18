//main.cpp
#include "pbPlots.hpp"
#include "supportLib.hpp"

#include <vector>
using std::vector;

#include <utility>
using std::pair;

#include <random>
using std::uniform_real_distribution;
using std::default_random_engine;

#include <cmath>
using std::sqrt;

#include <iostream>

using std::cout;
using std::endl;
using std::cerr;
using std::wcerr;

#include <chrono>
using namespace std::chrono;


#define coordinate pair<double,double>
#define coordinateList vector<coordinate>

bool isLeft(coordinate a, coordinate b, coordinate c) {
	return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first) > 0;
}

bool isRight(coordinate a, coordinate b, coordinate c) {
	return (a.first - b.first) * (c.second - b.second) - (a.second - b.second) * (c.first - b.first) > 0;
}

double sq(double x) {
	return x * x;
}

coordinate vectorRejection(coordinate a, coordinate b, coordinate c) {
	coordinate ac = { c.first - a.first,c.second - a.second };
	coordinate ab = { b.first - a.first,b.second - a.second };
	coordinate rejection;
	double scale = ((ac.first * ab.first) + (ac.second * ab.second)) / ((ab.first * ab.first) + (ab.second * ab.second));

	ab.first *= scale;
	ab.second *= scale;

	ac.first -= ab.first;
	ac.second -= ab.second;

	return ac;
}

double distance(coordinate a) {

	return sqrt(sq(a.first) + sq(a.second));

}


void recurseHull(coordinate minx, coordinate maxx, coordinateList A, coordinateList& hull) {
	double maxdist = 0;
	coordinate max;
	coordinateList left;
	coordinateList right;

	if (A.size() == 1) {
		hull.push_back(A[0]);
		return;
	}
	if (A.size() == 0){
		return;
	}
	for (auto i : A) {
		double newdist = distance(vectorRejection(minx, maxx, i));
		if (newdist > maxdist) {
			maxdist = newdist;
			max = i;
		}
	}
	hull.push_back(max);
	for (auto i : A) {
		if (isLeft(minx, max, i)) {
			left.push_back(i);
		}
		if (isLeft(max, maxx, i)) {
			right.push_back(i);
		}
	}
	recurseHull(minx, max, left, hull);
	recurseHull(max, maxx, right, hull);
	return;
}
coordinateList quickHull(coordinateList A) {
	coordinate minx = { INFINITY,0 };
	coordinate maxx = { -INFINITY, 0 };
	coordinateList hull;
	coordinateList left;
	coordinateList right;

	for (auto i : A) {
		if (i.first < minx.first)
			minx = i;
		if (i.first > maxx.first)
			maxx = i;
	}
	hull.push_back(minx);
	hull.push_back(maxx);

	for (auto i : A) {
		if (isLeft(minx, maxx, i)) {
			left.push_back(i);
		}
		if (isRight(minx, maxx, i)) {
			right.push_back(i);
		}
	}
	recurseHull(minx, maxx, left, hull);
	recurseHull(maxx, minx, right, hull);
	return hull;
}






coordinateList randomPoints(int size) {
	coordinateList A;
	coordinate p;
	double lower_bound = -1;
	double upper_bound = 1;
	uniform_real_distribution<double> unif(lower_bound, upper_bound);
	default_random_engine re;
	for (int i = 0; i < size; i++) {
		p = { unif(re),unif(re)};
		A.push_back(p);
	}
	return A;
}

void printPoints(coordinateList A) {
	for (auto i : A) {
		cout << "(" << i.first << ", " << i.second << "), ";
	}
}

coordinateList circlePoints(int size) {
	coordinateList p;
	double max = size;
	double angle;
	double j;
	for (int i = 0; i < size; i++) {
		j = i;
		angle = 360 * (j / max);
		p.push_back({ (1 * cos(angle)),(1 * sin(angle)) });
	}
	return p;
}

int main() {

	//coordinateList A = randomPoints(10000);
	coordinateList A = circlePoints(10000);
	vector<double> xs;
	vector<double> ys;
	vector<double> xs2;
	vector<double> ys2;
	for (auto i : A) {
		xs.push_back(i.first);
		ys.push_back(i.second);
	}
	auto start = high_resolution_clock::now();
	A = quickHull(A);
	auto stop = high_resolution_clock::now();
	cout << duration_cast<microseconds>(stop - start).count();
	for (auto i : A) {
		xs2.push_back(i.first);
		ys2.push_back(i.second);
	}
	bool success;
	StringReference* errorMessage = CreateStringReferenceLengthValue(0, L' ');
	RGBABitmapImageReference* imageReference = CreateRGBABitmapImageReference();
	
	
	//quickHull(A);

	ScatterPlotSeries* allPoints = GetDefaultScatterPlotSeriesSettings();
	allPoints->xs = &xs;
	allPoints->ys = &ys;
	allPoints->linearInterpolation = false;
	allPoints->pointType = toVector(L"dots");
	allPoints->color = CreateRGBColor(1, 0, 0);

	ScatterPlotSeries* hullPoints = GetDefaultScatterPlotSeriesSettings();
	hullPoints->xs = &xs2;
	hullPoints->ys = &ys2;
	hullPoints->linearInterpolation = false;
	hullPoints->pointType = toVector(L"dots");
	hullPoints->lineThickness = 2;
	hullPoints->color = CreateRGBColor(0, 0, 1);

	ScatterPlotSettings* settings = GetDefaultScatterPlotSettings();
	settings->width = 600;
	settings->height = 400;
	settings->autoBoundaries = false;
	settings->xMax = 1.1;
	settings->xMin = -1.1;
	settings->yMax = 1.1;
	settings->yMin = -1.1;
	settings->autoPadding = true;
	settings->title = toVector(L"");
	settings->xLabel = toVector(L"");
	settings->yLabel = toVector(L"");
	settings->scatterPlotSeries->push_back(allPoints);
	settings->scatterPlotSeries->push_back(hullPoints);

	success = DrawScatterPlotFromSettings(imageReference, settings, errorMessage);

	if (success) {
		vector<double>* pngdata = ConvertToPNG(imageReference->image);
		WriteToFile(pngdata, "example3.png");
		DeleteImage(imageReference->image);
	}
	else {
		cerr << "Error: ";
		for (wchar_t c : *errorMessage->string) {
			cerr << c;
		}
		cerr << endl;
	}

	FreeAllocations();

	return success ? 0 : 1;

}