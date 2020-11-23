#include "Cards.h"
#include <algorithm>


extern int goldRushType;
extern int isGoldRush;

const char* handNames[HAND_TYPE_COUNT] = 
{
	"",
	"High card",
	"One pair",
	"Two pairs",
	"Three of a kind",
	"Straight",
	"Flush",
	"Fullhouse",
	"Four of a kind",
	"Straight flush",
	"Royal flush"
};

const char* getHandName(int type)
{

	HAND_TYPE t = (HAND_TYPE)type;
	if(t >= HAND_TYPE_COUNT || t < 0)
	{
		return handNames[0];
	}else
	{
		return handNames[t];
	}

}


int calculateScore(std::vector<Card>& v)
{
	if (v.size() != 5)
	{
		return 0;
	}

	std::sort(v.begin(), v.end(), [](const Card& c1, const Card& c2)
		{
			return c1.number < c2.number;
		});

	int type = 0;

	bool sameColor = 1;
	int twoOfAKind = 0;
	int isTwo = 0;
	int threeOfAKind = 0;
	int isTree = 0;
	bool straight = 0;

	//sameColor
	{
		int color = v[0].suits;
		for (int i = 1; i < 5; i++)
		{
			if (color != v[i].suits)
			{
				sameColor = 0;
				break;
			}
		}
	}

	//more of a kind
	{
		for (int i = 0; i < 3; i++)
		{
			if
				(
					v[i + 0].number == v[i + 1].number
					&&
					v[i + 1].number == v[i + 2].number
					)
			{
				threeOfAKind = v[i].number;
				isTree = 1;
				break;
			}
		}

		for (int i = 0; i < 4; i++)
		{
			if
				(
					v[i + 0].number == v[i + 1].number
					&& v[i].number != threeOfAKind
					)
			{
				twoOfAKind = v[i].number;
				isTwo = 1;
				break;
			}
		}
	}

	//straight
	{
		straight = true;
		for (int i = 1; i < 5; i++)
		{
			if (v[i].number != v[i - 1].number + 1)
			{
				straight = false;
				break;
			}
		}

		if(
			v[0].number == 1 &&
			v[1].number == 10 &&
			v[2].number == 11 &&
			v[3].number == 12 &&
			v[4].number == 13 
			)
		{
			straight = true;
		}

	}


	//ROYAL_FLUSH,
	if (
		v[0].number == 1 &&
		v[1].number == 10 &&
		v[2].number == 11 &&
		v[3].number == 12 &&
		v[4].number == 13 &&
		sameColor
		)
	{
		return ROYAL_FLUSH;
	}


	//STRAIGHT_FLUSH,
	{
		if(straight && sameColor)
		{
			return STRAIGHT_FLUSH;
		}
	}


	//FOUR_OF_A_KIND,
	{
		if (
			(
				v[1].number == v[0].number &&
				v[2].number == v[0].number &&
				v[3].number == v[0].number
				)
			||
			(
				v[1].number == v[4].number &&
				v[2].number == v[4].number &&
				v[3].number == v[4].number
				)
			)
		{
			return FOUR_OF_A_KIND;
		}

	}

	//FULLHOUSE,
	{
		if (isTree && isTwo)
		{
			return FULLHOUSE;
		}

	}


	//FLUSH,
	{
		if ( sameColor)
		{
			return FLUSH;
		}

	}

	//STRAIGHT,
	{
		if (straight)
		{
			return STRAIGHT;
		}

	}

	//THREE_OF_A_KIND,
	{
		if (isTree)
		{
			return THREE_OF_A_KIND;
		}

	}

	//TWO_PAIR,
	{
		bool is = 0;

		for (int i = 0; i < 4; i++)
		{
			if
				(
					v[i + 0].number == v[i + 1].number
					&& v[i].number != twoOfAKind
					)
			{
				is = v[i].number;
				break;
			}
		}

		if (is)
		{
			return TWO_PAIR;
		}

	}


	//ONE_PAIR,
	{
		if (isTwo)
		{
			return ONE_PAIR;
		}

	}

	//HIGH_CARD ,
	{
		if (
			v[0].number == 1 || v[0].number > 10 ||
			v[1].number == 1 || v[1].number > 10 ||
			v[2].number == 1 || v[2].number > 10 ||
			v[3].number == 1 || v[3].number > 10 ||
			v[4].number == 1 || v[4].number > 10
			)
		{
			return HIGH_CARD;
		}
	}

	return NONE;

}

Card getArandomCardAsisted(std::vector<Card>& hand, std::vector<FallingCard>& existing, std::mt19937& random, 
	std::vector<Card>& rightHad
	)
{

	std::uniform_int_distribution<int> dist(0, 10);

	if(rightHad.size() == 3 && dist(random)== 1)
	{
		int s = 0 + 1 + 2 + 3;
		for(auto &i: rightHad)
		{
			s -= i.suits;
		}
	
		Card c;
		c.number = 14;
		c.suits = s;

		return c;
	}

	if(hand.size() != 4)
	{
		return getArandomCard(hand, existing, random);
	}

	const int S = 15;

	Card c[S];

	for(int i=0; i <S; i++)
	{
		c[i] = getArandomCard(hand, existing, random);
		if(c[i].number == 14)
		{
			i--;
		}
	}
	
	int maxScore = 0;
	int pos = 0;

	for (int i = 0; i < S; i++)
	{
		std::vector<Card> tempHand = hand;
		tempHand.push_back(c[i]);

		int s = calculateScore(hand);
		if(s > maxScore)
		{
			maxScore = 0;
			pos = i;
		}
	}

	return c[pos];

}

Card getArandomCard(std::vector<Card> &hand, std::vector<FallingCard> &existing, std::mt19937& random)
{
	Card c;

	std::uniform_int_distribution<int> distValue(1, 14);

	if(goldRushType == high_card && isGoldRush)
	{
		distValue = std::uniform_int_distribution<int>(10, 15);
	}

	std::uniform_int_distribution<int> distSuit(0, 3);

	bool run = 1;
	int count = 0;
	while(run)
	{
		run = 0;
		c.suits = distSuit(random);
		c.number = distValue(random);

		if (goldRushType == high_card && isGoldRush && c.number == 15)
		{
			c.number = 1;
		}
		
		if (goldRushType == one_type && isGoldRush )
		{
			c.suits = 0;
		}

		for(auto &i :hand)
		{
			if(c == i)
			{
				run = 1;
				break;
			}
		}
		
		for (auto& i : existing)
		{
			if (c == i.card)
			{
				run = 1;
				break;
			}
		}


		count ++;
		if(count > 40)
		{
			break;
		}

	}

	return c;
}
