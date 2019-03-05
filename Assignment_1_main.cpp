
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Birthday Banner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	//set-up code
	SDL_Event event;
	bool done = false;

	glViewport(0, 0, 640, 360);
	
	float lastFrameTicks = 0.0f;
	float angle = 45.0f * (3.1415926f / 180.0f);

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint donut = LoadTexture(RESOURCE_FOLDER"donut.png");
	GLuint ice_cream = LoadTexture(RESOURCE_FOLDER"ice_cream.png");
	GLuint cake = LoadTexture(RESOURCE_FOLDER"Cake.png");
	GLuint birthday = LoadTexture(RESOURCE_FOLDER"birthday.jpg");

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		//Write in-loop code here

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.9f, 0.2f, 0.5f, 0.0f);
		
		
		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		float vertices_T1[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		float texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		angle += elapsed;



		//Background
		glBindTexture(GL_TEXTURE_2D, birthday);
		glm::mat4 modelMatrix_4 = glm::mat4(1.0f);
		modelMatrix_4 = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 0.0f));
		program.SetModelMatrix(modelMatrix_4);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);


		//Donut
		glBindTexture(GL_TEXTURE_2D, donut);
		glm::mat4 modelMatrix_0 = glm::mat4(1.0f);
		modelMatrix_0 = glm::translate(modelMatrix, glm::vec3(1.0f, -0.5f, 0.0f));
		modelMatrix_0 = glm::rotate(modelMatrix_0, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		program.SetModelMatrix(modelMatrix_0);
		
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		
		//Ice Cream
		glBindTexture(GL_TEXTURE_2D, ice_cream);
		glm::mat4 modelMatrix_1 = glm::mat4(1.0f);
		modelMatrix_1 = glm::translate(modelMatrix, glm::vec3(-1.2f, -0.5f, 0.0f));
		modelMatrix_1 = glm::rotate(modelMatrix_1, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		program.SetModelMatrix(modelMatrix_1);

		
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);;

		//Cake
		glBindTexture(GL_TEXTURE_2D, cake);
		glm::mat4 modelMatrix_2 = glm::mat4(1.0f);
		modelMatrix_2 = glm::translate(modelMatrix, glm::vec3(0.0f, 0.5f, 0.0f));
		modelMatrix_2 = glm::rotate(modelMatrix_2,angle, glm::vec3(0.0f, 0.0f, 1.0f));
		program.SetModelMatrix(modelMatrix_2);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}
	
	SDL_Quit();
	return 0;
}


