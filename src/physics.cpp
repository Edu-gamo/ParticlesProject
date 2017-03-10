#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

int type = 1;
int style = 0;
float elasticity = 0.5f;
float friction = 0.75f;
int object = 0;
float posA[3] = { 0.0f,1.0f,0.0f };
float posB[3] = { -3.0f,2.0f,-2.0f };
float posC[3] = { -4.0f,2.0f,2.0f };

float posParticles[3] = { 0.0f,10.0f,0.0f };

float posParticlesA[3] = { 1.0f,10.0f,0.0f };
float posParticlesB[3] = { -1.0f,10.0f,0.0f };

float radiusSphere = 1.0f;
float radiusCapsule = 1.0f;

float forceX = 0.0f;
float forceY = -9.8f;
float forceZ = 0.0f;

struct particle{

	float posX, posY, posZ;
	float prePosX, prePosY, prePosZ;
	float velX, velY, velZ;
	float mass;
	float life;
	float tmpX, tmpY, tmpZ;

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
		ImGui::RadioButton("Fuente", &type, 1); ImGui::SameLine();
		ImGui::RadioButton("Cascada", &type, 0); 
		if (type == 0) {
			ImGui::DragFloat3("Particles Pos.A", posParticlesA, 0.1f);
			ImGui::DragFloat3("Particles Pos.B", posParticlesB, 0.1f);
		}
		else {
			ImGui::DragFloat3("Particles Pos.", posParticles, 0.1f);
		}

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

		particles[i].prePosX = posParticles[0];
		particles[i].prePosY = posParticles[1];
		particles[i].prePosZ = posParticles[2];

		particles[i].velX = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velY = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velZ = ((float)rand() / RAND_MAX) * 2 - 1;

		particles[i].mass = 1.0f;

		particles[i].life = ((float)rand() / RAND_MAX) * 5 - 1;

	}
}

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < 500; ++i) {

		if (particles[i].life <= 0.0f) {

			if (type == 0) {
				particles[i].posX = RandomFloat(posParticlesA[0], posParticlesB[0]);
				particles[i].posY = RandomFloat(posParticlesA[1], posParticlesB[1]);
				particles[i].posZ = RandomFloat(posParticlesA[2], posParticlesB[2]);
			}
			else {
				particles[i].posX = posParticles[0];
				particles[i].posY = posParticles[1];
				particles[i].posZ = posParticles[2];
			}

			particles[i].prePosX = RandomFloat(particles[i].posX - 0.05f, particles[i].posX + 0.05f);
			particles[i].prePosY = particles[i].posY + 0.05f;
			particles[i].prePosZ = RandomFloat(particles[i].posZ - 0.05f, particles[i].posZ + 0.05f);

			particles[i].velX = ((float)rand() / RAND_MAX) * 2 - 1;
			particles[i].velY = ((float)rand() / RAND_MAX) * 2 - 1;
			particles[i].velZ = ((float)rand() / RAND_MAX) * 2 - 1;

			particles[i].life = ((float)rand() / RAND_MAX) * 5 - 1;

		}

		particles[i].life -= dt * 0.5f;

		particles[i].tmpX = particles[i].posX;
		particles[i].tmpY = particles[i].posY;
		particles[i].tmpZ = particles[i].posZ;

		if (style == 0) {
			//EULER
			//Calcular posicion
			particles[i].posX = particles[i].posX + dt * particles[i].velX;
			particles[i].posY = particles[i].posY + dt * particles[i].velY;
			particles[i].posZ = particles[i].posZ + dt * particles[i].velZ;

			//Calcular velocidad
			particles[i].velX = particles[i].velX + dt * (forceX / particles[i].mass);
			particles[i].velY = particles[i].velY + dt * (forceY / particles[i].mass);
			particles[i].velZ = particles[i].velZ + dt * (forceZ / particles[i].mass);

		} else if (style == 1) {
			//VERLET
			//Calcular posicion
			particles[i].posX = particles[i].posX + (particles[i].posX - particles[i].prePosX) - (fabs(forceX)/particles[i].mass) * pow(dt, 2);
			particles[i].posY = particles[i].posY + (particles[i].posY - particles[i].prePosY) - (fabs(forceY)/particles[i].mass) * pow(dt, 2);
			particles[i].posZ = particles[i].posZ + (particles[i].posZ - particles[i].prePosZ) - (fabs(forceZ)/particles[i].mass) * pow(dt, 2);

		}

		particles[i].prePosX = particles[i].tmpX;
		particles[i].prePosY = particles[i].tmpY;
		particles[i].prePosZ = particles[i].tmpZ;

		//Detectar Colisiones
		//if ((particles[i].posY + dt) <= 0 || ((particles[i].posY + dt * particles[i].velY) + dt) <= 0) {
		if (particles[i].posY <= 0) {
			if (style == 0) {
				particles[i].velX = particles[i].velX * friction;
				particles[i].velY = -particles[i].velY * elasticity;
				particles[i].velZ = particles[i].velZ * friction;
			} else if (style == 1) {
				//particles[i].posX = particles[i].posX * friction;
				particles[i].posY = -particles[i].posY * elasticity;
				//particles[i].posZ = particles[i].posZ * friction;

			}
		}
		if (((particles[i].posX + dt) <= -5 || (particles[i].posX + dt) >= 5) || (((particles[i].posX + dt * particles[i].velX) + dt) <= -5 || ((particles[i].posX + dt * particles[i].velX) + dt) >= 5)) {
			particles[i].velX = -particles[i].velX * elasticity;
			particles[i].velY = particles[i].velY * friction;
			particles[i].velZ = particles[i].velZ * friction;
		}
		if (((particles[i].posZ + dt) <= -5 || (particles[i].posZ + dt) >= 5) || (((particles[i].posZ + dt * particles[i].velZ) + dt) <= -5 || ((particles[i].posZ + dt * particles[i].velZ) + dt) >= 5)) {
			particles[i].velX = particles[i].velX * friction;
			particles[i].velY = particles[i].velY * friction;
			particles[i].velZ = -particles[i].velZ * elasticity;
		}

		//Colisiones Esfera
		float distX = fabs(particles[i].posX - posA[0]);
		float distY = fabs(particles[i].posY - posA[1]);
		float distZ = fabs(particles[i].posZ - posA[2]);
		if (distX <= radiusSphere && distY <= radiusSphere && distZ <= radiusSphere) {
			particles[i].velX = particles[i].velX * friction;
			particles[i].velY = -particles[i].velY * elasticity;
			particles[i].velZ = particles[i].velZ * friction;
		}

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