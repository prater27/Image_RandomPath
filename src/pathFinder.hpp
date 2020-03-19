#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

#include "../include/MST.h"
#include "../include/Matching/Matching.h"
#include "../include/Matching/Graph.h"
#include "../include/Christofides.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>

using namespace cv;

//https://gist.github.com/cbsmith/5538174
// -*- compile-command: "clang++ -ggdb -o random_selection -std=c++0x -stdlib=libc++ random_selection.cpp" -*-
//Reference implementation for doing random number selection from a container.
//Kept for posterity and because I made a surprising number of subtle mistakes on my first attempt.
#include <random>
#include <iterator>

template <typename RandomGenerator = std::default_random_engine>
struct random_selector
{
	//On most platforms, you probably want to use std::random_device("/dev/urandom")()
	random_selector(RandomGenerator g = RandomGenerator(std::random_device()()))
		: gen(g) {}

	template <typename Iter>
	Iter select(Iter start, Iter end) {
		std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
		std::advance(start, dis(gen));
		return start;
	}

	//convenience function
	template <typename Iter>
	Iter operator()(Iter start, Iter end) {
		return select(start, end);
	}

	//convenience function that works on anything with a sensible begin() and end(), and returns with a ref to the value type
	template <typename Container>
	auto operator()(const Container& c) -> decltype(*begin(c))& {
		return *select(begin(c), end(c));
	}

private:
	RandomGenerator gen;
};


struct pointWithDistance
{
	Point2f p;
	double dist;
	
	pointWithDistance(double x, double y, double distance){
		p.x=x;
		p.y=y;
		dist=distance;
	}
	
	void setDistance(double distance){
			dist=distance;
	}
};


bool cmp(const pointWithDistance &a, const pointWithDistance &b)
{
	return a.dist<b.dist;
}
 
void processNextPoint(std::vector<Point2f> &path, std::vector<pointWithDistance> &pdVector, int n, random_selector<> &selector){
	
	Point2f lastPathPoint=path.back();

	for(unsigned int i=0;i<(pdVector.size());i++){
		pdVector[i].setDistance(pow((lastPathPoint.x - pdVector[i].p.x ),2)+pow((lastPathPoint.y - pdVector[i].p.y),2));
	}
	
	std::sort(pdVector.begin(), pdVector.end(), cmp);
	
	std::vector<pointWithDistance*> closestPoints;
	
	int minim = pdVector.size() < n? pdVector.size() : n;

	for (unsigned int k=0; k<minim; k++){
		closestPoints.push_back(&(pdVector[k]));
	}

	pointWithDistance* pdAuxiliar = selector(closestPoints);
	
	path.push_back((*pdAuxiliar).p);

	pdVector.erase(pdVector.begin() + (pdAuxiliar-&pdVector[0]));
	
	std::cout << "\n left points: " << pdVector.size();
	std::cout << "\n Points in path: " << path.size();


	
}


class pathFinder{

private:
std::vector<Point2f> path;

public:
std::vector<Point2f> getPath(){return path;};

pathFinder(std::vector<pointWithDistance> &pdVector, int n, int totalPoints){
		
	random_selector<> selector{};
	
	path.push_back(selector(pdVector).p);
	
	while(path.size()<(totalPoints-1))
	{
		processNextPoint(path, pdVector, n, selector);
	}
	
	path.push_back(pdVector[0].p);
}
}; //End of the class