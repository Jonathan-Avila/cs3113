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
#include <vector>
#define MAX_BULLETS 100


float time = 0.0f;
float speed = 0.75f;


SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath){
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL){
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

struct SheetSprite{

	SheetSprite() {}
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}

	void Draw(ShaderProgram &program){
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
		};

		float aspect = width / height;
		float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size ,
		0.5f * size * aspect, -0.5f * size };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}
	    float size;
		unsigned int textureID;
		float u;
		float v;
		float width;
		float height;
};

struct Entity{
	Entity() {}
	Entity(float x, float y, float h, float yBullets = 0.0f)
	{
		position.x = x;
		position.y = y;
		velocity.x = h;
		velocity.y = yBullets;
	}
	void Draw(ShaderProgram &program){
		glm::mat4 modelMatrix_0 = glm::mat4(1.0f);
		modelMatrix_0 = glm::translate(modelMatrix_0, glm::vec3(position.x, position.y, 0.0f));
		program.SetModelMatrix(modelMatrix_0); 
		sprite.Draw(program);
	}

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 size;

	SheetSprite sprite;

	float timeAlive = 0.0f;
};

std::vector<Entity> enemies; 

struct GameState{
	Entity player;
	std::vector<Entity>enemies;
	Entity bullets[MAX_BULLETS];
};
GameState state;

