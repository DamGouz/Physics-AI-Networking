#include "Octree.h"

Octree::Octree(float size, Vector3 start) {
	leaves.push_back(this);
	leaf = true;
	startArea = start;
	endArea = start + Vector3(size, size, size);
	root = false;
	capacity = 200;
	active = false;
}
void Octree::split() {
	if (!leaves.empty())
		leaves.erase(std::find(leaves.begin(), leaves.end(), this));
	leaf = false;
	float l = (endArea.x - startArea.x) / 2;
	nodes[0] = new Octree(l, Vector3(startArea.x, startArea.y, startArea.z));
	nodes[1] = new Octree(l, Vector3((endArea.x + startArea.x)/2, startArea.y, startArea.z));
	nodes[2] = new Octree(l, Vector3(startArea.x, startArea.y, (endArea.z + startArea.z)/2));
	nodes[3] = new Octree(l, Vector3((endArea.x + startArea.x) / 2, startArea.y, (endArea.z + startArea.z) / 2));
	nodes[4] = new Octree(l, Vector3(startArea.x, (endArea.y + startArea.y)/2, startArea.z));
	nodes[5] = new Octree(l, Vector3((endArea.x + startArea.x) / 2, (endArea.y + startArea.y) / 2, startArea.z));
	nodes[6] = new Octree(l, Vector3(startArea.x, (endArea.y + startArea.y) / 2, (endArea.z + startArea.z) / 2));
	nodes[7] = new Octree(l, Vector3((endArea.x + startArea.x) / 2, (endArea.y + startArea.y) / 2, (endArea.z + startArea.z) / 2));
}

void Octree::push(GameObject * obj) {
	if (this->contains(obj)) {
		if (!leaf) {
			for (int i = 0; i < 8; i++) {
				nodes[i]->push(obj);
			}
		}
		else if (leaf && objects.size() < capacity) {
			objects.push_back(obj);
		}
		else {
			this->split();
			for (int i = 0; i < 8; i++) {
				nodes[i]->push(obj);
			}
			
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < capacity; j++)
					nodes[i]->push(objects[j]);
			}
			objects.empty();
		}
	}
}

void Octree::pop(GameObject * obj) {
	if (!leaf) {
		for (int i = 0; i < 8; i++) {
			nodes[i]->pop(obj);
		}
	}
	else {
		if (std::find(objects.begin(), objects.end(), obj) != objects.end())
			objects.erase(std::find(objects.begin(), objects.end(), obj));
	}
}

bool Octree::contains(GameObject * obj) {

	Vector3 circleC = (obj)->Physics()->GetPosition();

	float d_sq = (obj)->Physics()->GetCollisionRadius() * (obj)->Physics()->GetCollisionRadius();
	if (circleC.x < this->startArea.x)
		d_sq -= (circleC.x - this->startArea.x) * (circleC.x - this->startArea.x);
	else if (circleC.x > this->endArea.x)
		d_sq -= (circleC.x - this->endArea.x) * (circleC.x - this->endArea.x);
	if (circleC.y < this->startArea.y)
		d_sq -= (circleC.y - this->startArea.y) * (circleC.y - this->startArea.y);
	else if (circleC.y > this->endArea.y)
		d_sq -= (circleC.y - this->endArea.y) * (circleC.y - this->endArea.y);
	if (circleC.z < this->startArea.z)
		d_sq -= (circleC.z - this->startArea.z) * (circleC.z - this->startArea.z);
	else if (circleC.z > this->endArea.z)
		d_sq -= (circleC.z - this->endArea.z) * (circleC.z - this->endArea.z);
	return d_sq > 0;
}

void Octree::draw() {
	float l = endArea.x - startArea.x;

	NCLDebug::DrawThickLine(startArea, startArea + Vector3(l, 0, 0), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea, startArea + Vector3(0, l, 0), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea, startArea + Vector3(0, 0, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	NCLDebug::DrawThickLine(startArea + Vector3(l, 0, 0), startArea + Vector3(l, l, 0), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea + Vector3(l, 0, 0), startArea + Vector3(l, 0, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	NCLDebug::DrawThickLine(startArea + Vector3(0, l, 0), startArea + Vector3(l, l, 0), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea + Vector3(0, l, 0), startArea + Vector3(0, l, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	NCLDebug::DrawThickLine(startArea + Vector3(l, l, 0), startArea + Vector3(l, l, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	NCLDebug::DrawThickLine(startArea + Vector3(0, 0, l), startArea + Vector3(l, 0, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea + Vector3(0, 0, l), startArea + Vector3(0, l, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	NCLDebug::DrawThickLine(startArea + Vector3(l, 0, l), startArea + Vector3(l, l, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));
	NCLDebug::DrawThickLine(startArea + Vector3(0, l, l), startArea + Vector3(l, l, l), 0.1f, Vector3(0.0f, 0.0f, 0.0f));

	if (nodes[0]) {
		for (int i = 0; i < 8; i++) {
			nodes[i]->draw();
		}
	}
}
void Octree::empty(Octree * rt) {
	if (!rt->root) {
		if (rt->nodes[0]) {
			for (int i = 0; i < 8; i++) {
				empty(rt->nodes[i]);	
			}
		}
		delete rt;
	}
	else {
		if (rt->nodes[0]) {
			for (int i = 0; i < 8; i++) {
				empty(rt->nodes[i]);
			}
		}
		leaves.clear();
		objects.clear();
		leaf = true;
		leaves.push_back(this);
	}
	
}




// In Progress
void Octree::fixTree() {
	int cnt = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j<6; j++)
			nodes[i]->push(objects[j]);
	}
	objects.empty();
}