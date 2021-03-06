

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>

#include "SysOpenGLInit.h"
#include "SysOpenGLSetting.h"
#include "Model.h"
#include "CameraFreeLook.h"
#include "Input.h"
#include "Shader.h"
#include "Light.h"
#include "STModel.h"
#include "GUI.h"
#include "STLight.h"
#include "STMaterial.h"
#include "Grid.h"
#include "MeshManager.h"
#include "Render.h"
#include "Physics.h"
#include "Mesh.h"

SDL_Window* window;
SDL_GLContext context;
unsigned int SCREEN_WIDTH = 1280;
unsigned int SCREEN_HEIGHT = 720;
bool quit = false;
bool showGrid = true;
bool simPhysics = false;
bool showDebugDraw = false;

STLight lightProps;
STLight lightProps2;
STMaterial materialProps;

int main(int argc, char* args[]) {
	/*-----------
	OPENGL / SDL
	-----------*/
	if (unsigned int e = SystemOpenGLInit::defaults(window, context, SCREEN_WIDTH, SCREEN_HEIGHT, 3, 3) > 0) return e;
	SysOpenGLSetting::defaults(window, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*--------------
	INITIALIZATIONS
	--------------*/
	GUI::init(window, context);

	Input input = Input();
	CameraFreeLook camera = CameraFreeLook(SCREEN_WIDTH, SCREEN_HEIGHT);
	Light light = Light();
	Grid grid = Grid();
	Shader modelShader = Shader("shaders/basic_lighting_no_texture.vert", "shaders/basic_lighting_no_texture.frag");
	Shader lightShader = Shader("shaders/light.vert", "shaders/light.frag");
	MeshManager* meshManager = new MeshManager();
	Physics physics = Physics();

	meshManager->loadModel("assets/map1/player.dae", 0);
	meshManager->loadModel("assets/map1/map1.dae", 1);
	
	
	std::vector<Mesh> meshes = meshManager->getMeshes();

	for (int i = 0; i < meshes.size(); i++) {

		if (meshes[i].meshType == 0) {
			physics.addDynamicBox(meshes[i], i);
		}

		if (meshes[i].meshType == 1) {
			physics.addStaticBox(meshes[i], i);
		}
	}

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.getViewMatrix();

	/*-----------
	LOAD MODELS
	-----------*/
	 
	light.load(glm::vec3(0, 16, 0));

	/*------
	TIMESTEP
	-------*/
	double t = 0.0;
	double dt = 0.01;
	double currentTime = SDL_GetTicks();
	double accumulator = 0.0;

	/*----------- 
	GAME LOOP
	-----------*/
	while (!quit) {
		double newTime = SDL_GetTicks();
		double frameTime = newTime - currentTime;

		if (frameTime > 0.25) {
			frameTime = 0.25;
		}

		currentTime = newTime;
		accumulator += frameTime;

		while (accumulator >= dt) {
			// Perform physics processes
			
			accumulator -= dt;
		}

		if (simPhysics) physics.simulate(dt);
		physics.getUpdatedPositions(meshManager->meshes);

		/*----
		UPDATE
		-----*/
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
		if (input.isP()) simPhysics = true;
		if (input.isQuit()) quit = true;

		view = camera.getViewMatrix();

		/*-----------
		RENDER MODELS
		-----------*/

		if (showGrid) grid.render(projection, view, camera.getCameraPosition());
		if (showDebugDraw) physics.drawDebugData(projection, view);

		modelShader.use();
		modelShader.setFloat("shininess", materialProps.shininess);
		modelShader.setVec3("material_specular", materialProps.specular);
		modelShader.setVec3("light_specular", lightProps.specular);
		modelShader.setVec3("lightPos", light.position);
		modelShader.setVec3("viewPos", camera.getCameraPosition());

		light.draw(projection, view, lightShader);
		 
		Render::mesh(meshManager->getMeshes(), projection, view, modelShader);
		

		/*-----------
		GUI RENDER
		-----------*/

		GUI::begin(window);
		GUI::mainPanel(camera, light, lightProps, materialProps, showGrid, showDebugDraw);
		GUI::render();
		 
		/*-----------
		SWAP BUFFER
		-----------*/
		SDL_GL_SwapWindow(window);

		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
	}

	delete meshManager;
	GUI::shutdown();
	return SystemOpenGLInit::shutDown(window, context);
};