#include <iostream>
#include <vector>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>  //Sort


using namespace std;

const int MAXVAL=13;
const int SUITS=4;
const int STARTINGCHIPS=100;
const int BIGBLINDBET=4;
const int LILBLINDBET=1;

const int CHECK=0;
const int FOLD=-1;
const int DRAW=-1;

const int Verbose=0;

enum Suit{
  Spade=0,
  Heart=1,
  Club=2,
  Diamond=3
};

const Suit gSuits[SUITS]={Spade,Heart,Club,Diamond};
const string fSuitNames[SUITS]={"Spades","Hearts","Clubs","Diamonds"};

/////
class Card{
 public:
  Card(int Value, Suit suit){
    fValue=Value;
    fSuit=suit;
  }

  Card(int Value){
    fValue=Value/10;
    fSuit=gSuits[Value%10];
  }

  int GetValue(){return fValue;}
  Suit GetSuit(){return fSuit;}
  int GetCard(){return fValue*10+fSuit;}//Alan test this
  void PrintCard(){cerr<<fValue<<" of "<<fSuitNames[fSuit]<<endl;}

 private:
  int fValue;
  Suit fSuit;
};


class Deck{
 public:
  Deck(){
    fDeck.reserve(SUITS*MAXVAL);
  }

  void ShuffleDeck();
  int GetCardsInDeck(){return fDeck.size();}
  Card* DrawCard();
  void PrintDeck();

 private:
  vector <Card*> fDeck;
};


class DiscardPile{
 public:
  DiscardPile(){
    fDiscard.reserve(SUITS*MAXVAL);
  }
  vector <Card*> fDiscard;

  int GetCardsInDiscard(){return fDiscard.size();}
  void AddCardToDiscardPile(Card * card);
  void ClearDiscardPile();
};


class Table{
 public:
  Table(){}
  vector <Card*> fTable;

  int GetCardsOnTable(){return fTable.size();}
  void ClearTable();
  void AddCardToTable(Card* card){fTable.push_back(card);}
};


class Hand{
 public:
  Hand(){}
  void AddCardToHand(Card* card){fHand.push_back(card);}
  void ClearHand();

  vector <Card*> fHand;
};

class GameStats{
 public:
  GameStats(){
    NoOfPlayers=0;
    BigBlind=0;
    LittleBlind=0;
    Dealer=0;
    GameStage=-1;
    HandNumber=0;
    ThePot=-1;
    CurrentBet=-1;
  };
  int NoOfPlayers;
  int YourPosition;
  vector <int> PlayerChipCounts;
  vector <int> Bets;
  vector <int> ActiveInHand;
  vector <int> ActiveInGame;
  int Dealer;
  int BigBlind;
  int LittleBlind;
  double GameStage;
  int HandNumber;
  Table TheTable;
  int ThePot;
  int CurrentBet;
  Hand YourHand;

  void UpdateGameStats(GameStats *gs, int pos);
  void UpdateGameStats(GameStats *gs, int pos, Hand* hand);
  void DumpStats();
  void Init();
};


class Player{
 public:
  Player(int (*Strategy)(GameStats)){
    fChipCount=STARTINGCHIPS;
    fBettingStrategy = Strategy;
  }

  int GetChipCount(){return fChipCount;}
  int GetBet();

  GameStats LocalGameStats;

  int (*fBettingStrategy)(GameStats);
  Hand fPlayerHand;
  int fChipCount;
};



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

int Rand(int a, int b);
