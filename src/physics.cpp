#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <stdlib.h>

int type = 0;
int style = 0;
int elasticity = 0;
int friction = 0;
int object = 0;
float posA[3] = { 0.0f,1.0f,0.0f };
float posB[3] = { -3.0f,2.0f,-2.0f };
float posC[3] = { -4.0f,2.0f,2.0f };

float posParticles[3] = { 0.0f,10.0f,0.0f };

float radiusSphere = 1.0f;
float radiusCapsule = 1.0f;

struct particle{

	float posX, posY, posZ;
	float velX, velY, velZ;

};

particle *particles = new particle[500];

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
		ImGui::DragInt("Elasticidad", &elasticity, 1, 0, 100);

		//static int friction = 0;
		ImGui::DragInt("Friccion", &friction, 1, 0, 100);

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
	for (int i = 0; i < 500; i++) {
		particles[i].posX = posParticles[0];
		particles[i].posY = posParticles[1];
		particles[i].posZ = posParticles[2];

		particles[i].velX = 0.0f;
		particles[i].velY = 0.0f;
		particles[i].velZ = 0.0f;
	}
}
void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < 500; ++i) {
		particles[i].posX = posParticles[0];
		particles[i].posY = posParticles[1];
		particles[i].posZ = posParticles[2];
		/*particles[i].posY -= dt*5.0f;
		if (particles[i].posY < 0.0f) particles[i].posY = 10.0f;*/
	}
}
void PhysicsCleanup() {
	//TODO
}