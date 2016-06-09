#include <iostream>
#include <vector>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>  //Sort

#include "PokerUtilities.h"


using namespace std;
using namespace UtilNS;


class Game{
 public:
  Game(){
    fMasterTable.ClearTable();
    fDeck = new Deck();
    fDiscardPile = new DiscardPile();
    ClearCards();
  }

  void AddPlayer(Player* player);
  vector <Player> fPlayers;
  void ClearCards();
  void DoHand();
  void DoPhase();
  void DoBettingRound();
  void FoldPlayer(int index);
  void HandleBet(int bet, int i);
  void RevealCards();
  void AddToPot(int amt, int plyr);
  void DealCards();
  int CheckForWinner();
  double ScoreHand(Hand * hand);
  double ScoreHand(int a, int s, int d, int f, int g);
  void EndTheHand();
  void PlayGame();

  GameStats MasterGameStats;

  vector <int> fWins;
  vector <int> TotalHands;

  Table fMasterTable;
  vector <Hand*> fPlayerHands;
  Deck* fDeck;
  DiscardPile* fDiscardPile;
};


namespace Alan{
  int AlansBettingStrategy(GameStats stats);

  int BotBettingStrategy(GameStats stats);

  int Bot2BettingStrategy(GameStats stats);
}


