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

float posParticles[3] = { 0.0f,9.0f,0.0f };

float posParticlesA[3] = { 1.0f,9.0f,0.0f };
float posParticlesB[3] = { -1.0f,9.0f,0.0f };

float radiusSphere = 1.0f;
float radiusCapsule = 1.0f;

float forceX = 0.0f;
float forceY = -9.8f;
float forceZ = 0.0f;

//Normales de los planos del eje X
float normalXRight[3] = { 0.f };
float normalXLeft[3] = { 0.f };

//Normales de los planos del eje Y
float normalYDown[3] = { 0.f };
float normalYTop[3] = { 0.f };

//Normales de los planos del eje Z
float normalZFront[3] = { 0.f };
float normalZBack[3] = { 0.f };

float dDown, dTop, dRight, dLeft, dFront, dBack;

struct particle{

	float posX, posY, posZ;
	float prePosX, prePosY, prePosZ;
	float postPosX, postPosY, postPosZ;
	float velX, velY, velZ;
	float postVelX, postVelY, postVelZ;
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

void NormalPlane(float* pointA, float* pointB, float* pointC, float* normal) {

	float vectorA[3] = { pointA[0] - pointB[0], pointA[1] - pointB[1], pointA[2] - pointB[2] };
	float vectorB[3] = { pointC[0] - pointB[0], pointC[1] - pointB[1], pointC[2] - pointB[2] };
	
	normal[0] = vectorA[1] * vectorB[2] - vectorA[2] * vectorB[1];
	normal[1] = vectorA[2] * vectorB[0] - vectorA[0] * vectorB[2];
	normal[2] = vectorA[0] * vectorB[1] - vectorA[1] * vectorB[0];

	//Normalizar el vector
	float modulo = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= modulo;
	normal[1] /= modulo;
	normal[2] /= modulo;

	//return normal;
}

void PhysicsInit() {
	//TODO
	srand(time(NULL));

	//Calcular la normal de los planos
	//Plano bajo
	float pointA[3] = { -5.0f, 0.0f, -5.0f };
	float pointB[3] = { -5.0f, 0.0f, 5.0f };
	float pointC[3] = { 5.0f, 0.0f, 5.0f };
	NormalPlane(pointA, pointB, pointC, normalYDown);
	dDown = -(normalYDown[0] * pointA[0]) - (normalYDown[1] * pointA[1]) - (normalYDown[2] * pointA[2]);

	//Plano alto
	pointA[0] = 5.0f;
	pointA[1] = 10.0f;
	pointA[2] = 5.0f;

	pointB[0] = -5.0f;
	pointB[1] = 10.0f;
	pointB[2] = 5.0f;

	pointC[0] = -5.0f;
	pointC[1] = 10.0f;
	pointC[2] = -5.0f;
	NormalPlane(pointA, pointB, pointC, normalYTop);
	dTop = -(normalYTop[0] * pointA[0]) - (normalYTop[1] * pointA[1]) - (normalYTop[2] * pointA[2]);

	//Plano derecha
	pointA[0] = 5.0f;
	pointA[1] = 0.0f;
	pointA[2] = -5.0f;

	pointB[0] = 5.0f;
	pointB[1] = 0.0f;
	pointB[2] = 5.0f;

	pointC[0] = 5.0f;
	pointC[1] = 10.0f;
	pointC[2] = 5.0f;
	NormalPlane(pointA, pointB, pointC, normalXRight);
	dRight = -(normalXRight[0] * pointA[0]) - (normalXRight[1] * pointA[1]) - (normalXRight[2] * pointA[2]);

	//Plano izquierda
	pointA[0] = -5.0f;
	pointA[1] = 10.0f;
	pointA[2] = 5.0f;

	pointB[0] = -5.0f;
	pointB[1] = 0.0f;
	pointB[2] = 5.0f;

	pointC[0] = -5.0f;
	pointC[1] = 0.0f;
	pointC[2] = -5.0f;
	NormalPlane(pointA, pointB, pointC, normalXLeft);
	dLeft = -(normalXLeft[0] * pointA[0]) - (normalXLeft[1] * pointA[1]) - (normalXLeft[2] * pointA[2]);

	//Plano frontal
	pointA[0] = 5.0f;
	pointA[1] = 0.0f;
	pointA[2] = 5.0f;

	pointB[0] = -5.0f;
	pointB[1] = 0.0f;
	pointB[2] = 5.0f;

	pointC[0] = -5.0f;
	pointC[1] = 10.0f;
	pointC[2] = 5.0f;
	NormalPlane(pointA, pointB, pointC, normalZFront);
	dFront = -(normalZFront[0] * pointA[0]) - (normalZFront[1] * pointA[1]) - (normalZFront[2] * pointA[2]);

	//Plano trasero
	pointA[0] = -5.0f;
	pointA[1] = 10.0f;
	pointA[2] = -5.0f;

	pointB[0] = -5.0f;
	pointB[1] = 0.0f;
	pointB[2] = -5.0f;

	pointC[0] = 5.0f;
	pointC[1] = 0.0f;
	pointC[2] = -5.0f;
	NormalPlane(pointA, pointB, pointC, normalZBack);
	dBack = -(normalZBack[0] * pointA[0]) - (normalZBack[1] * pointA[1]) - (normalZBack[2] * pointA[2]);

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

		if (style == 0) {
			//EULER
			//Calcular posicion
			particles[i].postPosX = particles[i].posX + dt * particles[i].velX;
			particles[i].postPosY = particles[i].posY + dt * particles[i].velY;
			particles[i].postPosZ = particles[i].posZ + dt * particles[i].velZ;

			//Calcular velocidad
			particles[i].postVelX = particles[i].velX + dt * (forceX / particles[i].mass);
			particles[i].postVelY = particles[i].velY + dt * (forceY / particles[i].mass);
			particles[i].postVelZ = particles[i].velZ + dt * (forceZ / particles[i].mass);

		} else if (style == 1) {
			//VERLET
			//Calcular posicion
			if (particles[i].prePosX != particles[i].posX && particles[i].prePosY != particles[i].posY && particles[i].prePosZ != particles[i].posZ) {
				particles[i].postPosX = particles[i].posX + ((particles[i].posX - particles[i].prePosX) + (forceX / particles[i].mass) * pow(dt, 2));
				particles[i].postPosY = particles[i].posY + ((particles[i].posY - particles[i].prePosY) + (forceY / particles[i].mass) * pow(dt, 2));
				particles[i].postPosZ = particles[i].posZ + ((particles[i].posZ - particles[i].prePosZ) + (forceZ / particles[i].mass) * pow(dt, 2));
			}

		}

		float dotProductDown = (normalYDown[0] * particles[i].posX + normalYDown[1] * particles[i].posY + normalYDown[2] * particles[i].posZ);
		float dotProductPostDown = (normalYDown[0] * particles[i].postPosX + normalYDown[1] * particles[i].postPosY + normalYDown[2] * particles[i].postPosZ);

		//Detectar Colisiones
		//Plano bajo
		if ((dotProductDown + dDown) * (dotProductPostDown + dDown) <= 0) {

			if (style == 0) {

				float dotProductPostVelDown = (normalYDown[0] * particles[i].postVelX + normalYDown[1] * particles[i].postVelY + normalYDown[2] * particles[i].postVelZ);
				float dotProductVelDown = (normalYDown[0] * particles[i].velX + normalYDown[1] * particles[i].velY + normalYDown[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelDown * normalYDown[0], dotProductVelDown * normalYDown[1], dotProductVelDown * normalYDown[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostDown + dDown)*normalYDown[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostDown + dDown)*normalYDown[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostDown + dDown)*normalYDown[2];

				float dotProductPosDown = (normalYDown[0] * particles[i].posX + normalYDown[1] * particles[i].posY + normalYDown[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosDown + dDown)*normalYDown[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosDown = (normalYDown[0] * distPos[0] + normalYDown[1] * distPos[1] + normalYDown[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosDown * normalYDown[0], dotProductNormalPosDown * normalYDown[1], dotProductNormalPosDown * normalYDown[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		float dotProductTop = (normalYTop[0] * particles[i].posX + normalYTop[1] * particles[i].posY + normalYTop[2] * particles[i].posZ);
		float dotProductPostTop = (normalYTop[0] * particles[i].postPosX + normalYTop[1] * particles[i].postPosY + normalYTop[2] * particles[i].postPosZ);

		//Plano alto
		if ((dotProductTop + dTop) * (dotProductPostTop + dTop) <= 0) {

			if (style == 0) {

				float dotProductPostVelTop = (normalYTop[0] * particles[i].postVelX + normalYTop[1] * particles[i].postVelY + normalYTop[2] * particles[i].postVelZ);
				float dotProductVelTop = (normalYTop[0] * particles[i].velX + normalYTop[1] * particles[i].velY + normalYTop[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelTop * normalYTop[0], dotProductVelTop * normalYTop[1], dotProductVelTop * normalYTop[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostTop + dTop)*normalYTop[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostTop + dTop)*normalYTop[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostTop + dTop)*normalYTop[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelTop)*normalYTop[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelTop)*normalYTop[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelTop)*normalYTop[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostTop + dTop)*normalYTop[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostTop + dTop)*normalYTop[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostTop + dTop)*normalYTop[2];

				float dotProductPosTop = (normalYTop[0] * particles[i].posX + normalYTop[1] * particles[i].posY + normalYTop[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosTop + dTop)*normalYTop[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosTop = (normalYTop[0] * distPos[0] + normalYTop[1] * distPos[1] + normalYTop[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosTop * normalYTop[0], dotProductNormalPosTop * normalYTop[1], dotProductNormalPosTop * normalYTop[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		float dotProductRight = (normalXRight[0] * particles[i].posX + normalXRight[1] * particles[i].posY + normalXRight[2] * particles[i].posZ);
		float dotProductPostRight = (normalXRight[0] * particles[i].postPosX + normalXRight[1] * particles[i].postPosY + normalXRight[2] * particles[i].postPosZ);

		//Plano darecha
		if ((dotProductRight + dRight) * (dotProductPostRight + dRight) <= 0) {

			if (style == 0) {

				float dotProductPostVelRight = (normalXRight[0] * particles[i].postVelX + normalXRight[1] * particles[i].postVelY + normalXRight[2] * particles[i].postVelZ);
				float dotProductVelRight = (normalXRight[0] * particles[i].velX + normalXRight[1] * particles[i].velY + normalXRight[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelRight * normalXRight[0], dotProductVelRight * normalXRight[1], dotProductVelRight * normalXRight[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostRight + dRight)*normalXRight[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostRight + dRight)*normalXRight[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostRight + dRight)*normalXRight[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelRight)*normalXRight[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelRight)*normalXRight[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelRight)*normalXRight[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostRight + dRight)*normalXRight[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostRight + dRight)*normalXRight[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostRight + dRight)*normalXRight[2];

				float dotProductPosRight = (normalYTop[0] * particles[i].posX + normalYTop[1] * particles[i].posY + normalYTop[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosRight + dRight)*normalXRight[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosRight = (normalXRight[0] * distPos[0] + normalXRight[1] * distPos[1] + normalXRight[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosRight * normalXRight[0], dotProductNormalPosRight * normalXRight[1], dotProductNormalPosRight * normalXRight[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		float dotProductLeft = (normalXLeft[0] * particles[i].posX + normalXLeft[1] * particles[i].posY + normalXLeft[2] * particles[i].posZ);
		float dotProductPostLeft = (normalXLeft[0] * particles[i].postPosX + normalXLeft[1] * particles[i].postPosY + normalXLeft[2] * particles[i].postPosZ);

		//Plano izquierda
		if ((dotProductLeft + dLeft) * (dotProductPostLeft + dLeft) <= 0) {

			if (style == 0) {

				float dotProductPostVelLeft = (normalXLeft[0] * particles[i].postVelX + normalXLeft[1] * particles[i].postVelY + normalXLeft[2] * particles[i].postVelZ);
				float dotProductVelLeft = (normalXLeft[0] * particles[i].velX + normalXLeft[1] * particles[i].velY + normalXLeft[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelLeft * normalXLeft[0], dotProductVelLeft * normalXLeft[1], dotProductVelLeft * normalXLeft[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostLeft + dLeft)*normalXLeft[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostLeft + dLeft)*normalXLeft[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostLeft + dLeft)*normalXLeft[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelLeft)*normalXLeft[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelLeft)*normalXLeft[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelLeft)*normalXLeft[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostLeft + dLeft)*normalXLeft[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostLeft + dLeft)*normalXLeft[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostLeft + dLeft)*normalXLeft[2];

				float dotProductPosLeft = (normalXLeft[0] * particles[i].posX + normalXLeft[1] * particles[i].posY + normalXLeft[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosLeft + dLeft)*normalXLeft[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosLeft = (normalXLeft[0] * distPos[0] + normalXLeft[1] * distPos[1] + normalXLeft[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosLeft * normalXLeft[0], dotProductNormalPosLeft * normalXLeft[1], dotProductNormalPosLeft * normalXLeft[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		float dotProductFront = (normalZFront[0] * particles[i].posX + normalZFront[1] * particles[i].posY + normalZFront[2] * particles[i].posZ);
		float dotProductPostFront = (normalZFront[0] * particles[i].postPosX + normalZFront[1] * particles[i].postPosY + normalZFront[2] * particles[i].postPosZ);

		//Plano frontal
		if ((dotProductFront + dFront) * (dotProductPostFront + dFront) <= 0) {

			if (style == 0) {

				float dotProductPostVelFront = (normalZFront[0] * particles[i].postVelX + normalZFront[1] * particles[i].postVelY + normalZFront[2] * particles[i].postVelZ);
				float dotProductVelFront = (normalZFront[0] * particles[i].velX + normalZFront[1] * particles[i].velY + normalZFront[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelFront * normalZFront[0], dotProductVelFront * normalZFront[1], dotProductVelFront * normalZFront[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostFront + dFront)*normalZFront[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostFront + dFront)*normalZFront[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostFront + dFront)*normalZFront[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelFront)*normalZFront[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelFront)*normalZFront[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelFront)*normalZFront[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostFront + dFront)*normalZFront[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostFront + dFront)*normalZFront[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostFront + dFront)*normalZFront[2];

				float dotProductPosFront = (normalZFront[0] * particles[i].posX + normalZFront[1] * particles[i].posY + normalZFront[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosFront + dFront)*normalZFront[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosFront = (normalZFront[0] * distPos[0] + normalZFront[1] * distPos[1] + normalZFront[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosFront * normalZFront[0], dotProductNormalPosFront * normalZFront[1], dotProductNormalPosFront * normalZFront[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		float dotProductBack = (normalZBack[0] * particles[i].posX + normalZBack[1] * particles[i].posY + normalZBack[2] * particles[i].posZ);
		float dotProductPostBack = (normalZBack[0] * particles[i].postPosX + normalZBack[1] * particles[i].postPosY + normalZBack[2] * particles[i].postPosZ);

		//Plano trasero
		if ((dotProductBack + dBack) * (dotProductPostBack + dBack) <= 0) {

			if (style == 0) {

				float dotProductPostVelBack = (normalZBack[0] * particles[i].postVelX + normalZBack[1] * particles[i].postVelY + normalZBack[2] * particles[i].postVelZ);
				float dotProductVelBack = (normalZBack[0] * particles[i].velX + normalZBack[1] * particles[i].velY + normalZBack[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelBack * normalZBack[0], dotProductVelBack * normalZBack[1], dotProductVelBack * normalZBack[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostBack + dBack)*normalZBack[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostBack + dBack)*normalZBack[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostBack + dBack)*normalZBack[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelBack)*normalZBack[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelBack)*normalZBack[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelBack)*normalZBack[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostBack + dBack)*normalZBack[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostBack + dBack)*normalZBack[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostBack + dBack)*normalZBack[2];

				float dotProductPosBack = (normalZBack[0] * particles[i].posX + normalZBack[1] * particles[i].posY + normalZBack[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosBack + dBack)*normalZBack[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosBack = (normalZBack[0] * distPos[0] + normalZBack[1] * distPos[1] + normalZBack[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosBack * normalZBack[0], dotProductNormalPosBack * normalZBack[1], dotProductNormalPosBack * normalZBack[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}
		}

		//Colisiones Esfera
		float dist = sqrt(pow(particles[i].postPosX - posA[0], 2) + pow(particles[i].postPosY - posA[1], 2) + pow(particles[i].postPosZ - posA[2], 2));
		if (dist <= radiusSphere) {

			//Calcular punto de colision
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			float P[3] = { particles[i].posX, particles[i].posY, particles[i].posZ };
			float Q[3] = { particles[i].postPosX, particles[i].postPosY, particles[i].postPosZ };
			float V[3] = { Q[0] - P[0], Q[1] - P[1], Q[2] - P[2] };

			float a = (pow(V[0], 2) + pow(V[1], 2) + pow(V[2], 2));
			float b = (2 * P[0] * V[0] - 2 * V[0] * posA[0] + 2 * P[1] * V[1] - 2 * V[1] * posA[1] + 2 * P[2] * V[2] - 2 * V[2] * posA[2]);
			float c = (pow(P[0], 2) - 2*P[0]*posA[0]+ pow(posA[0], 2) + pow(P[1], 2) - 2 * P[1] * posA[1] + pow(posA[1], 2) + pow(P[2], 2) - 2 * P[2] * posA[2] + pow(posA[2], 2)) - pow(radiusSphere, 2);

			float alpha[2] = { (-b + sqrt(pow(b,2) - 4 * a*c)) / (2 * a),
								(-b - sqrt(pow(b,2) - 4 * a*c)) / (2 * a),
			};

			float colision1[3] = { P[0] + V[0] * alpha[0], P[1] + V[1] * alpha[0] , P[2] + V[2] * alpha[0] };
			float colision2[3] = { P[0] + V[0] * alpha[1], P[1] + V[1] * alpha[1] , P[2] + V[2] * alpha[1] };

			float distCol1 = sqrt(pow(P[0] - colision1[0], 2) + pow(P[1] - colision1[1], 2) + pow(P[2] - colision1[2], 2));
			float distCol2 = sqrt(pow(P[0] - colision2[0], 2) + pow(P[1] - colision2[1], 2) + pow(P[2] - colision2[2], 2));

			float puntoColision[3] = { colision1[0], colision1[1], colision1[2] };

			if (distCol1 > distCol2) {
				puntoColision[0] = colision2[0];
				puntoColision[1] = colision2[1];
				puntoColision[2] = colision2[2];
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Calcular colision con el plano tangencial a la esfera con el punto de colision
			float normalColision[3] = { puntoColision[0] - posA[0], puntoColision[1] - posA[1], puntoColision[2] - posA[2] };
			/*float normalColMod = sqrt(pow(normalColision[0], 2) + pow(normalColision[1], 2) + pow(normalColision[2], 2));
			normalColision[0] /= normalColMod;
			normalColision[1] /= normalColMod;
			normalColision[2] /= normalColMod;*/
			//float dColision = -(normalColision[0] * puntoColision[0]) - (normalColision[1] * puntoColision[1]) - (normalColision[2] * puntoColision[2]);
			float dColision = -(normalColision[0] * puntoColision[0] + normalColision[1] * puntoColision[1] + normalColision[2] * puntoColision[2]);

			float dotProductColision = (normalColision[0] * particles[i].posX + normalColision[1] * particles[i].posY + normalColision[2] * particles[i].posZ);
			float dotProductPostColision = (normalColision[0] * particles[i].postPosX + normalColision[1] * particles[i].postPosY + normalColision[2] * particles[i].postPosZ);
			
			if (style == 0) {

				float dotProductPostVelColision = (normalColision[0] * particles[i].postVelX + normalColision[1] * particles[i].postVelY + normalColision[2] * particles[i].postVelZ);
				float dotProductVelColision = (normalColision[0] * particles[i].velX + normalColision[1] * particles[i].velY + normalColision[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelColision * normalColision[0], dotProductVelColision * normalColision[1], dotProductVelColision * normalColision[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelColision)*normalColision[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelColision)*normalColision[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelColision)*normalColision[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			} else if (style == 1) {

				particles[i].postPosX = particles[i].postPosX - 2 * (dotProductPostColision + dBack)*normalColision[0];
				particles[i].postPosY = particles[i].postPosY - 2 * (dotProductPostColision + dBack)*normalColision[1];
				particles[i].postPosZ = particles[i].postPosZ - 2 * (dotProductPostColision + dBack)*normalColision[2];

				float dotProductPosColision = (normalColision[0] * particles[i].posX + normalColision[1] * particles[i].posY + normalColision[2] * particles[i].posZ);
				particles[i].posY = particles[i].posY - 2 * (dotProductPosColision + dColision)*normalColision[1];

				float distPos[3] = { particles[i].postPosX - particles[i].posX , particles[i].postPosY - particles[i].posY , particles[i].postPosZ - particles[i].posZ }; //Es la distancia entre la posicion actual y la posicion siguiente (v)

				float dotProductNormalPosColision = (normalColision[0] * distPos[0] + normalColision[1] * distPos[1] + normalColision[2] * distPos[2]);
				float normalPos[3] = { dotProductNormalPosColision * normalColision[0], dotProductNormalPosColision * normalColision[1], dotProductNormalPosColision * normalColision[2] };
				float tangPos[3] = { distPos[0] - normalPos[0], distPos[1] - normalPos[1], distPos[2] - normalPos[2] };

				particles[i].posX = particles[i].posX + (1 - elasticity)*normalPos[0] + friction*tangPos[0];
				particles[i].posY = particles[i].posY + (1 - elasticity)*normalPos[1] + friction*tangPos[1];
				particles[i].posZ = particles[i].posZ + (1 - elasticity)*normalPos[2] + friction*tangPos[2];

			}

		}

		particles[i].prePosX = particles[i].posX;
		particles[i].prePosY = particles[i].posY;
		particles[i].prePosZ = particles[i].posZ;

		particles[i].posX = particles[i].postPosX;
		particles[i].posY = particles[i].postPosY;
		particles[i].posZ = particles[i].postPosZ;

		particles[i].velX = particles[i].postVelX;
		particles[i].velY = particles[i].postVelY;
		particles[i].velZ = particles[i].postVelZ;

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