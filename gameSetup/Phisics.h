#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <algorithm>
#include "opengl2Dlib.h"

#undef min
#undef max

namespace phisics
{

	
	struct BlockInfo
	{
		
		char type;
		
		bool isCollidable();
	};
	
	struct MapData
	{

		float BLOCK_SIZE = 64;
		
		BlockInfo* data;
		BlockInfo nullBlock = {};
	
		int w = 0;
		int h = 0;
	
		void create(int w, int h, const char* d);
		BlockInfo& get(int x, int y);
	
	
		void cleanup();
	
	
	};
	
	struct Entity
	{
		glm::vec2 pos;
		glm::vec2 lastPos;
	
		glm::vec2 dimensions;
	
		glm::vec2 velocity;

		float xMaxVelocity = 0;
		float xNormalVelocity = 0;
		float xSlowDownSpeed = 0;

		float yMaxVelocity = 0;
		float yNormalVelocity = 0;
		float ySlowDownSpeed = 0;

		void resolveConstrains(MapData& mapData);
	
	
		bool moving = 0;
	
	
		bool directionRight = 0;

	
		void move(glm::vec2 dir);
	
	
		//should be called only once per frame
		void updateMove();
	
		void applyGravity(float deltaTime);

		void applyVelocity(float deltaTime);

		void draw(gl2d::Renderer2D& renderer, float deltaTime, gl2d::Texture characterSprite);
	
	
	private:
		void checkCollisionBrute(glm::vec2& pos, glm::vec2 lastPos, MapData& mapData,
			bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch);
		glm::vec2 performCollision(MapData& mapData, glm::vec2 pos, glm::vec2 size, glm::vec2 delta,
			bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch);
	};
	
	
	//pos and size on on every component
	bool aabb(glm::vec4 b1, glm::vec4 b2);
	bool circleBox(glm::vec2 c, float r, glm::vec4 b);

};
