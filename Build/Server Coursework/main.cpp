
/******************************************************************************
Class: SERVER
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <sstream>
#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "MazeEntity.h"
//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include "Enemy.h"

#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second
std::string msg;
MazeGenerator*	generator;
NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
float rotation = 0.0f;
bool maze_generated = false;
void Win32_PrintAllAdapterIPAddresses();
bool play_animation = false;
Vector2 cur_pos;
int counter = 0;
int maze_size = 0;
map <ENetPeer *, MazeEntity *> PlayerMap;
vector <MazeEntity *> Enemies;
stringstream walls;
vector <Enemy *> Ghosts;
int num_ghosts = 4;
int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}
string TruncPacket(enet_uint8 * data, int length) {
	stringstream ss;
	for (int i = 5; i < length - 1; i++) {

		ss << data[i];

	}
	string body = ss.str();
	return body;
}
std::string GetPacketId(enet_uint8 * data) {
	std::stringstream ss;
	ss << data[0] << data[1] << data[2] << data[3];
	return ss.str();
}
string peer_to_string(ENetPeer* peer) {
	stringstream mypeer;
	mypeer << peer;
	return mypeer.str();
}
void BroadcastPacket(string id, string msg, ENetHost * hostConnection) {
	string sock = id + " " + msg;
	ENetPacket* packet = enet_packet_create(sock.c_str(), strlen(sock.c_str()) + 1, 0);
	enet_host_broadcast(hostConnection, 0, packet);
}
void SendPacket(string id, string msg, ENetPeer * peerConnection) {
	string sock = id + " " + msg;
	ENetPacket* packet = enet_packet_create(sock.c_str(), strlen(sock.c_str()) + 1, 0);
	enet_peer_send(peerConnection, 0, packet);
}
void generate_line_points(MazeEntity * M_E, int st, int en, bool first_time) {
	generator->setStart(st);
	generator->setEnd(en);
	generator->search_as->FindBestPath(generator->GetStartNode(), generator->GetGoalNode());
	std::list<const GraphNode*> finalPath = generator->search_as->GetFinalPath();
	generator->GetNavPoints(finalPath, M_E->line_points);
	
		if (M_E->line_points.size() > 1 && !first_time) {
			cur_pos.x = M_E->GetPosition().x;
			cur_pos.y = M_E->GetPosition().z;
			if (M_E->line_points[0].first != M_E->line_points[1].first) {
				if ((cur_pos.x == M_E->line_points[0].first) || (cur_pos.x == M_E->line_points[1].first)) {
					M_E->line_points.insert(M_E->line_points.begin(), make_pair(cur_pos.x, cur_pos.y));
				}
				else if ((cur_pos.x < M_E->line_points[0].first) == (cur_pos.x > M_E->line_points[1].first)) {
					M_E->line_points[0].first = cur_pos.x;
					M_E->line_points[0].second = cur_pos.y;
				}
				else {
					M_E->line_points.insert(M_E->line_points.begin(), make_pair(cur_pos.x, cur_pos.y));
				}
			}
			else if (M_E->line_points[0].second != M_E->line_points[1].second) {
				if ((cur_pos.y == M_E->line_points[0].second) || (cur_pos.y == M_E->line_points[1].second)) {
					M_E->line_points.insert(M_E->line_points.begin(), make_pair(cur_pos.x, cur_pos.y));
				}
				else if ((cur_pos.y < M_E->line_points[0].second) == (cur_pos.y > M_E->line_points[1].second)) {
					M_E->line_points[0].first = cur_pos.x;
					M_E->line_points[0].second = cur_pos.y;
				}
				else {
					M_E->line_points.insert(M_E->line_points.begin(), make_pair(cur_pos.x, cur_pos.y));
				}
			}
		}
		if (M_E->line_points[0] == M_E->line_points[1]) {
			M_E->line_points.erase(M_E->line_points.begin(), M_E->line_points.begin() + 1);
		}
	if (first_time) {
		cur_pos.x = M_E->line_points[0].first;
		cur_pos.y = M_E->line_points[0].second;
		M_E->SetPosition(Vector3(cur_pos.x, 0, cur_pos.y));
	}
	
	Vector2 step = (Vector2(M_E->line_points[1].first, M_E->line_points[1].second) - Vector2(M_E->line_points[0].first, M_E->line_points[0].second));
	if (step.x > 0) step.x = 1;
	else if (step.x < 0) step.x = -1;
	if (step.y > 0) step.y = 1;
	else if (step.y < 0) step.y = -1;
	Vector3 velocity = Vector3(step.x, 0, step.y);
	M_E->SetLinearVelocity(velocity);
}
void add_enemies(int count) {
	for (int i = 0; i < count; i++) {
		Enemy * EN = new Enemy();
		generate_line_points(EN, rand()%maze_size *maze_size + rand() % maze_size, rand() % maze_size *maze_size + rand() % maze_size, EN->inactive);
		EN->inactive = false;
		Ghosts.push_back(EN);
		PhysicsEngine::Instance()->AddPhysicsObject(EN);
	}
}
void GetNextPoint(MazeEntity * Player) {
	Vector2 prev_point = Vector2(Player->line_points[0].first, Player->line_points[0].second);
	Vector2 next_point = Vector2(Player->line_points[1].first, Player->line_points[1].second);
	Vector2 cur_point = Vector2(Player->GetPosition().x, Player->GetPosition().z);
	if (cur_point.x!=prev_point.x || cur_point.y != prev_point.y) {
		int boolx1, booly1, boolx2, booly2;
		if ((double)prev_point.x < (double)next_point.x) boolx1 = 1;
		else if ((double)prev_point.x >(double)next_point.x) boolx1 = -1;
		else boolx1 = 0;
		if ((double)prev_point.y < (double)next_point.y) booly1 = 1;
		else if ((double)prev_point.y >(double)next_point.y) booly1 = -1;
		else booly1 = 0;

		if ((double)cur_point.x < (double)next_point.x) boolx2 = 1;
		else if ((double)cur_point.x >(double)next_point.x) boolx2 = -1;
		else boolx2 = 0;
		if ((double)cur_point.y < (double)next_point.y) booly2 = 1;
		else if ((double)cur_point.y >(double)next_point.y) booly2 = -1;
		else booly2 = 0;

		Vector2 step = (Vector2(Player->line_points[2].first, Player->line_points[2].second) -next_point);
		Vector3 velocity = Vector3(step.x, 0, step.y);
		//cout << boolx1 << " " << booly1 << " " << boolx2 << " " << booly2 << " " << prev_point.x << " " << prev_point.y << " " << cur_point.x  << " " << cur_point.y << " "<< next_point.x <<  " " << next_point.y << endl;

		if ((boolx1 != boolx2 || booly1 != booly2)) {
			Player->SetPosition(Vector3(next_point.x, 0, next_point.y));
			Player->SetLinearVelocity(velocity);
			Player->line_points.erase(Player->line_points.begin());
		}
	}
}

int main(int arcg, char** argv)
{
	srand((unsigned)time(NULL));
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");

	generator = new MazeGenerator();
	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				string other_peers = "";
				if (!PlayerMap.empty()) {
					for (map <ENetPeer *, MazeEntity *>::iterator it = PlayerMap.begin(); it != PlayerMap.end(); ++it) {
						other_peers += " " +peer_to_string(it->first);
						SendPacket("NewA", peer_to_string(evnt.peer), it->first);
					}
					other_peers += " ";
				}
				if (num_ghosts>0) {
					SendPacket("NmGh", to_string(num_ghosts), evnt.peer);
				}
				PlayerMap[evnt.peer] = new MazeEntity();
				PhysicsEngine::Instance()->AddPhysicsObject(PlayerMap[evnt.peer]);
				printf("- New Client Connected\n");

				SendPacket("ExMa", to_string(maze_generated) + " " + peer_to_string(evnt.peer) + other_peers, evnt.peer);
			}
				break;

			case ENET_EVENT_TYPE_RECEIVE:


				if (GetPacketId(evnt.packet->data) == "SiDe") {
					Ghosts.clear();
					
					cout << evnt.peer <<endl;
					string vals = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
					float density;
					int size;
					size = atoi((vals.substr(0, vals.find(" "))).c_str());
					density = atof((vals.substr(vals.find(" ") + 1)).c_str());
							
					generator->Generate(size, density);
					maze_generated = true;
					
					maze_size = size;
					add_enemies(num_ghosts);
					vector <bool> wl;
					wl = generator->GenFlatMaze();
					for (int i = 0; i < (size * 3 - 1)*(size * 3 - 1); ++i) {
						walls<<wl[i];
						if (i < (size * 3 - 1)*(size * 3 - 1) - 1) walls << " ";
					}
					SendPacket("Maze", walls.str(), evnt.peer);
					
				}
				else if (GetPacketId(evnt.packet->data) == "Conn") {
					SendPacket("Maze", walls.str(), evnt.peer);
				}
				else if (GetPacketId(evnt.packet->data) == "StEn") {
					counter = 0;
					string vals = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
					int st;
					int en;
					st = atoi((vals.substr(0, vals.find(" "))).c_str());
					en = atoi((vals.substr(vals.find(" ") + 1)).c_str());
					
					generate_line_points(PlayerMap[evnt.peer], st, en, PlayerMap[evnt.peer]->inactive);
					PlayerMap[evnt.peer]->inactive = false;
					string output = "";
					for (int i = 0; i < PlayerMap[evnt.peer]->line_points.size(); i++) {
						output += to_string(PlayerMap[evnt.peer]->line_points[i].first) + " ";
						output += to_string(PlayerMap[evnt.peer]->line_points[i].second) + " ";
					}
					SendPacket("Line", output, evnt.peer);
				}
				else if (GetPacketId(evnt.packet->data) == "Strt") {
					play_animation = true;
				}
				enet_packet_destroy(evnt.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		});
		
		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP && play_animation)
		{
			accum_time = 0.0f; 
			for (map <ENetPeer *, MazeEntity *>::iterator it = PlayerMap.begin(); it != PlayerMap.end();++it) {
				if (it->second->line_points.size() > 1) {
					GetNextPoint((it->second));	
					BroadcastPacket("Avat", peer_to_string(it->first)+" "+to_string(it->second->GetPosition().x) + " " + to_string(it->second->GetPosition().z), server.m_pNetwork);
				}
			}
			if (!Ghosts.empty()) {
				string ghost_pos = "";
				for (int i = 0; i < num_ghosts; i++) {
					if (Ghosts[i]->GetStateName() == "Patrol") {
						if (Ghosts[i]->line_points.size() > 1) {
							GetNextPoint(Ghosts[i]);
						}
						else if (Ghosts[i]->line_points.size() <= 1) {
							generate_line_points(Ghosts[i], Ghosts[i]->GetPosition().z*maze_size + Ghosts[i]->GetPosition().x, rand() % maze_size *maze_size + rand() % maze_size, false);
						}
						

						for (map <ENetPeer *, MazeEntity *>::iterator it = PlayerMap.begin(); it != PlayerMap.end(); ++it) {
							float fx1, fx2, fy1, fy2;
							int x1, x2, y1, y2;
							fx1 = Ghosts[i]->GetPosition().x;
							fy1 = Ghosts[i]->GetPosition().z;
							fx2 = it->second->GetPosition().x;
							fy2 = it->second->GetPosition().z;
							x1 = int(fx1);
							x2 = int(fx2);
							y1 = int(fy1);
							y2 = int(fy2);
							Vector3 gV = Ghosts[i]->GetLinearVelocity();
							Vector3 pV = it->second->GetLinearVelocity();
							//cout << fx1 << " " << fy1 << " " << fx2 << " " << fy2 << endl;
							if (gV.x == 1) x1 ++;
							else if (gV.z == 1) y1 ++;
							if (pV.x == 1) x2 ++;
							else if (pV.z == 1) y2 ++;

							//cout << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
							if ((fx1==fx2||fy1 == fy2)) {
								if (generator->line_of_sight(x1, y1, x2, y2)) {
									Ghosts[i]->SetStateName("Chasing");
									generate_line_points(Ghosts[i], int(fy1)*maze_size+ int(fx1), int(fy2)*maze_size + int(fx2), false);
								}
							}
						}
					}
					else if (Ghosts[i]->GetStateName() == "Chasing") {
						if (Ghosts[i]->line_points.size() > 1) {
							GetNextPoint(Ghosts[i]);
						}
						else if (Ghosts[i]->line_points.size() <= 1) {
							Ghosts[i]->SetStateName("Patrol");
						}
					}
					ghost_pos += to_string(Ghosts[i]->GetPosition().x) + " " + to_string(Ghosts[i]->GetPosition().z) + " ";
					BroadcastPacket("Ghst", ghost_pos, server.m_pNetwork);
				}
			}
		}
		if(play_animation == true)PhysicsEngine::Instance()->Update(dt);
		Sleep(0);
	}
	
	system("pause");
	server.Release();
}




//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}