
#pragma once
#include <nclgl\OBJMesh.h>
#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeRenderer.h"
//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;


	void ProcessNetworkEvent(const ENetEvent& evnt);

protected:
	map <string, GameObject*> PlayerMap;
	vector <pair<int, int>> points;
	int start;
	int end;
	Vector3 start_pos;
	Vector2 positionchange;
	void checkSelected();
	GameObject* box;
	int size;
	float density;
	NetworkBase network;
	ENetPeer*	serverConnection;
	MazeRenderer* MR;
	Mesh * wallmesh;
	float scale;
	string mypeer;
	vector<string> all_peers;
};