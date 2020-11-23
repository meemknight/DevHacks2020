#include <Windows.h>
#include "gameStructs.h"
#include <GL/glew.h>
#include <sstream>

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD     fdwReason,
	LPVOID    lpvReserved
)
{

	if(fdwReason == DLL_PROCESS_ATTACH)
	{
	//	OutputDebugString("gameLayer: dll attached");
	}

	return true;
}

FreeListAllocator* allocator = nullptr;
Console* console = nullptr;

#pragma region allocator

void* operator new  (std::size_t count)
{
	auto a = allocator->threadSafeAllocate(count);


	return a;
}

void* operator new[](std::size_t count)
{
	auto a = allocator->threadSafeAllocate(count);


	return a;
}

void operator delete  (void* ptr)
{

	allocator->threadSafeFree(ptr);
}

void operator delete[](void* ptr)
{

	allocator->threadSafeFree(ptr);
}

#pragma endregion

float GAMEZOOM = 110;

static float gameSizeX = 16 * GAMEZOOM; // 1760
static float gameSizeY = 9 * GAMEZOOM; // 990

static float cardSizePanelSide = 1.2 * GAMEZOOM;
static float cardSizePanelSidePadding = 0.1 * GAMEZOOM;


static float sidePannelSize = 2 * GAMEZOOM;

static float cardAspectRatio = 1.371;

glm::vec2 cardSize = { cardSizePanelSide  , cardSizePanelSide* cardAspectRatio };

int goldRushType;
int isGoldRush;

int getScor(int suite)
{
	HAND_TYPE t = (HAND_TYPE)suite;
	int scor = 0;

	switch (t)
	{
	case NONE:
		break;
	case HIGH_CARD:
		scor = 10;
		break;
	case ONE_PAIR:
		scor = 50;
		break;
	case TWO_PAIR:
		scor = 100;
		break;
	case THREE_OF_A_KIND:
		scor = 300;
		break;
	case STRAIGHT:
		scor = 600;
		break;
	case FLUSH:
		scor = 1000;
		break;
	case FULLHOUSE:
		scor = 1500;
		break;
	case FOUR_OF_A_KIND:
		scor = 2000;
		break;
	case STRAIGHT_FLUSH:
		scor = 3000;
		break;
	case ROYAL_FLUSH:
		scor = 5000;
		break;
	case HAND_TYPE_COUNT:
		break;
	default:
		break;
	}

	if(goldRushType == triple_points && isGoldRush)
	{
		scor *= 3;
	}

	return scor;
}

std::string minute_sec(int sec)
{
	std::string str;
	if (sec <= 0)
	{
		str = "00:00";
	}
	else
	{
		int minute = sec / 60;
		int restSec = sec % 60;
		std::string strMin, strSec;
		strMin = std::to_string(minute);
		strSec = std::to_string(restSec);
		if (minute < 10)
		{
			strMin = "0" + std::to_string(minute);
		}
		if (restSec < 10)
		{
			strSec = "0" + std::to_string(restSec);
		}

		if (minute == 0)
		{
			str = "00:" + strSec;
		}
		else if (restSec == 0)
		{
			str = strMin + ":00";
		}
		else
		{
			str = strMin + ":" + strSec;
		}
	}
	return str;
}

float getTilt()
{
	
	return 30 * sin(clock() / 100.f);

}

const char* goldRushName[] =
{
	"Only high cards will appear during gold rush.",
	"More cards will appear during gold rush.",
	"Only one suite will appear during gold rush.",
	"Triple points during gold rush.",
};

const float GOLD_RUSH_TIME = 60;

extern "C" __declspec(dllexport) void onCreate(GameMemory* mem, HeapMemory * heapMemory,
	WindowSettings *windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	new(mem) GameMemory; // *mem = GameMemory();
	console = &platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}

	gl2d::setErrorFuncCallback([](const char* c) {console->elog(c); });
	gl2d::init();
#pragma endregion


	//set the size of the window
	windowSettings->w = 640;
	windowSettings->h = 360;
	windowSettings->drawWithOpenGl = true;
	windowSettings->lockTo60fps = false;
	gl2d::setVsync(1);


	mem->renderer.create();

	mem->background.loadFromFile("resources//background.png");
	mem->cards.loadFromFile("resources//cards.png");
	mem->backCard.loadFromFile("resources//back.png");
	mem->goldBack.loadFromFile("resources//goldBack.png");
	mem->cardsAtlas.xCount = 14;
	mem->cardsAtlas.yCount = 4;

	mem->playerTexture.loadFromFile("resources//player.png");
	mem->playerAtlas.xCount = 2;
	mem->playerAtlas.yCount = 1;

	mem->ps.initParticleSystem(100);
	mem->ps.pixelateFactor = 1;
	mem->ps.postProcessing = false;


	mem->player.pos = { sidePannelSize , sidePannelSize };
	mem->player.dimensions = { GAMEZOOM * 0.8, GAMEZOOM * 0.8 };

	mem->font.createFromFile("resources//font.ttf");

	mem->wallTexture.loadFromFile("resources//wall.png");
	mem->wallAtlas.xCount = 2;
	mem->wallAtlas.yCount = 1;

	mem->diamond.loadFromFile("resources//part//diamonds.png");
	mem->club.loadFromFile("resources//part//clubs.png");
	mem->spade.loadFromFile("resources//part//spades.png");
	mem->heart.loadFromFile("resources//part//hearts.png");
	mem->clock.loadFromFile("resources//clock.png");
	mem->coin.loadFromFile("resources//part//coins.png");
	mem->diamondYellow.loadFromFile("resources//part//diamond.png");
	mem->dolar.loadFromFile("resources//part//dollar.png");
	mem->clover.loadFromFile("resources//part//FLClover.png");
	mem->star.loadFromFile("resources//part//Star.png");

	mem->shatter[0].loadFromFile("resources//shatter1.png");
	mem->shatter[1].loadFromFile("resources//shatter2.png");
	mem->shatter[2].loadFromFile("resources//shatter3.png");
	mem->shatter[3].loadFromFile("resources//shatter4.png");

	mem->rulette.loadFromFile("resources//Ruleta.png");

