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
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6 
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#define TILE_SIZE 0.25f
#include <math.h>
#define GRAVITY -9.8

using namespace std;

//float mapHeight = -1;
//float mapWidth = -1;
//unsigned char **mapData = nullptr;




float accumulator = 0.0f;

SDL_Window* displayWindow;


float time = 0.0f;
float speed = 0.75f;



enum EntityType { ENTITY_PLAYER, ENTITY_GROUND, ENTITY_COIN };

struct SheetSprite {

	SheetSprite() {}
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}

	void Draw(ShaderProgram &program) {
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


class Entity {
public:
	Entity() {}
	Entity(EntityType type, float xPos, float yPos) : type(type), x(xPos), y(yPos) {}
	SheetSprite sprite;
	float length = TILE_SIZE;
	float height = TILE_SIZE;
	float x;
	float y;
	glm::vec3 position = glm::vec3(x, y, 1.0f);
	glm::vec3 size = glm::vec3(length, height, 1.0f);
	float velocity_x = 0;
	float velocity_y = 0;
	bool isStatic = true;
	EntityType type;
	void Update(ShaderProgram &program, float elapsed) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x * elapsed * velocity_x, position.y * elapsed * velocity_y * GRAVITY, 1.0f));
		program.SetModelMatrix(modelMatrix);
	}
	void Render(ShaderProgram &program) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
		program.SetModelMatrix(modelMatrix);

	}
	bool CollidesWith(Entity &entity) {
		float yPenetration = fabs((position.y - entity.position.y) - height / 2 - entity.height / 2);
		float xPenetration = fabs((position.x - entity.position.x) - length / 2 - entity.length / 2);
		if (xPenetration != 0 || yPenetration != 0) {
			return true;

		} return false;
	}

};

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



void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX, int spriteCountY, int texture) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	float texCoords[] = {
	u, v + spriteHeight,
	u + spriteWidth, v,
	u, v,
	u + spriteWidth, v,
	u, v + spriteHeight,
	u + spriteWidth, v + spriteHeight
	};
	float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE,
	-0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE };
	// draw this data
	glBindTexture(GL_TEXTURE_2D, texture);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

class FlareMap {
public:
	float mapWidth = -1;
	float mapHeight = -1;
	unsigned char **mapData = nullptr;



	void loadMap(ifstream& infile) {
		
		string line;
		while (getline(infile, line)) {

			if (line == "[header]") {
				if (!readHeader(infile)) {
					return;
				}
			}
			else if (line == "[layer]") {
				readLayerData(infile);
			}
			else if (line == "[Objectslayer]") {
				readEntityData(infile);
			}
		}
	}


	bool readHeader(std::ifstream &stream) {
		string line;
		mapWidth = -1;
		mapHeight = -1;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "width") {
				mapWidth = atoi(value.c_str());
			}
			else if (key == "height") {
				mapHeight = atoi(value.c_str());
			}
		}
		if (mapWidth == -1 || mapHeight == -1) {
			return false;
		}
		else { // allocate our map data
			mapData = new unsigned char*[mapHeight];
			for (int i = 0; i < mapHeight; ++i) {
				mapData[i] = new unsigned char[mapWidth];
			}
			return true;
		}
	}

	bool readLayerData(std::ifstream &stream) {
		string line;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "data") {
				for (int y = 0; y < mapHeight; y++) {
					getline(stream, line);
					istringstream lineStream(line);
					string tile;
					for (int x = 0; x < mapWidth; x++) {
						getline(lineStream, tile, ',');
						unsigned char val = (unsigned char)atoi(tile.c_str());
						if (val > 0) {
							// be careful, the tiles in this format are indexed from 1 not 0
							mapData[y][x] = val - 1;
						}
						else {
							mapData[y][x] = 0;
						}
					}
				}
			}
		}
		return true;
	}

	GLuint drawTileMap(ShaderProgram &program)
	{
		GLuint mapTexture = LoadTexture("arne_sprites.png");
		std::vector<float> vertexData;
		std::vector<float> texCoordData;
		for (int y = 0; y < 22; y++) {
			for (int x = 0; x < 16; x++) {
				if (mapData[y][x] != 0) {
					// add vertices

					float u = (float)(((int)mapData[y][x]) % 22) / (float)16;
					float v = (float)(((int)mapData[y][x]) / 16) / (float)22;
					float spriteWidth = 1.0f / (float)16;
					float spriteHeight = 1.0f / (float)22;

					vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
						});
					texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
						});
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, mapTexture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		return mapTexture;
	}

	bool readEntityData(std::ifstream &stream) {
		string line;
		string type;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "type") {
				type = value;
			}
			else if (key == "location") {
				istringstream lineStream(value);
				string xPosition, yPosition;
				getline(lineStream, xPosition, ',');
				getline(lineStream, yPosition, ',');
				float placeX = atoi(xPosition.c_str())* TILE_SIZE;
				float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
				placeEntity(type, placeX, placeY);
			}
		}
		return true;
	}

	void placeEntity(string type, float x, float y) {
		int counter = 0;
		if (type == "ENTITY_PLAYER") {
			Entity player(ENTITY_PLAYER, x, y);
			counter++;
			if (counter > 1) {
				Entity player(ENTITY_PLAYER, x + 5, y);
			}
		}
		else if (type == "ENTITY_COIN") {
			Entity coin(ENTITY_COIN, x, y);

		}
		else {
			Entity ground(ENTITY_GROUND, x, y);

		}


	}
	
	//vector<FlareMapEntity> entities;
};


