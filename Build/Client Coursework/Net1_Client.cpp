/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!" 
if it successfully connects. The server, will continually broadcast a packet containing a 
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOU'RE BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to 
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on 
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>

const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

string TruncPacket(enet_uint8 * data, int length) {
	stringstream ss;
	for (int i = 5; i < length-1; i++) {

		ss << data[i];

	}
	string body = ss.str();
	return body;
}
string GetPacketId(enet_uint8 * data) {
	stringstream ss;
	ss << data[0] << data[1] << data[2] << data[3];
	return ss.str();
}
void SendPacket(string id, string msg, ENetPeer * serverConnection) {
	string sock = id + " " + msg;
	ENetPacket* packet = enet_packet_create(sock.c_str(), strlen(sock.c_str()) + 1, 0);
	enet_peer_send(serverConnection, 0, packet);
}

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, box(NULL)
{
	wallmesh = new OBJMesh(MESHDIR"cube.obj");
	srand((unsigned)time(NULL));
	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);
	positionchange = Vector2(-1000, 1000);
	start = -1;
	end = -1;
	wallmesh->SetTexture(whitetex);
	size = 0;
	density = 0.0f;
}

void Net1_Client::OnInitializeScene()
{
	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}
}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	box = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}
void Net1_Client::checkSelected() {
	if (positionchange.x != -1000 && positionchange.y != 1000) {
		int x = (positionchange.x / scale + 3 * size - 3) / 6;
		int y = (positionchange.y / scale + 3 * size - 3) / 6;
		int pos = y*size + x;
		if (start == -1) {
			start = pos;
			start_pos = Vector3((x * 6 + 3 - 3 * size)*scale, 0.0f, (y * 6 + 3 - 3 * size)*scale);
			this->AddGameObject(CommonUtils::BuildCuboidObject(
				mypeer,
				Vector3((x * 6 + 3 - 3 * size)*scale, 0.0f, (y * 6 + 3 - 3 * size)*scale),
				Vector3(0.04f, 0.4f, 0.04f),
				true,
				0.0f,
				false,
				true,
				Vector4(0.9f, 0.2f, 0.3f, 1.0f)));
		}
		else if (end != pos && start!=pos) {
			end = pos;
			SendPacket("StEn", to_string(start) + " " + to_string(end), serverConnection);
		}
		positionchange = Vector2(-1000, 1000);
	}
}
void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);
	checkSelected();
	if (!points.empty()) {
		MR->DrawSearchHistory(points, 0.01);
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) { SendPacket("Strt", "", serverConnection); }
	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);



	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	//NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
	//	"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);

	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
}