#pragma region particle

	mem->basicParticle[0].onCreateCount = 1;
	mem->basicParticle[0].subemitParticleTime = {};
	mem->basicParticle[0].particleLifeTime = { 1.9, 3.5 };
	mem->basicParticle[0].directionX = { -280,280 };
	mem->basicParticle[0].directionY = { 280,-280 };
	mem->basicParticle[0].createApearence.size = { 90, 90 };
	mem->basicParticle[0].dragX = { -5,5 };
	mem->basicParticle[0].dragY = { 250,300 };
	mem->basicParticle[0].rotation = { 0, 360 };
	mem->basicParticle[0].rotationSpeed = { 45, 60 };
	mem->basicParticle[0].rotationDrag = { 0, 10 };
	mem->basicParticle[0].createApearence.color1 = { 1,1,1,1};
	mem->basicParticle[0].createApearence.color2 = { 1,1,1,1};
	mem->basicParticle[0].createEndApearence.color1 = { 1,1,1, 0.5};
	mem->basicParticle[0].createEndApearence.size = { 100,100 };
	mem->basicParticle[0].tranzitionType = gl2d::TRANZITION_TYPES::linear;
	mem->basicParticle[0].texturePtr = &mem->heart;
	mem->basicParticle[0].deathRattle = 0;
	mem->basicParticle[0].subemitParticle = nullptr;
	mem->basicParticle[0].positionX = { -20,20 };
	mem->basicParticle[0].positionY = { -20,20 };


	for(int i=1; i<sizeof(mem->basicParticle) / sizeof(mem->basicParticle[0]); i++)
	{
		mem->basicParticle[i] = mem->basicParticle[0];
	}

	for(int i=0; i<4; i++)
	{
		mem->shatterPart[i] = mem->basicParticle[0];

		mem->shatterPart[i].directionX = {-30,30};
		mem->shatterPart[i].directionY = {-30,30};

		mem->shatterPart[i].createApearence.size = { 180,190 };
		mem->shatterPart[i].createEndApearence.size = { 200,200 };
		mem->shatterPart[i].createEndApearence.color1 = { 1,1,1,0.8};


		mem->shatterPart[i].texturePtr = &mem->shatter[i];
	}

	mem->basicParticle[0].texturePtr = &mem->heart;
	mem->basicParticle[1].texturePtr = &mem->spade;
	mem->basicParticle[2].texturePtr = &mem->diamond;
	mem->basicParticle[3].texturePtr = &mem->club;
	mem->basicParticle[4].texturePtr = &mem->coin;
	mem->basicParticle[5].texturePtr = &mem->diamondYellow;
	mem->basicParticle[6].texturePtr = &mem->dolar;
	mem->basicParticle[7].texturePtr = &mem->clover;
	mem->basicParticle[8].texturePtr = &mem->star;


	mem->coinEmiter = mem->basicParticle[4];
	mem->manyCoins = mem->basicParticle[4];

	mem->manyCoins.onCreateCount = 6;

	mem->coinEmiter.subemitParticleTime = { 0.20, 0.15 };
	mem->coinEmiter.subemitParticle = &mem->manyCoins;
	mem->coinEmiter.createApearence.color1 = {};
	mem->coinEmiter.createApearence.color2 = {};
	mem->coinEmiter.createEndApearence.color1 = {};
	mem->coinEmiter.particleLifeTime = { 0.2,0.7 };

	mem->downCoin = mem->basicParticle[4];
	mem->downCoin.dragX = {};
	mem->downCoin.directionX = {};

	mem->coinRain.onCreateCount = 1;
	mem->coinRain.subemitParticleTime = {0.1,0.01};
	mem->coinRain.particleLifeTime = { GOLD_RUSH_TIME-5, GOLD_RUSH_TIME-5 };
	mem->coinRain.createApearence.size = { 1, 1 };
	mem->coinRain.createApearence.color1 = { 0,0,0,0 };
	mem->coinRain.createApearence.color2 = { 0,0,0,0 };
	mem->coinRain.createEndApearence.color1 = { 0,0,0,0 };
	mem->coinRain.createEndApearence.size = { 10,10 };
	mem->coinRain.tranzitionType = gl2d::TRANZITION_TYPES::none;
	mem->coinRain.texturePtr = &mem->heart;
	mem->coinRain.deathRattle = &mem->coinEmiter;
	mem->coinRain.subemitParticle = &mem->downCoin;
	mem->coinRain.positionX = { };
	mem->coinRain.positionY = { };

