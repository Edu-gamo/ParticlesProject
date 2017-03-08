#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

int type = 0;
int style = 0;
float elasticity = 1.0f;
float friction = 0.0f;
int object = 0;
float posA[3] = { 0.0f,1.0f,0.0f };
float posB[3] = { -3.0f,2.0f,-2.0f };
float posC[3] = { -4.0f,2.0f,2.0f };

float posParticles[3] = { 0.0f,10.0f,0.0f };

float radiusSphere = 1.0f;
float radiusCapsule = 1.0f;

float forceX = 0.0f;
float forceY = -9.8f;
float forceZ = 0.0f;

struct particle{

	float posX, posY, posZ;
	float velX, velY, velZ;
	float mass;

};

particle *particles = new particle[500];

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}

bool show_test_window = false;
void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	{	//Propiedades
		ImGui::Begin("Propiedades");
		
		ImGui::Text("Seleccionar tipo:");
		//static int type = 0;
		ImGui::RadioButton("Cascada", &type, 0); ImGui::SameLine();
		ImGui::RadioButton("Fuente", &type, 1);

		ImGui::DragFloat3("Particles Pos.", posParticles, 0.1f);

		ImGui::Text("\nSeleccionar forma de calculo:");
		//static int style = 0;
		ImGui::RadioButton("Euler", &style, 0); ImGui::SameLine();
		ImGui::RadioButton("Verlet", &style, 1);

		ImGui::Text("\nPropiedades:");
		//static int elasticity = 0;
		ImGui::DragFloat("Elasticidad", &elasticity, 0.01f, 0, 1);

		//static int friction = 0;
		ImGui::DragFloat("Friccion", &friction, 0.01f, 0, 1);

		ImGui::Text("\nSeleccionar objeto a modificar:");
		//static int object = 0;
		ImGui::RadioButton("Esfera", &object, 0); ImGui::SameLine();
		ImGui::RadioButton("Capsula", &object, 1);

		ImGui::Text("\nPropiedades:");
		//static int posA[3] = { 0,0,0 }, posB[3] = { 1,1,1 }, radius = 10;
		if (object == 0) {
			ImGui::DragFloat3("Position", posA, 0.1f);
			ImGui::DragFloat("Radius", &radiusSphere, 0.1f, 0.1f, 10.0f);
		} else {
			ImGui::DragFloat3("Position A", posB, 0.1f);
			ImGui::DragFloat3("Position B", posC, 0.1f);
			ImGui::DragFloat("Radius", &radiusCapsule, 0.1f, 0.1f, 10.0f);
		}

		ImGui::End();
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void PhysicsInit() {
	//TODO
	srand(time(NULL));
	for (int i = 0; i < 500; i++) {
		particles[i].posX = posParticles[0];
		particles[i].posY = posParticles[1];
		particles[i].posZ = posParticles[2];

		particles[i].velX = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velY = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velZ = ((float)rand() / RAND_MAX) * 2 - 1;

		std::cout << particles[i].velY << std::endl;

		particles[i].mass = 0.1f;

	}
}
void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < 500; ++i) {
		//Calcular posicion
		particles[i].posX = particles[i].posX + dt * particles[i].velX;
		particles[i].posY = particles[i].posY + dt * particles[i].velY;
		particles[i].posZ = particles[i].posZ + dt * particles[i].velZ;

		//Calcular velocidad
		/*particles[i].velX = particles[i].mass * forceX + particles[i].velX;
		particles[i].velY = particles[i].mass * forceY + particles[i].velY;
		particles[i].velZ = particles[i].mass * forceZ + particles[i].velZ;*/
		particles[i].velX = particles[i].velX + forceX * dt;
		particles[i].velY = particles[i].velY + forceY * dt;
		particles[i].velZ = particles[i].velZ + forceX * dt;

		//Detectar Colision
		/*if ((9.8f * particles[i].posY + dt) <= 0 || (9.8f * (particles[i].posY + dt * particles[i].velY) + dt) <= 0) {
			particles[i].posX = (particles[i].posX + dt * particles[i].velX) - (1 + elasticity) * (-forceX*(particles[i].posX + dt * particles[i].velX) + dt)*-forceX;
			particles[i].velX = (particles[i].mass * forceX + particles[i].velX) - (1 + elasticity) * (-forceX*(particles[i].mass * forceX + particles[i].velX))*-forceX;

			particles[i].posY = (particles[i].posY + dt * particles[i].velY) - (1 + elasticity) * (-forceY*(particles[i].posY + dt * particles[i].velY) + dt)*-forceY;
			particles[i].velY = (particles[i].mass * forceY + particles[i].velY) - (1 + elasticity) * (-forceY*(particles[i].mass * forceY + particles[i].velY))*-forceY;

			particles[i].posZ = (particles[i].posZ + dt * particles[i].velZ) - (1 + elasticity) * (-forceZ*(particles[i].posZ + dt * particles[i].velZ) + dt)*-forceZ;
			particles[i].velZ = (particles[i].mass * forceZ + particles[i].velZ) - (1 + elasticity) * (-forceZ*(particles[i].mass * forceZ + particles[i].velZ))*-forceZ;
		}*/

	}

	float *partVerts = new float[500 * 3];
	for (int i = 0; i < 500; ++i) {
		partVerts[i * 3 + 0] = particles[i].posX;
		partVerts[i * 3 + 1] = particles[i].posY;
		partVerts[i * 3 + 2] = particles[i].posZ;
	}
	LilSpheres::updateParticles(0, 500, partVerts);
	delete[] partVerts;

}
void PhysicsCleanup() {
	//TODO
}