void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
	//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
		{
			if (evnt.peer == serverConnection)
			{
				NCLDebug::Log(status_color3, "Network: Successfully connected to server!");

			}	
		}
		break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
		{
			if (GetPacketId(evnt.packet->data) == "ExMa")
			{
				
				bool mg;
				string vals = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				int f = vals.find(" ");
				mg = stoi(vals.substr(0, f));
				vals = vals.substr(f+1);
				cout << mg <<endl;
				if (!mg) {
					mypeer = vals;
					cout << "Please enter grid size!" << endl;
					int grid_size;
					cin >> grid_size;
					size = grid_size;
					cout << "Please enter density!" << endl;
					float den;
					cin >> den;
					density = den;
					SendPacket("SiDe", to_string(grid_size)+" "+to_string(den), serverConnection);
				}
				else {
					f = vals.find(" ");
					mypeer = vals.substr(0,f);
					vals = vals.substr(f+1);
					int pnt = 0;
					while (pnt<strlen(vals.c_str())) {
						string c = string(vals.substr(pnt, vals.find_first_of(" ", pnt) - pnt));
						pnt = vals.find_first_of(" ", pnt) + 1;
						this->AddGameObject(CommonUtils::BuildCuboidObject(
							c,
							Vector3(-100.f, 0.0f, -100.f),
							Vector3(0.04f, 0.4f, 0.04f),
							true,
							0.0f,
							false,
							true,
							Vector4(0.3f, 0.2f, 0.8f, 1.0f)));
						all_peers.push_back(c);
					}
					SendPacket("Conn", "", serverConnection);
				}
			}
			else if (GetPacketId(evnt.packet->data) == "NmGh")
			{
				string vals = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				int num_ghosts = stoi(vals);
				for (int i = 0; i < num_ghosts; i++) {
					this->AddGameObject((CommonUtils::BuildCuboidObject(
						"Ghost"+to_string(i),
						Vector3(-100.f, 0.0f, -100.f),
						Vector3(0.04f, 0.4f, 0.04f),
						true,
						0.0f,
						false,
						true,
						Vector4(0.0f, 0.0f, 0.0f, 1.0f))));
				}
			}
			else if (GetPacketId(evnt.packet->data) == "NewA") {
				string new_peer = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				this->AddGameObject(CommonUtils::BuildCuboidObject(
					new_peer,
					Vector3(-100.f, 0.0f, -100.f),
					Vector3(0.04f, 0.4f, 0.04f),
					true,
					0.0f,
					false,
					true,
					Vector4(0.3f, 0.2f, 0.8f, 1.0f)));
					all_peers.push_back(new_peer);
			}
			else if (GetPacketId(evnt.packet->data) == "Maze") {
				string msg = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				size = (1 + sqrt((strlen(msg.c_str()) + 1) / 2)) / 3;
				vector <bool> wl;
				for (int i = 0; i < 2*(size * 3 - 1)*(size * 3 - 1); i+=2) {
					wl.push_back(stoi(msg.substr(i,i+1)));
				}
				
				
				/*for (int i = 0; i < (size * 3 - 1)*(size * 3 - 1); ++i) wl.push_back(walls[i]);*/
				MR = new MazeRenderer(wl,wallmesh);
				Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f, 5.0f / float(size), 5.f)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));

				MR->Render()->SetTransform(Matrix4::Translation(Vector3(0.0f, 0.0f, 0.0f)) * maze_scalar);
				this->AddGameObject(MR);
				
				scale = 2.5 / (3 * (float)size - 1);
				for (int i = 0; i < 3 * size-1; i+=3) {
					for (int j = 0; j < 3 * size-1; j +=3) {
						//cout << wl[j*(3 * size-1) + i];
						if (!wl[(j+2)*(3 * size - 1) + i] && j<3 * size - 3){
							this->AddGameObject(CommonUtils::BuildGround(
								"Ground",
								Vector3(((i+1) * 2 - 3 * size)*scale, 0.0f, ((j+3) * 2 - 3 * size)*scale),
								Vector3(scale, 0.01f, scale),
								positionchange,
								false,
								0.0f,
								false,
								false,
								Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
							this->AddGameObject(CommonUtils::BuildGround(
								"Ground",
								Vector3(((i + 2) * 2 - 3 * size)*scale, 0.0f, ((j +3) * 2 - 3 * size)*scale),
								Vector3(scale, 0.01f, scale),
								positionchange,
								false,
								0.0f,
								false,
								false,
								Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
						}
						if (!wl[(j)*(3 * size - 1) + i+2] && i<3 * size - 3) {
							this->AddGameObject(CommonUtils::BuildGround(
								"Ground",
								Vector3(((i + 3) * 2 - 3 * size)*scale, 0.0f, ((j + 1) * 2 - 3 * size)*scale),
								Vector3(scale, 0.01f, scale),
								positionchange,
								false,
								0.0f,
								false,
								false,
								Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
							this->AddGameObject(CommonUtils::BuildGround(
								"Ground",
								Vector3(((i + 3) * 2 - 3 * size)*scale, 0.0f, ((j + 2) * 2 - 3 * size)*scale),
								Vector3(scale, 0.01f, scale),
								positionchange,
								false,
								0.0f,
								false,
								false,
								Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
						}
					}
					
				}
			
				for (int i = 0; i < size; i++) {
					for (int j = 0; j < size; j++) {
						this->AddGameObject(CommonUtils::BuildGround(
							"Ground",
							Vector3((i * 6 +3 - 3 * size)*scale, 0.0f, (j * 6+3 - 3 * size)*scale),
							Vector3(scale*2, 0.01f, scale*2),
							positionchange,
							false,
							0.0f,
							false,
							true,
							Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
					}
				}
				
			}
			
			else if (GetPacketId(evnt.packet->data) == "Line") {
				int pnt = 0;
				points.clear();
				string msg = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				while (pnt<strlen(msg.c_str())) {
					int x = stoi(msg.substr(pnt, msg.find_first_of(" ", pnt)-pnt));
					pnt = msg.find_first_of(" ", pnt)+1;
					int y = stoi(msg.substr(pnt, msg.find_first_of(" ", pnt) - pnt));
					pnt = msg.find_first_of(" ", pnt) + 1;
					points.push_back(make_pair(x,y));
				}
				
			}
			else if (GetPacketId(evnt.packet->data) == "Avat") {
				string newpos = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				int f = newpos.find(" ");
				string p = newpos.substr(0,f);
				newpos = newpos.substr(f+1);
				f = newpos.find(" ");
				float x = stof(newpos.substr(0,f));
				float y = stof(newpos.substr(f+1));
				start_pos = Vector3((x * 6 + 3 - 3 * size)*scale, 0.0f, (y * 6 + 3 - 3 * size)*scale);
				this->FindGameObject(p)->Physics()->SetPosition(start_pos);
				if (p == mypeer) start = (int)y*size + (int)x;
			}
			else if (GetPacketId(evnt.packet->data) == "Ghst") {
				string newpos = TruncPacket(evnt.packet->data, evnt.packet->dataLength);
				uint cntr = 0;
				while (newpos.length()>1) {
					int f = newpos.find(" ");
					string p = newpos.substr(0, f);
					newpos = newpos.substr(f + 1);
					float x = stof(p);
					f = newpos.find(" ");
					p = newpos.substr(0, f);
					newpos = newpos.substr(f + 1);
					float y = stof(p);
					Vector3 ps = Vector3((x * 6 + 3 - 3 * size)*scale, 0.0f, (y * 6 + 3 - 3 * size)*scale);
					this->FindGameObject("Ghost"+to_string(cntr))->Physics()->SetPosition(ps);
					cntr++;
				}
			}
			else
			{
				cout << "ERROR " << evnt.packet->dataLength << endl;
				NCLERROR("Recieved Invalid Network Packet!");
			}

		}
		break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log(status_color3, "Network: Server has disconnected!");
		}
		break;
	}
}