GLuint fontTexture;

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing){
	float character_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++){
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
		((size + spacing) * i) + (-0.5f * size), 0.5f * size,
		((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		((size + spacing) * i) + (0.5f * size), 0.5f * size,
		((size + spacing) * i) + (0.5f * size), -0.5f * size,
		((size + spacing) * i) + (0.5f * size), 0.5f * size,
		((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
		texCoordData.insert(texCoordData.end(), {
		texture_x, texture_y,
		texture_x, texture_y + character_size,
		texture_x + character_size, texture_y,
		texture_x + character_size, texture_y + character_size,
		texture_x + character_size, texture_y,
		texture_x, texture_y + character_size,
			});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
   

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

int bulletIndex = 0;

void shootBullet(Entity &object) {

	state.bullets[bulletIndex].position.x = object.position.x;
	state.bullets[bulletIndex].position.y = object.position.y;
	bulletIndex++;

	if (bulletIndex > MAX_BULLETS - 1)
	{
		bulletIndex = 0;
	}
}

enum GameMode { TITLESCREEN, GAMELEVEL };
GameMode mode = TITLESCREEN;

void RenderMainMenu(ShaderProgram& program) {
	glm::mat4 modelMatrix_1 = glm::mat4(1.0f);
	glm::mat4 modelMatrix_2 = glm::mat4(1.0f);
	modelMatrix_1 = glm::translate(modelMatrix_1, glm::vec3(-1.0f, 0.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_1);
	DrawText(program, fontTexture, "Space Invaders!", 0.13f, 0.0f);
	modelMatrix_2 = glm::translate(modelMatrix_1, glm::vec3(0.0f, -0.5f, 0.0f));
	modelMatrix_2 = glm::scale(modelMatrix_2, glm::vec3(0.75f, 1.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_2);
	DrawText(program, fontTexture, "Destroy the Space Rocks!", 0.13f, 0.0f);
	
}

void RenderGameLevel(ShaderProgram& program, GameState &state) {
	state.player.Draw(program);

	for (Entity &enemy : state.enemies)
	{
		enemy.Draw(program);
	}

	for (Entity &bullet : state.bullets)
	{
		bullet.Draw(program);
	}
}

void UpdateGame(GameState& state, float elapsed) {

	bool shouldRemovebullet(); {


	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (state.player.position.x < -1.77) {
		state.player.position.x = -1.77;
	}

	if (state.player.position.x > 1.77) {
		state.player.position.x = 1.77;
	}

	if (keys[SDL_SCANCODE_D])
	{
		state.player.position.x += (state.player.velocity.x * elapsed);
	}

	if (keys[SDL_SCANCODE_A])
	{
		state.player.position.x += -(state.player.velocity.x * elapsed);
	}

	for (Entity &bullet : state.bullets)
	{

		bullet.position.y += elapsed * bullet.velocity.y;
		//std::cout << bullet.timeAlive << std::endl;
		if (bullet.timeAlive > 5) {
			bullet.position.y = -1000;
			bullet.timeAlive = 0;
		}
		else {
			bullet.timeAlive += elapsed;
		}	
		}
	}


	for (int i = 0; i < state.enemies.size(); i++){
		for (int j = 0; j < MAX_BULLETS; j++){

			float horizontal_collision = (abs(state.enemies[i].position.x - state.bullets[j].position.x)) - ((state.bullets[j].size.x + state.enemies[i].size.x)/2);

			float vertical_collision = (abs(state.enemies[i].position.y - state.bullets[j].position.y)) - ((state.bullets[j].size.y + state.enemies[i].size.y)/2);
			
			if (horizontal_collision < 0 && vertical_collision < 0){

				state.enemies[i].position.y = -1000.0f;

				state.bullets[j].position.y = -1000.0f;
			}
		}
	}
}

void Update(float elapsed) {
	switch (mode) {
	case TITLESCREEN:
		break;
	case GAMELEVEL:
		UpdateGame(state, elapsed);
		break;
	}
}

void Render(ShaderProgram& program){
	switch (mode) {
	case TITLESCREEN:
		RenderMainMenu(program);
		break;
	case GAMELEVEL:
		RenderGameLevel(program, state);
		break;
	}
}


int main(int argc, char *argv[])
{
	float lastFrameTicks = 0.0f;
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 1200, 600);
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint space_background = LoadTexture(RESOURCE_FOLDER"space.jpeg");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	fontTexture = LoadTexture("font1.png");
	SheetSprite player = SheetSprite(spriteSheetTexture, 247.0f / 1024.0f, 84.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.2f);

	Entity Object(0.0f, -0.9f, 1.0f);
	Object.sprite = player;
	Object.size.x = player.size * (player.width / player.height);
	Object.size.y = player.size;

	state.player = Object;

	SheetSprite rockSprite = SheetSprite(spriteSheetTexture, 224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);

	float x = -1.0;

	float y = 0.5f;

	int row_size = 0;

	for (int i = 0; i < 35; ++i){
		Entity Object(x, y, 1.0f);
		Object.sprite = rockSprite;
		Object.size.x = rockSprite.size * (rockSprite.width/rockSprite.height);
		Object.size.y = rockSprite.size;
		enemies.push_back(Object);
		x += Object.size.x;
		row_size++;

		if (row_size == 7){
			x = -1.0f;
			y -= rockSprite.size;
			row_size  = 0;
		}
	}
	state.enemies = enemies;
	SheetSprite Bullet = SheetSprite(spriteSheetTexture, 327.0f / 1024.0f, 644.0f / 1024.0f, 40.0f / 1024.0f, 20.0f / 1024.0f, 0.015f);
	
	for (int i = 0; i < MAX_BULLETS; ++i){
		state.bullets[i].position.x = -1000.0f;
		state.bullets[i].velocity.y = 0.75f;
		state.bullets[i].sprite = Bullet;
	}

	SDL_Event event;
	bool done = false;
	while (!done)
	{
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		time += elapsed; 
		lastFrameTicks = ticks;

		if (mode == TITLESCREEN && time >= 3)
		{
			mode = GAMELEVEL;
		}

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
			{
				done = true;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S){
				
				shootBullet(state.player);
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.9f, 0.2f, 0.5f, 0.0f);

		glBindTexture(GL_TEXTURE_2D, space_background);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);

		projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

		glUseProgram(program.programID);

		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix); 

		glm::mat4 modelMatrix_3 = glm::mat4(1.0f);
		modelMatrix_3 = glm::scale(modelMatrix_3, glm::vec3(2.0f, 2.0f, 0.0f));
		program.SetModelMatrix(modelMatrix_3);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		float texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		Render(program);

		Update(elapsed);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;

}
