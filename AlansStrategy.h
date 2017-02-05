#ifndef POKER_H_
#define POKER_H_
#include "Poker.h"
#endif

#include <iomanip>


namespace Alan {
double HandValue(int A[], int N);
double CurrentBestScore(GameStats Stats);
void PossibleScores(GameStats Stats);
void OpponentsPossibleScores(Table table, int MyCard1, int MyCard2);

int AlansBettingStrategy(GameStats stats);
int BotBettingStrategy(GameStats stats);
int Bot2BettingStrategy(GameStats stats);
}