#pragma once
#include <ncltech\GameObject.h>
#include <ncltech\Scene.h>
#include "SearchAlgorithm.h"
#include "SearchAStar.h"

class MazeGenerator
{
public:
	MazeGenerator(); //Maze_density goes from 1 (single path to exit) to 0 (no walls at all)
	virtual ~MazeGenerator();
	void Generate(int size, float maze_density);

	//All points on the maze grid are connected in some shape or form
	// so any two nodes picked randomly /will/ have a path between them
	GraphNode* GetStartNode() const		{ return start; }
	GraphNode* GetGoalNode()  const		{ return end; }
	uint GetSize() const { return size; }
	vector<bool> GenFlatMaze();

	//Used as a hack for the MazeRenderer to generate the walls more effeciently
	GraphNode* GetAllNodesArr() { return allNodes; }
	void setStart(int x) { start = &allNodes[x]; };
	void setEnd(int x) { end = &allNodes[x]; };
	void GetNavPoints(std::list<const GraphNode*> finalPath, vector<pair <float, float>> &line_points);
	bool line_of_sight(int x1, int y1, int x2, int y2);
protected:
	void GetRandomStartEndNodes();

	void Initiate_Arrays();

	void Generate_Prims();
	void Generate_Sparse(float density);
	
public:
	
	SearchAStar* search_as;
	uint size;
	GraphNode *start, *end;
	GraphNode* allNodes;
	GraphEdge* allEdges;

};