#pragma endregion

	windowSettings->w = 940;
	windowSettings->h = 540;
}

//this might be usefull to change variables on runtime
extern "C" __declspec(dllexport) void onReload(GameMemory * mem, HeapMemory * heapMemory,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	console = &platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}

	gl2d::setErrorFuncCallback([](const char* c) {console->elog(c); });
	gl2d::init();
#pragma endregion

	platformFunctions->console.log("reloaded...");
	gl2d::setVsync(1);


}


extern "C" __declspec(dllexport) void gameLogic(GameInput * input, GameMemory * mem,
	HeapMemory * heapMemory, VolatileMemory * volatileMemory, GameWindowBuffer * windowBuffer,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{

#pragma region per frame setup
	allocator = &heapMemory->allocator;
	float deltaTime = input->deltaTime;
	console = &platformFunctions->console;

	glClear(GL_COLOR_BUFFER_BIT);
	//glViewport(0, 0, windowBuffer->w, windowBuffer->h);
	mem->renderer.updateWindowMetrics(windowBuffer->w, windowBuffer->h);

	if (windowSettings->fullScreen)
	{
		//mem->renderer.currentCamera.zoom = windowSettings->fullScreenZoon;
		mem->renderer.currentCamera.zoom = windowSettings->w / 1920.f;

	}
	else
	{
		windowSettings->w = 940;
		windowSettings->h = 540;
		mem->renderer.currentCamera.zoom = windowSettings->w / 1920.f;
	}

	//mem->renderer.currentCamera.position = { 0,0 };
	mem->renderer.currentCamera.follow({ gameSizeX / 2, gameSizeY / 2 }, deltaTime * 100, 30, windowBuffer->w, windowBuffer->h);


	float w = windowBuffer->w;
	float h = windowBuffer->h;

	auto& renderer = mem->renderer;


#pragma endregion

	if(isGoldRush)
	{
		platformFunctions->keepPlayingMusic("resources\\Audio\\goldRushMusic.mp3", 0.06);

	}else
	{
		platformFunctions->keepPlayingMusic("resources\\Audio\\Poker.mp3", 0.06);
	}

	platformFunctions->setMasterVolume(0.8);

	glm::vec4 textC;
	glm::vec4 textS;
	glm::vec4 textL;

	glm::vec4 flushText;

	{
		auto c = [](glm::vec3 v) {return v / 255.f; };

		mem->flushTextTime -= deltaTime;
		if(mem->flushTextTime < 0)
		{
			mem->flushTextState = !mem->flushTextState;
			mem->flushTextTime = 0.1;
		}

		if(mem->flushTextState)
		{
			flushText = { c({  252, 208, 13  }), 1 };
		}else
		{
			flushText = { c({ 255, 228, 213 }), 1 };
		}


		textC={c({ 252, 208, 13 }), 1  };
		textS={c({ 212, 141, 42 }), 1  };
		textL= { 1,0.9,0.3,0.1 } ;

	}

	renderer.renderRectangle({ 0, 0, gameSizeX, gameSizeY }, { 0.5f,0.5f,0.5f,1 }, {}, 0, mem->background);

	if (input->keyBoard[Button::F].released)
	{
		windowSettings->fullScreen = !windowSettings->fullScreen;
	}


#pragma region menu

	goldRushType = mem->goldRushType;
	isGoldRush = mem->goldRushTime > 0;

	if (mem->gameState == 0)
	{
		mem->goldRushTime = 0;

		renderer.renderRectangle({ gameSizeX/2.f - 400, gameSizeY / 2.f - 400, 800, 800 },
			 {}, mem->ruleteSpin, mem->rulette);

		if ((input->leftMouse.pressed || input->keyBoard[Button::Space].pressed)
			&& !mem->pressedSpin
			)
		{
			std::uniform_real_distribution<float> dist1(50, 61);
			std::uniform_real_distribution<float> dist2(5, 8);

			mem->pressedSpin = true;
			mem->spinSpeed = dist1(mem->random);
			mem->spinDrag = dist2(mem->random);

			platformFunctions->playSound("resources//audio//wheelSpin.mp3", 0.2);
		}

		if(mem->pressedSpin)
		{
			mem->spinSpeed -= deltaTime * mem->spinDrag;
			if(mem->spinSpeed < 0)
			{
				if (input->leftMouse.pressed || input->keyBoard[Button::Space].pressed)
				{
					//reset
					mem->spinSpeed = 0;

					mem->gameState = 1;
					mem->pressedSpin = 0;
					mem->spinDrag = 0;
					mem->spinSpeed = 0; //etc

					mem->gameTime = 60;
					mem->player.pos = { sidePannelSize , sidePannelSize };
					mem->fallingCards.clear();
					mem->cardHand.clear();
					mem->rightHand.clear();
					mem->fadeRightHand.clear();
					mem->fadeCardHand.clear();
					mem->fadeCardTime=0;
					mem->fadeRightCardTime=0;
					mem->timeTilllastFall = 0;
					mem->score = 0;
					mem->bigWinTime = 0;
					mem->currentSuite = 0;
					mem->fadeSuiteTime = 0;
					mem->flushTextTime = 0;
					mem->flushTextState = 0;
					mem->showMinusTimeTime = 0;
					mem->goldRushTime=0;
					isGoldRush = 0;

					
				}

				mem->goldRushType = (((int)mem->ruleteSpin % 360) / 90) %4;

				renderer.renderText({ gameSizeX / 2.f , gameSizeY / 2.f + 200 },
					goldRushName[mem->goldRushType], mem->font,
					{ textC }, 1.5, 2, 3, true,
					{ textS },
					{ textL });
				goldRushType = mem->goldRushType;

			}else
			{
				mem->ruleteSpin += mem->spinSpeed;
			}
		
		}else
		{
			renderer.renderText({ gameSizeX / 2.f , gameSizeY / 2.f },
				"Spin me!", mem->font,
				{ textC }, 1.1, 2, 3, true,
				{ textS },
				{ textL });


		}

		renderer.renderText({ 50, 50},
			"Press \'f\' for full screen", mem->font,
			{ textC }, 1.1, 2, 3, false,
			{ textS },
			{ textL });

	

		mem->renderer.flush();
		return;
	}

	mem->pressedSpin = 0;


#pragma endregion


	const float MINUS_TIME_TIME = 2;
	const float FADE_TIME = 2;
	const float SUITE_FADE_TIME = 4;
	const float BIG_WIN_FADE_TIME = 6;


#pragma region fall cards

	for(int i=0; i<mem->fallingCards.size();i++)
	{
		if(mem->fallingCards[i].pos.y > gameSizeY + 100)
		{
			mem->fallingCards.erase(mem->fallingCards.begin() + i);
			i--;
			continue;
		}


		//pickup
		//collide
		//pick card
		//collect colect
		if (phisics::circleBox(
			glm::vec2{ mem->player.pos } + glm::vec2(glm::vec2{mem->player.dimensions}/2.f),
			mem->player.dimensions.x / 2,
			{ mem->fallingCards[i].pos, cardSize * 0.9f }
		))
		{

			//penalty
			//adica negru
			if (mem->player.directionRight)
			{
				if(mem->fallingCards[i].card.suits == SUIT::clubs ||
					mem->fallingCards[i].card.suits == SUIT::spades
					)
				{

					if (mem->fallingCards[i].card.number <= 13)
					{
						mem->cardHand.insert(mem->cardHand.begin(), mem->fallingCards[i].card);
						platformFunctions->playSound("resources/audio/normalCollect.mp3", 0.06);

					}
					else
					{
						platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

						mem->ps.emitParticleWave(&mem->coinEmiter,
							glm::vec2{ mem->fallingCards[i].pos + cardSize / 2.f }
						);

						int bScore = 150;
						if(isGoldRush && goldRushType == triple_points)
						{
							bScore *= 3;
						}

						mem->score += bScore;

						int good = 1;

						for(auto &j: mem->rightHand)
						{
							if(j.suits == mem->fallingCards[i].card.suits)
							{
								good = false;
								break;
							}
						}

						if(good)
						{
							mem->rightHand.insert(mem->rightHand.begin(), mem->fallingCards[i].card);
						}

					}

				}else
				{
					//mem->gameTime -= 5;
					//mem->showMinusTimeTime = MINUS_TIME_TIME;

					for(int j=0; j<4;j++)
						mem->ps.emitParticleWave(&mem->shatterPart[j], 
							glm::vec2{ mem->fallingCards[i].pos } +
							(glm::vec2{ cardSize }/2.f)
						);

					mem->ps.emitParticleWave(&mem->basicParticle[mem->fallingCards[i].card.suits],
						glm::vec2{ mem->fallingCards[i].pos } +
						(glm::vec2{ cardSize } / 2.f));

					platformFunctions->playSound("resources\\Audio\\shatter.mp3", 0.05);


				}

			}else
			{
				if (mem->fallingCards[i].card.suits == SUIT::hearts ||
					mem->fallingCards[i].card.suits == SUIT::diamond
					)
				{

					//golden 
					if(mem->fallingCards[i].card.number <= 13)
					{
						mem->cardHand.insert(mem->cardHand.begin(), mem->fallingCards[i].card);
						platformFunctions->playSound("resources/audio/normalCollect.mp3", 0.08);


					}else
					{
						platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

						mem->ps.emitParticleWave(&mem->coinEmiter,
							glm::vec2{ mem->fallingCards[i].pos + cardSize / 2.f }
						);

						mem->score += 150;

						int good = 1;

						for (auto& j : mem->rightHand)
						{
							if (j.suits == mem->fallingCards[i].card.suits)
							{
								good = false;
								break;
							}
						}

						if (good)
						{
							mem->rightHand.insert(mem->rightHand.begin(), mem->fallingCards[i].card);
						}
					}

				}
				else
				{
					//mem->gameTime -= 5;
					//mem->showMinusTimeTime = MINUS_TIME_TIME;

					for (int j = 0; j < 4; j++)
						mem->ps.emitParticleWave(&mem->shatterPart[j],
							glm::vec2{ mem->fallingCards[i].pos } +
							(glm::vec2{ cardSize } / 2.f)
						);

					mem->ps.emitParticleWave(&mem->basicParticle[mem->fallingCards[i].card.suits],
						glm::vec2{ mem->fallingCards[i].pos } +
						(glm::vec2{ cardSize } / 2.f));

					platformFunctions->playSound("resources\\Audio\\shatter.mp3", 0.05);

				}
			}


			mem->fallingCards.erase(mem->fallingCards.begin() + i);
			i--;

			//mem->ps.emitParticleWave(&mem->coinEmiter, mem->player.pos);
		}

	}

	for(auto &i: mem->fallingCards)
	{
		i.pos.y += 110 * deltaTime;
		
	}

	//new card
	//spawn
	mem->timeTilllastFall -= deltaTime;

	if (mem->timeTilllastFall < 0)
	{
		std::uniform_real_distribution<float> timeDist(1, 3);

		mem->timeTilllastFall = timeDist(mem->random);

		if (goldRushType == more_cards && isGoldRush)
		{
			mem->timeTilllastFall / 2.8;
		}

		Card c = getArandomCardAsisted(mem->cardHand, mem->fallingCards, mem->random, mem->rightHand);

		std::uniform_real_distribution<float> dist(sidePannelSize, gameSizeX - sidePannelSize - cardSize.x);

		FallingCard f;
		f.card = c;
		f.pos.y = -cardSize.y;
		f.pos.x = dist(mem->random);

		mem->fallingCards.push_back(f);
		
	}

	for (auto& i : mem->fallingCards)
	{
		
		renderer.renderRectangle({ i.pos, cardSize }, {}, 0, mem->cards, mem->cardsAtlas.get(i.card.number - 1, i.card.suits));

	}

#pragma endregion

#pragma region deck



	//calculate suite
	if(mem->cardHand.size() >= 5)
	{

		while(mem->cardHand.size() > 5)
		{
			mem->cardHand.erase(mem->cardHand.begin());
		}


		mem->currentSuite = calculateScore(mem->cardHand);
		mem->fadeSuiteTime = SUITE_FADE_TIME;

		mem->score += getScor(mem->currentSuite);

		for(auto &i : mem->cardHand)
		{
			if(i.number == 1 || i.number >= 10)
			{
				mem->score += 5;
			}

		}


		mem->fadeCardHand = mem->cardHand;
		mem->fadeCardTime = FADE_TIME;

		mem->cardHand.clear();
	
		const char* sound[] =
		{
		"",
		"resources/audio/01.mp3",
		"resources/audio/02.mp3",
		"resources/audio/03.mp3",
		"resources/audio/04.mp3",
		"resources/audio/05.mp3",
		"resources/audio/06.mp3",
		"resources/audio/07.mp3",
		"resources/audio/08.mp3",
		"resources/audio/09.mp3",
		"resources/audio/10.mp3",
		};

		platformFunctions->playSound(sound[mem->currentSuite], 0.4);
	
		//celebrate

		switch (mem->currentSuite)
		{
		case NONE:
			break;
		case HIGH_CARD:
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[4], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[4], { 50,50 });

			mem->gameTime += 10;

			break;
		case ONE_PAIR:

			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[4], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[4], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });

			mem->gameTime += 20;

			break;
		case TWO_PAIR:

			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });

			mem->gameTime += 25;


			break;
		case THREE_OF_A_KIND:

			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->gameTime += 30;


			break;
		case STRAIGHT:

			mem->gameTime += 40;


			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;

			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);


			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });

			break;
		case FLUSH:
			
			mem->gameTime += 45;


			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;

			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 + 100, gameSizeY / 2 });

			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 + 100, gameSizeY / 2 });

			break;
		case FULLHOUSE:

			mem->gameTime += 50;


			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;

			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 + 100, gameSizeY / 2 });

			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 + 100, gameSizeY / 2 });


			break;
		case FOUR_OF_A_KIND:

			mem->gameTime += 55;

			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;
			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 + 100, gameSizeY / 2 });

			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 + 100, gameSizeY / 2 });

			break;
		case STRAIGHT_FLUSH:

			mem->gameTime += 60;

			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;
			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 + 100, gameSizeY / 2 });

			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { gameSizeX / 2 + 100, gameSizeY / 2 });

			break;
		case ROYAL_FLUSH:

			mem->gameTime += 80;

			platformFunctions->playSound("resources/audio/jackpot.ogg", 0.1);

			mem->ps.emitParticleWave(&mem->manyCoins, { 50,50 });
			mem->bigWinTime = BIG_WIN_FADE_TIME;
			mem->ps.emitParticleWave(&mem->coinEmiter, { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });
			mem->ps.emitParticleWave(&mem->basicParticle[8], { 50,50 });

			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->coinEmiter, { gameSizeX / 2 + 100, gameSizeY / 2 });

			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[6], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[7], { gameSizeX / 2 + 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { gameSizeX / 2 - 100, gameSizeY / 2 });
			mem->ps.emitParticleWave(&mem->basicParticle[5], { gameSizeX / 2 + 100, gameSizeY / 2 });

			break;
		case HAND_TYPE_COUNT:
	
			break;
		default:
			break;

		}


		float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
		float xPos = cardSizePanelSide / 4;
		for (int i = mem->fadeCardHand.size() - 1; i >= 0; i--)
		{
		
			if (mem->fadeCardHand[i].number == 1 || mem->fadeCardHand[i].number >= 10)
			{
				mem->ps.emitParticleWave(&mem->basicParticle[mem->fadeCardHand[i].suits], { xPos, yPos });
				mem->ps.emitParticleWave(&mem->basicParticle[mem->fadeCardHand[i].suits], { xPos, yPos });
				mem->ps.emitParticleWave(&mem->basicParticle[mem->fadeCardHand[i].suits], { xPos, yPos });
				mem->ps.emitParticleWave(&mem->basicParticle[mem->fadeCardHand[i].suits], { xPos, yPos });

			}
			yPos -= cardSizePanelSide + cardSizePanelSidePadding;

		}

	}



	if(mem->fadeCardHand.size())
	{
		mem->fadeCardTime -= deltaTime;

		if(mem->fadeCardTime <= 0)
		{
			mem->fadeCardHand.clear();
		}
	}