vector<Entity> entityVec;

struct GameState {
	Entity player;
	vector<Entity>Ground;
	vector<Entity>Coins;
};
GameState state;

GLuint fontTexture;

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
	float character_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
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
	//glBindTexture(GL_TEXTURE_2D, fontTexture);


	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

int bulletIndex = 0;



void jump(ShaderProgram program, float elapsed, Entity &player) {

	state.player.velocity_y = 10;
	state.player.Update(program, elapsed);

}





enum GameMode { TITLESCREEN, GAMELEVEL, GAMEOVER };
GameMode mode = TITLESCREEN;

void RenderTitleScreen(ShaderProgram& program) {
	glm::mat4 modelMatrix_1 = glm::mat4(1.0f);
	glm::mat4 modelMatrix_2 = glm::mat4(1.0f);
	modelMatrix_1 = glm::translate(modelMatrix_1, glm::vec3(-1.0f, 0.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_1);
	DrawText(program, fontTexture, "Welcome", 0.13f, 0.0f);
	modelMatrix_2 = glm::translate(modelMatrix_1, glm::vec3(0.0f, -0.5f, 0.0f));
	modelMatrix_2 = glm::scale(modelMatrix_2, glm::vec3(0.75f, 1.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_2);
	DrawText(program, fontTexture, "Jump!", 0.13f, 0.0f);

}

void RenderGameLevel(ShaderProgram& program, GameState &state) {

	state.player.Render(program);

	state.player.Render(program);


	for (Entity &Ground : state.Ground)
	{
		Ground.Render(program);
	}

	for (Entity& Coins : state.Coins)
	{
		Coins.Render(program);
	}
}

void RenderGameOver(ShaderProgram& program) {
	glm::mat4 modelMatrix_1 = glm::mat4(1.0f);
	glm::mat4 modelMatrix_2 = glm::mat4(1.0f);
	modelMatrix_1 = glm::translate(modelMatrix_1, glm::vec3(-1.0f, 0.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_1);
	DrawText(program, fontTexture, "Sorry You Lost", 0.13f, 0.0f);
	modelMatrix_2 = glm::translate(modelMatrix_1, glm::vec3(0.0f, -0.5f, 0.0f));
	modelMatrix_2 = glm::scale(modelMatrix_2, glm::vec3(0.75f, 1.0f, 0.0f));
	program.SetModelMatrix(modelMatrix_2);
	DrawText(program, fontTexture, "Better Luck Next Time!", 0.13f, 0.0f);

}

void UpdateGame(GameState& state, float elapsed) {

	


		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (state.player.position.x < -1.77) {
			state.player.position.x = -1.77;
		}

		if (state.player.position.x > 1.77) {
			state.player.position.x = 1.77;
		}

		if (keys[SDL_SCANCODE_D])
		{
			state.player.position.x += (state.player.velocity_x * elapsed);
		}

		if (keys[SDL_SCANCODE_A])
		{
			state.player.position.x += -(state.player.velocity_x * elapsed);
		}
		/*
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
	*/


	for (int i = 0; i < state.Coins.size(); i++) {
		//for (int j = 0; j < MAX_BULLETS; j++) {

			float horizontal_collision = (abs(state.Coins[i].position.x - state.player.position.x)) - ((state.player.size.x + state.Coins[i].size.x) / 2);

			float vertical_collision = (abs(state.Coins[i].position.y - state.player.position.y)) - ((state.player.size.y + state.Coins[i].size.y) / 2);

			if (horizontal_collision < 0 && vertical_collision < 0) {

				state.Coins[i].position.y = -1000.0f;

				//state.Coins[j].position.y = -1000.0f;
			}
		//}
	}
}

void Update(float elapsed) {
	switch (mode) {
	case TITLESCREEN:
		break;
	case GAMELEVEL:
		UpdateGame(state, elapsed);
		break;
	case GAMEOVER:
		break;
	}
}

void Render(ShaderProgram& program) {
	switch (mode) {
	case TITLESCREEN:
		RenderTitleScreen(program);
		break;
	case GAMELEVEL:
		RenderGameLevel(program, state);
		break;
	}
}












//Main()

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

	//GLuint space_background = LoadTexture(RESOURCE_FOLDER"space.jpeg");
	ifstream infile ("JFlareMap.txt");

	FlareMap map;
	map.loadMap(infile);
	map.readHeader(infile);
	map.readLayerData(infile);
	map.readEntityData(infile);
	map.drawTileMap(program);
	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//GLuint spriteSheetTexture = LoadTexture("sheet.png");
	//fontTexture = LoadTexture("font1.png");
	SheetSprite player = SheetSprite(map.drawTileMap(program), 247.0f / 1024.0f, 84.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.2f);

	Entity Object(ENTITY_PLAYER, 0.0f, -0.8f);
	Object.sprite = player;
	Object.size.x = player.size * (player.width / player.height);
	Object.size.y = player.size;

	state.player = Object;

	//SheetSprite Coin = SheetSprite(map.drawTileMap(program), 224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);

	float x = -1.0;

	float y = 0.5f;

	int row_size = 0;

	for (int i = 0; i < 35; ++i) {
		Entity Object(ENTITY_COIN, 1.0f, 1.0f);
		//Object.sprite = Coin;
		//Object.size.x = Coin.size * (Coin.width / Coin.height);
		//Object.size.y = Coin.size;
		state.Coins.push_back(Object);
		x += Object.size.x;
		row_size++;

		if (row_size == 7) {
			x = -1.0f;
			//y -= Coin.size;
			row_size = 0;
		}
	}
	//state.Coins = Coins;
	//SheetSprite Coins = SheetSprite(spriteSheetTexture, 327.0f / 1024.0f, 644.0f / 1024.0f, 40.0f / 1024.0f, 20.0f / 1024.0f, 0.015f);
	/*
	for (int i = 0; i < MAX_BULLETS; ++i) {
		state.bullets[i].position.x = -1000.0f;
		state.bullets[i].velocity.y = 0.75f;
		state.bullets[i].sprite = Bullet;
	}*/

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
		if (mode == GAMELEVEL && time >= 60)
		{
			mode = GAMEOVER;
		}

		

		//while(!done){
		//	//get elapsed time
		//	elapsed += accumulator;
		//	if(elapsed < FIXED_TIMESTEP){
		//		 accumulator = elapsed;
		//		 continue;
  //           }

			while(elapsed >= FIXED_TIMESTEP){
	             Update(FIXED_TIMESTEP);
	             elapsed -= FIXED_TIMESTEP;
	        }
	        accumulator = elapsed;
	        Render(program);
	        



		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_Q) //Gives option to quit game
			{
				done = true;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_S) {

				jump(program,elapsed, state.player);
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		//glClearColor(0.9f, 0.2f, 0.5f, 0.0f);

		//glBindTexture(GL_TEXTURE_2D, space_background);
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

		//glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-state.player.position.x, -state.player.position.y, 0.0f));
		program.SetViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;

}


