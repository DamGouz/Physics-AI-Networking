#pragma once
#include <nclgl\Matrix3.h>
#include <ncltech\GameObject.h>
#include <vector>

class Octree {
public:
	Octree(float size, Vector3 start);
	~Octree() {
		this->objects.clear();
	};
	bool isLeaf() { return leaf; }
	void push(GameObject * obj);
	void pop(GameObject * obj);
	static vector <Octree *> leaves;
	vector <GameObject *> objects;

	void fixTree();
	void empty(Octree * rt);
	void draw();
	void setActive(bool ac) { active = ac; }
	bool isActive() { return active; }
	void setRoot(bool t) { root = t; };
protected:
	bool active;
	int capacity;
	bool root;
	Vector3 startArea;
	Vector3 endArea;
	void split();
	Octree * nodes[8];
	bool leaf;
	bool contains(GameObject * obj);

};