#pragma endregion

	
	
#pragma region player

		if(mem->player.pos.x < sidePannelSize)
		{
			mem->player.pos.x = sidePannelSize;
			mem->player.directionRight = true;
			mem->player.velocity.x = mem->player.xNormalVelocity;
		}else if(mem->player.pos.x + mem->player.dimensions.x > gameSizeX - sidePannelSize)
		{
			mem->player.pos.x = gameSizeX - sidePannelSize - mem->player.dimensions.x;
			mem->player.directionRight = false;
			mem->player.velocity.x = -mem->player.xNormalVelocity;
		}


		if(input->leftMouse.pressed || input->keyBoard[Button::Space].pressed)
		{
			
			if(mem->player.velocity.x > 0)
			{
				mem->player.velocity.x = mem->player.xNormalVelocity + 150;
			}else
			{
				mem->player.velocity.x = -mem->player.xNormalVelocity - 150;
			}


			if(mem->player.velocity.y > 0)
			{
				mem->player.velocity.y = 0;
			}

			mem->player.velocity.y += -600;
		}


		mem->player.applyGravity(deltaTime);

		mem->player.applyVelocity(deltaTime);

		//mem->player.resolveConstrains(mem->mapData);

		mem->player.updateMove();
		

		mem->player.xMaxVelocity = 600;
		mem->player.xNormalVelocity = 370;
		mem->player.xSlowDownSpeed = 200;

		mem->player.yMaxVelocity = 1000;
		mem->player.yNormalVelocity = 900;
		mem->player.ySlowDownSpeed = 2000;


		if(mem->player.pos.y < 0)
		{
			mem->player.pos.y = 0;
		}

		if (mem->player.pos.y + mem->player.dimensions.y > gameSizeY)
		{
			mem->player.pos.y = gameSizeY - mem->player.dimensions.y;
		}


		if(!mem->player.directionRight)
		{
			mem->playerRotation += 110 * deltaTime;

		}else
		{
			mem->playerRotation -= 110 * deltaTime;
		}

		renderer.renderRectangle({ mem->player.pos, mem->player.dimensions }, {}, mem->playerRotation, mem->playerTexture,
			mem->playerAtlas.get(mem->player.directionRight, 0));


