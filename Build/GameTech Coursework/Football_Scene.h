
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\NCLDebug.h>

class Football_Scene : public Scene
{
public:
	Football_Scene(const std::string& friendly_name)
		: Scene(friendly_name)
	{
		tex = SOIL_load_OGL_texture(
			TEXTUREDIR"football.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		pitch_tex = SOIL_load_OGL_texture(
			TEXTUREDIR"pitch.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		sb = SOIL_load_OGL_texture(
			TEXTUREDIR"goal.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		white = SOIL_load_OGL_texture(
			TEXTUREDIR"white.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	}

	virtual void OnInitializeScene() override
	{
		//Create Ground (..why not?)
		GameObject* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(5.05f, 0.0f, 46.5f),
			Vector3(33.0f, 1.0f, 60.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		(*ground->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(pitch_tex);
		this->AddGameObject(ground);

		

		float x1 = 0.f;
		float y1 = 5.f;
		float z1 = -0.1f;
		//dwh
		float size = 1.0f;

		int w = 15 * size;
		int h = 21 * size;
		int d = 51 * size;

		float step = 0.2 / size;

		for (uint i = 0; i < w; i++) {
			for (uint j = 0; j < h; j++) {
				float mass = 10.0f;
				if (i == 0 || j == h-1 || (i==w-1 && j ==0))
					mass = 0.0f;
				this->AddGameObject(
					CommonUtils::BuildPointObject('s'+ std::to_string(0) +'x' + std::to_string(i) + 'x' + std::to_string(j),
					Vector3(x1, y1 - j*step, z1 - i*step),				//Position
					0.1f,									//Radius
					true,									//Has Physics Object
					mass ,									//Infinite Mass
					true,									//No Collision Shape Yet
					true,									//Dragable by the user
					CommonUtils::GenColor(0.5f, 1.0f)));
			}
		}
		for (uint i = 0; i < w; i++) {
			for (uint j = 0; j < h; j++) {
				vector <GameObject *> balls;
				GameObject * b1 = this->FindGameObject('s' + std::to_string(0) + 'x' + std::to_string(i) + 'x' + std::to_string(j));
				if (i < w-1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(0) + 'x' + std::to_string(i+1) + 'x' + std::to_string(j)));
				}
				if (j < h-1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(0) + 'x' + std::to_string(i) + 'x' + std::to_string(j+1)));
				}
				
				for (uint k = 0; k < balls.size(); k++) {


					DistanceConstraint* constraint = new DistanceConstraint(
						b1->Physics(),					//Physics Object A
						balls[k]->Physics(),					//Physics Object B
						b1->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						balls[k]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
		}
		
		for (uint i = 1; i < d; i++) {
			for (uint j = 0; j < h; j++) {
				float mass = 10.0f;
				if (j == h - 1 || (i%5 == (d-1)%5 && j == 0))
					mass = 0.0f;
				this->AddGameObject(
					CommonUtils::BuildPointObject('s' + std::to_string(i) + 'x' + std::to_string(w-1) + 'x' + std::to_string(j),
						Vector3(x1 + step*i, y1 - j*step, -2.9),				//Position
						0.1f,									//Radius
						true,									//Has Physics Object
						mass,									//Infinite Mass
						true,									//No Collision Shape Yet
						true,									//Dragable by the user
						CommonUtils::GenColor(0.5f, 1.0f)));
			}
		}
		for (uint i = 0; i < d; i++) {
			for (uint j = 0; j < h; j++) {
				vector <GameObject *> balls;
				GameObject * b1 = this->FindGameObject('s' + std::to_string(i) + 'x' + std::to_string(w-1) + 'x' + std::to_string(j));
				if (i < d - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(i+1) + 'x' + std::to_string(w-1) + 'x' + std::to_string(j)));
				}
				if (j < h - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(i) + 'x' + std::to_string(w-1) + 'x' + std::to_string(j+1)));
				}

				for (uint k = 0; k < balls.size(); k++) {


					DistanceConstraint* constraint = new DistanceConstraint(
						b1->Physics(),					//Physics Object A
						balls[k]->Physics(),					//Physics Object B
						b1->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						balls[k]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
		}
		for (uint i = 0; i < w-1; i++) {
			for (uint j = 0; j < h; j++) {
				float mass = 10.0f;
				if (i == 0 || j == h - 1)
					mass = 0.0f;
				this->AddGameObject(
					CommonUtils::BuildPointObject('s' + std::to_string(d-1) + 'x' + std::to_string(i) + 'x' + std::to_string(j),
						Vector3(10.0f, y1 - j*step, z1 - i*step),				//Position
						0.1f,									//Radius
						true,									//Has Physics Object
						mass,									//Infinite Mass
						true,									//No Collision Shape Yet
						true,									//Dragable by the user
						CommonUtils::GenColor(0.5f, 1.0f)));
			}
		}
		for (uint i = 0; i < w-1; i++) {
			for (uint j = 0; j < h-1; j++) {
				vector <GameObject *> balls;
				GameObject * b1 = this->FindGameObject('s' + std::to_string(d - 1) + 'x' + std::to_string(i) + 'x' + std::to_string(j));
				if (i < w - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(d - 1) + 'x' + std::to_string(i + 1) + 'x' + std::to_string(j)));
				}
				if (j < h - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(d - 1) + 'x' + std::to_string(i) + 'x' + std::to_string(j + 1)));
				}

				for (uint k = 0; k < balls.size(); k++) {


					DistanceConstraint* constraint = new DistanceConstraint(
						b1->Physics(),					//Physics Object A
						balls[k]->Physics(),					//Physics Object B
						b1->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						balls[k]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
		}
		//dwh
		for (uint i = 0; i < w - 1; i++) {
			for (uint j = 1; j < d - 1; j++) {
				float mass = 10.0f;
				if (i == 0)
					mass = 0.0f;
				this->AddGameObject(
					CommonUtils::BuildPointObject('s' + std::to_string(j) + 'x' + std::to_string(i) + 'x' + std::to_string(0),
						Vector3(x1 + j*step, 5.f, z1 - i*step),				//Position
						0.1f,									//Radius
						true,									//Has Physics Object
						mass,									//Infinite Mass
						true,									//No Collision Shape Yet
						true,									//Dragable by the user
						CommonUtils::GenColor(0.5f, 1.0f)));
			}
		}
		for (uint i = 0; i < w; i++) {
			for (uint j = 0; j < d; j++) {
				vector <GameObject *> balls;
				GameObject * b1 = this->FindGameObject('s' + std::to_string(j) + 'x' + std::to_string(i) + 'x' + std::to_string(0));
				if (i < w - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(j) + 'x' + std::to_string(i + 1) + 'x' + std::to_string(0)));
				}
				if (j < d - 1) {
					balls.push_back(this->FindGameObject('s' + std::to_string(j + 1) + 'x' + std::to_string(i) + 'x' + std::to_string(0)));
				}

				for (uint k = 0; k < balls.size(); k++) {


					DistanceConstraint* constraint = new DistanceConstraint(
						b1->Physics(),					//Physics Object A
						balls[k]->Physics(),					//Physics Object B
						b1->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						balls[k]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
		}
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"LPost",
			Vector3(0.0f, 3.0f, -0.05f),
			Vector3(0.05f, 2.0f, 0.05f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
		
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"BLPost",
			Vector3(0.0f, 1.05f, -1.5f),
			Vector3(0.05f, 0.05f, 1.5f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"RPost",
			Vector3(10.0f, 3.0f, -0.05f),
			Vector3(0.05f, 2.0f, 0.05f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));


		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"BRPost",
			Vector3(10.0f, 1.05f, -1.5f),
			Vector3(0.05f, 0.05f, 1.5f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"VPost",
			Vector3(5.0f, 5.0f, -0.05f),
			Vector3(5.0f, 0.05f, 0.05f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"BVPost",
			Vector3(5.0f, 1.05f, -3.0f),
			Vector3(5.0f, 0.05f, 0.05f),
			true,
			0.0f,
			true,
			false,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
		(*this->FindGameObject("LPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*this->FindGameObject("BLPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*this->FindGameObject("RPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*this->FindGameObject("BRPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*this->FindGameObject("VPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*this->FindGameObject("BVPost")->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		scoreboard=(CommonUtils::BuildCuboidObject(
			"Scoreboard",
			Vector3(5.0f, 10.00f, -3.0f),
			Vector3(5.0f, 2.05f, 0.05f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.0f, 0.0f, 0.0f, 1.0f)));

		(*scoreboard->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);

		this->AddGameObject(scoreboard);


		shot_ball = CommonUtils::BuildSphereObject("BALL",
			Vector3(5.0f, 4.05f, -3.0f),									//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f,									//Not Infinite Mass
			true,									//Has Collision Shape
			false,									//Not Dragable by the user
			CommonUtils::GenHSVColor(Vector3(0,0,1.0f),1.0f));	//Color;
		//Position, vel and acceleration all set in "ResetScene()"
		(*shot_ball->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(tex);
		this->AddGameObject(shot_ball);
		
		//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
		shot_ball->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
		{
			shot_ball->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
		});
		shot_ball->Physics()->SetElasticity(0.7f);
	}

	virtual void OnUpdateScene(float dt) override
	{
		Vector3 pos = shot_ball->Physics()->GetPosition();
		float rad = 0.5f;
		if (shot_ball->Physics()->GetLinearVelocity().z <= 0 && ball_active) {
			if ((pos.x-rad)>=0.05&&(pos.x+rad)<=9.95&&(pos.y-rad)>=0&&(pos.y+rad)<=4.95&&pos.z>=-rad && pos.z<=rad) {
				through_goal = true;
			}
			else if (through_goal && pos.z<-rad) {
				through_goal = false;
				ball_active = false;
				score += 100;
			}
			else {
				through_goal = false;
			}
		}
		if (!ball_active) {
			(*scoreboard->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(sb);
			(*scoreboard->Render()->GetChildIteratorStart())->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		}
		NCLDebug::DrawTextCs(Vector4(0.7f, 0.9f, 0.0f, 1.0f), 30, "Score: "+std::to_string(score), TEXTALIGN_CENTRE, Vector4(0.8f, 0.2f, 0.4f, 1.0f));
		Scene::OnUpdateScene(dt);
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J))	CalcForce();
		if (!Window::GetKeyboard()->KeyDown(KEYBOARD_J)&&j_ispressed)	ResetBall();
	}
	void ResetBall ()
	{
		if (ball_active) {
			score -= 50;
		}
		
		(*scoreboard->Render()->GetChildIteratorStart())->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		ball_active = true;
		through_goal = false;
		(*shot_ball->Render()->GetChildIteratorStart())->SetColor(Vector4 (1.0f, 1.0f, 1.0f, 1.0f));
		j_ispressed = false;
		Camera * camera = GraphicsPipeline::Instance()->GetCamera();
		float yaw = camera->GetYaw();
		float pitch = camera->GetPitch();


		shot_ball->Physics()->SetPosition(camera->GetPosition());
		//shot_ball->Physics()->SetLinearVelocity(Vector3(-12.0f * sin(DegToRad(yaw)), 9.0f* pitch, -12.0f * cos(DegToRad(yaw))));
		shot_ball->Physics()->SetLinearVelocity(Vector3(-15 *f* sin(DegToRad(yaw)), (1.0f + pitch *0.4f)*f, -15 * f* cos(DegToRad(yaw))));
		shot_ball->Physics()->SetForce(Vector3(0.0f, 0.0f, 0.0f));
		shot_ball->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, 0.f));
		f = 0.5f;
	}
	void CalcForce() {

		j_ispressed = true;
		if (f <= 4)
			f += 0.03f;
		(*scoreboard->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(white);
		(*scoreboard->Render()->GetChildIteratorStart())->SetColor(Vector4(f/4.0f, 0.0f, 0.0f, 1.0f));

	}
	protected:
		GLuint white;
		GLuint sb;
		GLuint tex;
		GLuint pitch_tex;
		Mesh * sphereMesh;
		GameObject*				shot_ball;
		GameObject*				scoreboard;
		float f = 0.5f;
		bool fup = true;
		bool j_ispressed = false;
		bool ball_active = true;
		bool through_goal;
		int score = 0;
};