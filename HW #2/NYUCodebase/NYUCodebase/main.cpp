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

float PI = glm::pi<float>();

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("PONG!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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
	float PaddleA_Y = 0.0f;
	float PaddleA_X = -1.5f;
	float PaddleB_Y = 0.0f;
	float PaddleB_X = 1.5f;
	float BallX = 0.0f;
	float BallY = 0.0f;
	float initial_angle = 15.0f;
	float angle = initial_angle;
	float Dist_Per_Sec = 1.0f;
	float BallVelocity = 1.0f;
	float DirectionX = cosf(angle * PI / 180.0f);
	float DirectionY = sinf(angle * PI / 180.0f); 
	float make_angle_inverse = 270.0f;
	ShaderProgram program;
	
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");


	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	float paddleWidth = 1.0f;
	float paddleHeight = 1.0f;

	float ballWidth = 1.0f;
	float ballHeight = 1.0f;

	while (!done) {

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
			//Write in-loop code here
			glClear(GL_COLOR_BUFFER_BIT);
			float ticks = SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			float vertices_T1[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			
			BallX += DirectionX * elapsed * BallVelocity;
			BallY += DirectionY * elapsed * BallVelocity;
			
			
			glm::mat4 modelMatrix_0 = glm::mat4(1.0f);
			modelMatrix_0 = glm::translate(modelMatrix, glm::vec3(-1.5f, PaddleA_Y, 0.0f));
			float paddle_YscaleFactor = 0.45f;
			float paddle_XscaleFactor = 0.1f;
			modelMatrix_0 = glm::scale(modelMatrix_0, glm::vec3(paddle_XscaleFactor, paddle_YscaleFactor, 0.0f));
			paddleWidth = paddle_XscaleFactor;
			paddleHeight = paddle_YscaleFactor;
			program.SetModelMatrix(modelMatrix_0);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);

			glm::mat4 modelMatrix_1 = glm::mat4(1.0f);
			modelMatrix_1 = glm::translate(modelMatrix, glm::vec3(1.5f, PaddleB_Y, 0.0f));
			modelMatrix_1 = glm::scale(modelMatrix_1, glm::vec3(paddle_XscaleFactor, paddle_YscaleFactor, 0.0f));
			program.SetModelMatrix(modelMatrix_1);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);


			glm::mat4 modelMatrix_2 = glm::mat4(1.0f);
			float ball_YScaleFactor = 0.08f;
			float ball_XScaleFactor = 0.08f;
			ballWidth = ball_XScaleFactor;
			ballHeight = ball_YScaleFactor;
			modelMatrix_2 = glm::translate(modelMatrix_2, glm::vec3(BallX, BallY, 0.0f));
			modelMatrix_2 = glm::scale(modelMatrix_2, glm::vec3(ballWidth, ballHeight, 0.0f));
			program.SetModelMatrix(modelMatrix_2);
			
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_T1);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);


			if (keys[SDL_SCANCODE_A] && PaddleA_Y + paddleHeight / 2.0f <= 1.0f) {
				PaddleA_Y += elapsed * Dist_Per_Sec;
			}
			else if (keys[SDL_SCANCODE_Z] && PaddleA_Y - paddleHeight / 2.0f >= -1.0f) {
				PaddleA_Y += -1.0f * elapsed * Dist_Per_Sec;

			}
			else if (keys[SDL_SCANCODE_K] && PaddleB_Y + paddleHeight / 2.0f <= 1.0f) {
				PaddleB_Y += elapsed * Dist_Per_Sec;

			}
			else if (keys[SDL_SCANCODE_M] && PaddleB_Y - paddleHeight / 2.0f >= -1.0f) {
				PaddleB_Y += -1.0f * elapsed * Dist_Per_Sec;
			}

			//Collisions

			float PA_horizontal_diff = fabs(PaddleA_X - BallX) - ((paddleWidth + ballWidth) / 2.0f);
			float PA_vertical_diff = fabs(PaddleA_Y - BallY) - ((paddleHeight + ballHeight) / 2.0f);
			if (PA_horizontal_diff < 0 && PA_vertical_diff < 0) {
				angle += make_angle_inverse;
				DirectionX = -1.0f * cosf(angle * PI / 180.0f);
				DirectionY = -1.0f * sinf(angle * PI / 180.0f);
			}
			float PB_horizontal_diff = fabs(PaddleB_X - BallX) - ((paddleWidth + ballWidth) / 2.0f);
			float PB_vertical_diff = fabs(PaddleB_Y - BallY) - ((paddleHeight + ballHeight) / 2.0f);
			if (PB_horizontal_diff < 0 && PB_vertical_diff < 0) {
				angle += make_angle_inverse;
				DirectionX = -1.0f * cosf(angle * PI / 180.0f);
				DirectionY = -1.0f * sinf(angle * PI / 180.0f);
			}

			if ((BallY + (ballHeight / 2.0f)) >= 1.0f || (BallY - ballHeight / 2.0f) <= -1.0f) {
				angle += make_angle_inverse;
				DirectionX = -1.0f * cosf(angle * PI / 180.0f);
				DirectionY = -1.0f * sinf(angle * PI / 180.0f);
			}

			if (BallX + (ballWidth / 2.0f) >= 1.777f) {
				std::cout << "Left Paddle Wins" << std::endl;
				BallX = 0;
				BallY = 0;
			}
			if (BallX - (ballWidth / 2.0f) <= -1.777f) {
				std::cout << "Right Paddle Wins" << std::endl;
				BallX = 0;
				BallY = 0;
			}

			SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;

}