#pragma endregion


#pragma region draw card on the side


		//renderer.renderRectangle({ 0,0, sidePannelSize, gameSizeY }, { 0.1,0.1,0.1,0.4 });
		renderer.renderRectangle({ 0,0, sidePannelSize, gameSizeY },
			{ 0.7,0.7,0.7, 0.9 },
			{}, {}, mem->wallTexture, mem->wallAtlas.get(0,0));


		//draw back
		{
			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = cardSizePanelSide / 4;
			for (int i = 4; i >= 0; i--)
			{


				renderer.renderRectangle({ xPos, yPos, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{ 0,0 }, 90, mem->backCard
				);


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}
		}

		

		//draw fade cards
		if (mem->fadeCardHand.size())
		{
			float fadeAnim = mem->fadeCardTime / FADE_TIME;
			fadeAnim = pow(fadeAnim, 2);
			//fadeAnim = sqrt(fadeAnim);

			float bonusXpadd = std::min(1.f, (1 - fadeAnim) * 3) * cardSize.y/2;

			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = cardSizePanelSide / 4 + bonusXpadd;
			for (int i = mem->fadeCardHand.size() - 1; i >= 0; i--)
			{

				glm::vec4 atlas = mem->cardsAtlas.get(mem->fadeCardHand[i].number - 1, mem->fadeCardHand[i].suits);

				//atlas = { atlas.w, atlas.x, atlas.y, atlas.z };

				renderer.renderRectangle({ xPos * fadeAnim, yPos * fadeAnim, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{1,1,1, fadeAnim},
					{ 0,0 }, 90, mem->cards, atlas
				);


				if(mem->fadeCardHand[i].number == 1 || mem->fadeCardHand[i].number >= 10)
				{
					renderer.renderText({  (xPos + 80 + cardSizePanelSide * cardAspectRatio) * sqrt(fadeAnim), 
						yPos * sqrt(fadeAnim) },
						"+5K", mem->font, 
						{ glm::vec3(textC) ,fadeAnim }, 1.1, 2, 3, false,
						{ glm::vec3(textS),fadeAnim },
						{ glm::vec3(textL),  fadeAnim });


				}


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}
			
		}

		

		//draw cards
		if(mem->cardHand.size())
		{
			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = cardSizePanelSide / 4;
			for (int i = mem->cardHand.size()-1; i >= 0; i--)
			{

				glm::vec4 atlas = mem->cardsAtlas.get(mem->cardHand[i].number-1, mem->cardHand[i].suits);

				//atlas = { atlas.w, atlas.x, atlas.y, atlas.z };

				renderer.renderRectangle({ xPos, yPos, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{ 0,0 }, 90, mem->cards, atlas
				);


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}
		}
	
		
#pragma region draw score


		std::string s = std::to_string(mem->score);
		s += "K";

		renderer.renderText({ 20 ,100 }, s.c_str(), mem->font,
			textC, 1.1, 2, 3, false,
			textS, 
			textL);


#pragma endregion

#pragma region draw time

		mem->gameTime = std::min(mem->gameTime, 120.f);

		mem->gameTime -= deltaTime;

		{

			auto c = textC;

			if(mem->gameTime < 25)
			{
				c = flushText;
				platformFunctions->keepPlayingMusic("resources\\Audio\\ticking.mp3", 0.2);
			}

			renderer.renderText({ gameSizeX / 2.f , 50 }, minute_sec((int)mem->gameTime).c_str(),
				mem->font,
				c, 1.5, 4, 3, true,
				textS,
				textL
			);

			if(mem->gameTime < 0)
			{
				//dead
				//game over
				//died
				//kill
				
				platformFunctions->playSound("resources\\Audio\\gameOver.mp3", 0.2);

				mem->gameState = 0;
			
			}

		}

		//{ 0.9, 0.8, 0.6, 1 }
		//{ 0.1, 0.1, 0.1, 1 }
		//{ 1, 0.9, 0.3, 0.1 }

		renderer.renderRectangle({ gameSizeX / 2.f + 80 , 30, 80, 80 }, {}, 0, mem->clock);

		if(mem->showMinusTimeTime > 0)
		{
			mem->showMinusTimeTime -= deltaTime;
		
			float f = mem->showMinusTimeTime / MINUS_TIME_TIME;

			renderer.renderText({ gameSizeX / 2.f + 210 , 40*f + 10 }, "-5s",
				mem->font,
				flushText, 1.5, 4, 3, true,
				textS,
				textL
			);
		}

#pragma endregion

	
		//draw suite win
		if (mem->currentSuite && mem->fadeSuiteTime >= 0)
		{
			mem->fadeSuiteTime -= deltaTime;
			float fadeAnim = mem->fadeSuiteTime / SUITE_FADE_TIME;
			fadeAnim = pow(fadeAnim, 2);

			float rAnim = 1 - fadeAnim;

			if (mem->currentSuite)
			{

				auto c = textC;

				if(mem->currentSuite > 3)
				{
					c = flushText;
				}

				renderer.renderText({ gameSizeX / 2.f , 250 - 100 * rAnim },
					getHandName(mem->currentSuite),
					mem->font, 
					c, 1.5 + 1 * rAnim , 4 + 1*rAnim, 3, true,
					textS,
					textL
					);

				std::string score = "+";
				int scor = 0;

				scor = getScor(mem->currentSuite);
			

				score += std::to_string(scor);
				score += "K";

				if(scor)
				{
					float mul = 1;
					
					if(fadeAnim < 0.5)
					{
						mul = fadeAnim / 0.5;
					}

					renderer.renderText({ (gameSizeX / 2.f) * mul, (450 - 100 * rAnim) * mul },
						score.c_str(),
						mem->font, textC, 1.5 + 0.5 * rAnim, 4 + 0.5 * rAnim, 3, false,
						textS,
						textL
					);
				
				}
			
			}


		}

		if(mem->bigWinTime > 0)
		{
			mem->bigWinTime -= deltaTime;
			if(mem->bigWinTime < 0)
			{
				mem->bigWinTime = 0;
			}

			mem->bigWinTime -= deltaTime;
			float fadeAnim = mem->bigWinTime / BIG_WIN_FADE_TIME;
			fadeAnim = pow(fadeAnim, 2);

			float rAnim = 1 - fadeAnim;

			renderer.renderText({ gameSizeX / 2.f , 550 - 100 * rAnim },
				"BIG WIN!",
				mem->font, flushText, 1.8 + 1 * rAnim, 4.1 + 1 * rAnim, 3, true,
				textS,
				textL
			);

		
		}


#pragma endregion

#pragma region draw to the right side

		//renderer.renderRectangle({ gameSizeX- sidePannelSize, 0, sidePannelSize, gameSizeY }, { 0.1,0.1,0.1,0.4 });
		renderer.renderRectangle({ gameSizeX - sidePannelSize, 0, sidePannelSize, gameSizeY },
			{ 0.7,0.7,0.7, 0.9 },
			{}, {}, mem->wallTexture, mem->wallAtlas.get(1, 0));

		//draw back
		{
			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = gameSizeX - cardSizePanelSide;
			for (int i = 3; i >= 0; i--)
			{

				renderer.renderRectangle({ xPos, yPos, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{ 0,0 }, 90, mem->goldBack
				);


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}
		}

		//draw fade cards
		if (mem->fadeRightHand.size())
		{
			mem->fadeRightCardTime -= deltaTime;
			float fadeAnim = mem->fadeRightCardTime / FADE_TIME;
			fadeAnim = pow(fadeAnim, 2);
			//fadeAnim = sqrt(fadeAnim);

			float bonusXpadd = std::min(1.f, (1 - fadeAnim) * 3) * cardSize.y / 2;

			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = gameSizeX - cardSizePanelSide - bonusXpadd - cardSize.x;
			for (int i = mem->fadeRightHand.size() - 1; i >= 0; i--)
			{

				glm::vec4 atlas = mem->cardsAtlas.get(mem->fadeRightHand[i].number - 1, mem->fadeRightHand[i].suits);

				//atlas = { atlas.w, atlas.x, atlas.y, atlas.z };

				renderer.renderRectangle({ xPos * fadeAnim + (1- fadeAnim)*(gameSizeX - cardSize.x)
					, yPos * fadeAnim, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{ 1,1,1, fadeAnim },
					{ 0,0 }, 90, mem->cards, atlas
				);


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}

		}


		if(mem->fadeRightCardTime <0)
		{
			mem->fadeRightCardTime = 0;
			mem->fadeRightHand.clear();

		}

		if (mem->rightHand.size())
		{
			float yPos = gameSizeY - cardSizePanelSidePadding - cardSizePanelSide - cardSizePanelSide / 2;
			float xPos = gameSizeX - cardSizePanelSide ;
			for (int i = mem->rightHand.size() - 1; i >= 0; i--)
			{

				glm::vec4 atlas = mem->cardsAtlas.get(mem->rightHand[i].number - 1, mem->rightHand[i].suits);

				//atlas = { atlas.w, atlas.x, atlas.y, atlas.z };

				renderer.renderRectangle({ xPos, yPos, cardSizePanelSide, cardSizePanelSide * cardAspectRatio },
					{ 0,0 }, 90, mem->cards, atlas
				);


				yPos -= cardSizePanelSide + cardSizePanelSidePadding;
			}
		}

#pragma endregion

#pragma region gold rush

		{

			if (mem->rightHand.size() >= 4)
			{
				mem->fadeRightCardTime = FADE_TIME;
				mem->fadeRightHand = mem->rightHand;

				mem->rightHand.clear();

				mem->goldRushTime = GOLD_RUSH_TIME;

				mem->ps.emitParticleWave(&mem->coinRain, { gameSizeX - 100, -200 });
				mem->ps.emitParticleWave(&mem->coinRain, { gameSizeX - 150, -200 });
				mem->ps.emitParticleWave(&mem->coinRain, { gameSizeX - 50, -200 });
				
				platformFunctions->playSound("resources//audio//goldRush.mp3", 0.4);
			}


			if (mem->goldRushTime > 0)
			{
				mem->goldRushTime -= deltaTime;

				if (mem->goldRushTime < 0)
				{
					mem->goldRushTime = 0;
				}

				renderer.renderText({ gameSizeX - 250 ,
						100 },
					"GOLD RUSH!", mem->font,
					{ flushText }, 1.5, 4, 3, true,
					{ textS },
					{ textL });

			}


		}

#pragma endregion


		mem->ps.applyMovement(deltaTime);
		mem->ps.draw(mem->renderer);

		mem->renderer.flush();


	//mem->renderer.renderRectangle({ 10,10,100,100 }, Colors_Green);
	//mem->renderer.flush();

}

extern "C" __declspec(dllexport) void onClose(GameMemory * mem, HeapMemory * heapMemory,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	console = &platformFunctions->console;


#pragma endregion




}