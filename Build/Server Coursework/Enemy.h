
//Very simple example of a finite state machine.
//The dinosaur has a hunger and thirst requirements, when
// one is low he will change state and go eat/drink



#pragma once
#include "MazeEntity.h"
#include <functional>
#include <algorithm>

//Callback function for state update
//Params:
//	float dt - timestep
//Return:
typedef std::function<void(float dt)> OnStateUpdateCallback;

class Enemy : public MazeEntity
{
public:
	Enemy();
	~Enemy();
	std::string GetStateName() { return state_name; }
	void SetStateName(string state) { state_name = state; }
protected:
	//Global
	std::string state_name;
};