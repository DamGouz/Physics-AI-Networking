#pragma once
#include <ncltech\GameObject.h>
class MazeEntity : public PhysicsNode
{
public:
	MazeEntity();
	~MazeEntity();
	vector<pair <float, float>> line_points;
	bool inactive;
};

