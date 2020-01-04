

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

//#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>
#include <iostream>

#include "SysOpenGLInit.h"
#include "SysOpenGLSetting.h"
#include "Model.h"
#include "CameraFreeLook.h"
#include "Input.h"
#include "Shader.h"
#include "Light.h"
#include "STModel.h"
#include "GUIMainPanel.h"
//#include "sqlite3/sqlite3.h"

SDL_Window* window;
SDL_GLContext context;
unsigned int SCREEN_WIDTH = 1280;
unsigned int SCREEN_HEIGHT = 720;
bool quit = false;

int main(int argc, char* args[]) {
	/*-----------
	OPENGL / SDL
	-----------*/
	if (unsigned int e = SystemOpenGLInit::defaults(window, context, SCREEN_WIDTH, SCREEN_HEIGHT, 3, 3) > 0) return e;
	SysOpenGLSetting::defaults(window, SCREEN_WIDTH, SCREEN_HEIGHT);


	/*---
	IMGUI
	----*/
	bool err = glewInit() != GLEW_OK;
	if (err) {
		std::cout << "Error loading GLew for Imgui" << std::endl;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330");


	/*--------------
	INITIALIZATIONS
	--------------*/
	Model box = Model();
	Input input = Input();
	CameraFreeLook camera = CameraFreeLook(SCREEN_WIDTH, SCREEN_HEIGHT);
	Light light = Light();
	Shader modelShader = Shader("shaders/basic_lighting_no_texture.vert", "shaders/basic_lighting_no_texture.frag");
	Shader lightShader = Shader("shaders/light.vert", "shaders/light.frag");

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.getViewMatrix();

	/*-----------
	LOAD MODELS
	-----------*/
	STModel stBox;
	stBox.position = glm::vec3(0, 0, 0);
	//box.loadModel("assets/hospitalroom/hp194.obj");
	box.loadModel("assets/test_map/test_map.obj");

	light.load(glm::vec3(0, 16, 0));

	double dt = 1.0f / 60.0; 
	bool wireframeToggle = false;

	while (!quit) {
		/*-----------
		UPDATE
		-----------*/
		input.update(dt);
		if (input.isLShift()) {
			SysOpenGLSetting::mouseCursor(false);
			SDL_WarpMouseInWindow(window, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f);

			if (input.isA()) camera.strafeLeft(dt);
			if (input.isD()) camera.strafeRight(dt);
			if (input.isW()) camera.forward(dt);
			if (input.isS()) camera.backward(dt);

			if (input.isMouseMotion()) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				camera.mousePositionUpdate(dt, x, y);
			}
		}
		else {
			SysOpenGLSetting::mouseCursor(true);
		}
		if (input.isQuit()) quit = true;
		if (input.isO()) SysOpenGLSetting::wireframe(true);
		if (input.isP()) SysOpenGLSetting::wireframe(false);


		view = camera.getViewMatrix();

		/*-----------
		RENDER MODELS
		-----------*/
		box.draw(projection, view, modelShader, stBox, light.position, camera.getCameraPosition());
		light.draw(projection, view, lightShader);

		/*-----------
		GUI RENDER
		-----------*/
		GUI::begin(window);
		GUI::mainPanel(camera);
		light.gui();
		GUI::render();
		 
		/*-----------
		SWAP BUFFER
		-----------*/
		SDL_GL_SwapWindow(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
	}

	ImGui_ImplSDL2_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	return SystemOpenGLInit::shutDown(window, context);
};