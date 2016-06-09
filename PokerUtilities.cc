#include "PokerUtilities.h"

using namespace UtilNS;

int Rand(int a, int b){
  if(a==b) return a;
  return rand() % (b-a) + a;
}

/////// Deck Stuff ///////
void Deck::ShuffleDeck(){
  for(unsigned int it=0; it<fDeck.size(); it++){
    delete (fDeck[it]);
  }
  fDeck.clear();

  //Fill a list of ordered cards
  int org[SUITS*MAXVAL];
  for(int suit=0; suit<SUITS; suit++){
    for(int val=0; val<MAXVAL; val++){
      org[val+MAXVAL*suit] = (val+2)*10+suit;  //Cards start at 2
    }
  }

  for(int i=SUITS*MAXVAL-1; i>=0; i--){
    int irand=Rand(0,i); //Get a random card from list
    Card *card = new Card(org[irand]); //create new card with that value
    fDeck.push_back(card);  //add to list
    org[irand]=org[i];  //move the current last card down
  }
  if(Verbose>1)
    cerr<<"Deck Reset\n";
}

//Gets the card pointer from the deck and removes it from the deck
Card* Deck::DrawCard(){
  Card* card = fDeck[fDeck.size()-1];  //get the card
  fDeck.erase(fDeck.begin()+fDeck.size()-1); //clear the vector entry

  if(!GetCardsInDeck()) cerr<<"Warning, that was the last card\n";

  return card;
}

//For debugging
void Deck::PrintDeck(){
  cerr<<"-----\n";
  for(int i=0;i<GetCardsInDeck(); i++){
    fDeck[i]->PrintCard();
  }
  cerr<<"-----\n";
}


//////Discard Pile Stuff///////
//Add the card to the discard pile (mostly for debugging)
void DiscardPile::AddCardToDiscardPile(Card *card){
  fDiscard.push_back(card);
}

//Clear all the cards and memory
void DiscardPile::ClearDiscardPile(){
  for(unsigned int it=0; it<fDiscard.size(); it++){
    delete (fDiscard[it]);
  }
  fDiscard.clear();
}


///////Table Stuff///////
void Table::ClearTable(){
  for(unsigned int it=0; it<fTable.size(); it++){
    delete (fTable[it]);
  }
  fTable.clear();
}




///////Hand Stuff///////
void Hand::ClearHand(){
  for(unsigned int it=0; it<fHand.size(); it++){
    delete(fHand[it]);
  }
  fHand.clear();
}



///////Game Stats stuff///////

void GameStats::Init(){
  YourPosition=0;
  for(int i=0; i<NoOfPlayers; i++){
    PlayerChipCounts[i] = STARTINGCHIPS;
    Bets[i]=0;
    ActiveInHand[i]=1;
    ActiveInGame[i]=1;
  }
  //Randomize the blinds and dealer
  LittleBlind = Rand(0,NoOfPlayers-1);
  BigBlind = (LittleBlind + 1) % (NoOfPlayers);
  Dealer = (BigBlind + 1) % (NoOfPlayers);
  GameStage = 0;
  HandNumber=0;
  TheTable.ClearTable();
  ThePot = 0;
  CurrentBet = 0;
  YourHand.ClearHand();
}

void GameStats::UpdateGameStats(GameStats *gs, int pos){
  NoOfPlayers = gs->NoOfPlayers;
  PlayerChipCounts.clear();
  ActiveInHand.clear();
  ActiveInGame.clear();
  Bets.clear();
  for(unsigned int i=0; i<gs->PlayerChipCounts.size(); i++){
    PlayerChipCounts.push_back(gs->PlayerChipCounts[i]);
    Bets.push_back(gs->Bets[i]);
    ActiveInHand.push_back(gs->ActiveInHand[i]);
    ActiveInGame.push_back(gs->ActiveInGame[i]);
  }
  YourPosition=pos;
  Dealer=gs->Dealer;
  BigBlind=gs->BigBlind;
  LittleBlind=gs->LittleBlind;
  GameStage=gs->GameStage;
  HandNumber=gs->HandNumber;
  ThePot=gs->ThePot;
  CurrentBet=gs->CurrentBet;
  if(gs->TheTable.GetCardsOnTable() != TheTable.GetCardsOnTable()){
    TheTable.ClearTable();
    for(int i=0; i<gs->TheTable.GetCardsOnTable(); i++){
      Card *CardCopy = new Card(gs->TheTable.fTable[i]->GetCard());
      TheTable.AddCardToTable(CardCopy);
    }
  }
}

void GameStats::UpdateGameStats(GameStats *gs, int pos, Hand* hand){
  NoOfPlayers = gs->NoOfPlayers;
  PlayerChipCounts.clear();
  ActiveInHand.clear();
  ActiveInGame.clear();
  Bets.clear();
  for(unsigned int i=0; i<gs->PlayerChipCounts.size(); i++){
    PlayerChipCounts.push_back(gs->PlayerChipCounts[i]);
    Bets.push_back(gs->Bets[i]);
    ActiveInHand.push_back(gs->ActiveInHand[i]);
    ActiveInGame.push_back(gs->ActiveInGame[i]);
  }
  YourPosition=pos;
  Dealer=gs->Dealer;
  BigBlind=gs->BigBlind;
  LittleBlind=gs->LittleBlind;
  GameStage=gs->GameStage;
  HandNumber=gs->HandNumber;
  ThePot=gs->ThePot;
  CurrentBet=gs->CurrentBet;
  if(gs->TheTable.GetCardsOnTable() != TheTable.GetCardsOnTable()){
    TheTable.ClearTable();
    for(int i=0; i<gs->TheTable.GetCardsOnTable(); i++){
      Card *CardCopy = new Card(gs->TheTable.fTable[i]->GetCard());
      TheTable.AddCardToTable(CardCopy);
    }
  }
  
  if(!YourHand.fHand.size()){
    Card* copy = new Card(hand->fHand[0]->GetCard());
    YourHand.fHand.push_back(copy);
    copy = new Card(hand->fHand[1]->GetCard());
    YourHand.fHand.push_back(copy);
    }
}

void GameStats::DumpStats(){
  cerr<<"---------------------------------"<<endl;
  cerr<<
    "No of players: "<<NoOfPlayers<<endl;
  for(int i=0; i<PlayerChipCounts.size(); i++){
    cerr<<"Player: "<<i+1<<endl;
    cerr<<"\tChips:\t\t"<<PlayerChipCounts[i]<<endl;
    cerr<<"\tCur Bet:\t"<<Bets[i]<<endl;
    cerr<<"\tIn Hand:\t"<<(ActiveInHand[i] ? "yes":"no")<<endl;
    cerr<< "\tIn Game:\t"<<(ActiveInGame[i] ? "yes":"no")<<endl;
  }
  cerr<<"Dealer is player: "<<Dealer+1<<endl<<
    "Big Blind is player: "<<BigBlind+1<<endl<<
    "Lil Blind is player: "<<LittleBlind+1<<endl<<
    "Game stage: "<<GameStage<<endl<<
    "Hand number: "<<HandNumber<<endl<<
    "Current Pot: "<<ThePot<<endl<<
    "Current Bet: "<<CurrentBet<<endl<<
    "Cards on Table: "<<TheTable.fTable.size()<<endl;
  cerr<<"---------------------------------"<<endl;
}


///////Player stuff//////
int Player::GetBet(){
  int val = fBettingStrategy(LocalGameStats);
  if(val < 1 && val != CHECK && val != FOLD)
    return FOLD;
  return val;
}