#pragma once
#include <vector>
#include "glm/vec2.hpp"
#include <random>

enum GOLD_RUSH_TYPE
{
	high_card,
	more_cards,
	one_type,
	triple_points
};

enum SUIT
{
	hearts = 0,
	spades,
	diamond,
	clubs
};

enum HAND_TYPE
{
	NONE = 0,
	HIGH_CARD = 1,
	ONE_PAIR,
	TWO_PAIR,
	THREE_OF_A_KIND,
	STRAIGHT,
	FLUSH,
	FULLHOUSE,
	FOUR_OF_A_KIND,
	STRAIGHT_FLUSH,
	ROYAL_FLUSH,
	HAND_TYPE_COUNT
};

const char* getHandName(int type);

struct Card
{
	int number;
	int suits;
	

};

struct FallingCard
{
	Card card;

	glm::vec2 pos;


};

inline bool operator==(const Card& first, const Card& other)
{
	return first.number == other.number && first.suits == other.suits;
}

int calculateScore(std::vector<Card>& v);

Card getArandomCard(std::vector<Card>& hand, std::vector<FallingCard>& existing, std::mt19937& random);
Card getArandomCardAsisted(std::vector<Card>& hand, std::vector<FallingCard>& existing, std::mt19937& random, std::vector<Card>& rightHad);
