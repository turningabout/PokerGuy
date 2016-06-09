#include "Poker.h"
#include <iomanip>

int main(){

  /* initialize random seed: */
  srand (time(NULL));

  cout<<"\nWelcome to Poker\n\n";

  Game* TheGame = new Game();
  Player *Alan = new Player(&Alan::AlansBettingStrategy);
  Player *Bot = new Player(&Alan::BotBettingStrategy);
  Player *Bot2 = new Player(&Alan::Bot2BettingStrategy);
  
  TheGame->AddPlayer(Alan);
  TheGame->AddPlayer(Bot);
  TheGame->AddPlayer(Bot2);

  TheGame->MasterGameStats.Init();

  int GamesToPlay = 1;


  for(int i=0; i<GamesToPlay; i++){
    if(!(i%20)) cerr<<i<<" / "<<GamesToPlay<<endl;
    TheGame->PlayGame();
  }

  cerr<<"Wins:\t";
  for(int i=0; i<TheGame->fWins.size(); i++)
    cerr<<TheGame->fWins[i]<<'\t';

  cerr<<endl;
  //cerr<<setprecision(17)<<"Score: "<<TheGame->ScoreHand(61,71,71,61,61)<<endl;

  return 0;
}




///////Game Control///////

//Run the game until only one player is left
void Game::PlayGame(){
  int Continue=MasterGameStats.ActiveInGame.size();
  
  for(int ply=0; ply<MasterGameStats.ActiveInGame.size(); ply++){
    MasterGameStats.ActiveInGame[ply]=1;
  }
  
  MasterGameStats.Init();

  while(Continue>1){
    DoHand();
    
    Continue=0;
    for(int ply=0; ply<MasterGameStats.ActiveInGame.size(); ply++){
      Continue+=MasterGameStats.ActiveInGame[ply];
    }
  }

  for(int ply=0; ply<MasterGameStats.ActiveInGame.size(); ply++){
    if(MasterGameStats.ActiveInGame[ply]) fWins[ply]++;
  }

  TotalHands.push_back(MasterGameStats.HandNumber);
  if(Verbose>0) cerr<<"Game took "<<MasterGameStats.HandNumber<<" hands "<<endl;
}

//Add player to the game and update the master game stats
void Game::AddPlayer(Player* player){
  fPlayers.push_back(*player);
  fWins.push_back(0);

  MasterGameStats.NoOfPlayers++;
  MasterGameStats.PlayerChipCounts.push_back(STARTINGCHIPS);
  MasterGameStats.ActiveInHand.push_back(1);
  MasterGameStats.ActiveInGame.push_back(1);
  MasterGameStats.Bets.push_back(0);
}


//Deal, bet, etc. for one hand of poker
void Game::DoHand(){
  int NPly = MasterGameStats.NoOfPlayers;

  //Set the game stage and reset board
  MasterGameStats.GameStage=0;
  MasterGameStats.HandNumber++;

  //Move the blinds/dealer
  do{
    MasterGameStats.BigBlind=(MasterGameStats.BigBlind + 1) % NPly;
  }while(!MasterGameStats.ActiveInGame[MasterGameStats.BigBlind]);
  do{
    MasterGameStats.LittleBlind=(MasterGameStats.LittleBlind + 1) % NPly;
  }while(!MasterGameStats.ActiveInGame[MasterGameStats.LittleBlind]);
  do{
    MasterGameStats.Dealer=(MasterGameStats.Dealer + 1) % NPly;
  }while(!MasterGameStats.ActiveInGame[MasterGameStats.Dealer]);
  
  //Deal 'em
  DealCards();
  
  //Add the blinds
  HandleBet(BIGBLINDBET,MasterGameStats.BigBlind);
  HandleBet(LILBLINDBET,MasterGameStats.LittleBlind);

  //Pre-flip betting round
  DoBettingRound();

  if(Verbose>2){
    cerr<<"END OF ROUND 0\nGAME BOARD IS:\n";
    MasterGameStats.DumpStats();
  }

  //Begin the 3 card flip
  MasterGameStats.GameStage=1;
  RevealCards();
  DoBettingRound();

  if(Verbose>2){
    cerr<<"END OF ROUND 1\nGAME BOARD IS:\n";
    MasterGameStats.DumpStats();
  }

  //Begin the 1 card flip
  MasterGameStats.GameStage=2;
  RevealCards();
  DoBettingRound();

  if(Verbose>2){
    cerr<<"END OF ROUND 2\nGAME BOARD IS:\n";
    MasterGameStats.DumpStats();
  }

  //Begin the 1 card flip part 2
  MasterGameStats.GameStage=3; 
  RevealCards();
  DoBettingRound();

  if(Verbose>2){
    cerr<<"END OF ROUND 3\nGAME BOARD IS:\n";
    MasterGameStats.DumpStats();
  }

  if(Verbose>2){
    cerr<<"CHECKING FOR WINNER\n";
  }
  CheckForWinner();

  if(Verbose>1){
    cerr<<"FINAL SCORES\n";
    MasterGameStats.DumpStats();
  }

  EndTheHand(); //Cleans up
}

void Game::DoBettingRound(){
  int NPly= MasterGameStats.NoOfPlayers;

  //First person to bet is the person after the dealer
  int index=(1+MasterGameStats.Dealer) % NPly;

  for(int i=0; i<NPly; i++){  

    //Only consider active players
    if(!MasterGameStats.ActiveInHand[index]) continue;

    if(Verbose>2)
      cerr<<"Player "<<index+1<<" is betting now."<<endl;

    //Update that person's local game stats and get their bet
    fPlayers[index].LocalGameStats.UpdateGameStats(&MasterGameStats,index);
    int PlayerBet = fPlayers[index].GetBet();

    HandleBet(PlayerBet,index);

    index=(1+index) % NPly;
  }

  //Update round to subround betting
  MasterGameStats.GameStage+=0.1;  //Maybe 0.01? I don't know if there is a limit

  int AllChecked = 1;  //Flag for an even table

  //See if anyone still is below the current bet
  for(int i=0; i<NPly; i++){
    if(MasterGameStats.ActiveInHand[i] && (MasterGameStats.Bets[i] != MasterGameStats.CurrentBet)){
      AllChecked=0;
    }
  }
  
  int BettingRounds=0; //Limit on the number of betting rounds

  while(!AllChecked && BettingRounds<4*NPly){
    BettingRounds++;
    if(!MasterGameStats.ActiveInHand[index]){ 
      index=(1+index) % NPly;
      continue;
    }
    //Update that person's local game stats
    fPlayers[index].LocalGameStats.UpdateGameStats(&MasterGameStats,index);
    //Get their bet
    int PlayerBet = fPlayers[index].GetBet();
    //Update the bet stats
    HandleBet(PlayerBet,index);
    
    index=(1+index) % NPly;
  
    //Check to see if we can continue
    AllChecked = 1;
    for(int i=0; i<NPly; i++){
      if(MasterGameStats.ActiveInHand[i] && (MasterGameStats.Bets[i] != MasterGameStats.CurrentBet)){
	AllChecked=0;
      }
    }
    //Bump the sub-round counter
    if(!(BettingRounds % NPly)) MasterGameStats.GameStage+=.1;
  }

  //The betting for the current Game Stage is over, clear the bet lists
  for(int i=0; i<MasterGameStats.NoOfPlayers; i++){
    MasterGameStats.Bets[i]=0;
  }

  MasterGameStats.CurrentBet=0;
}

//Handle adding money to the pot and removing it from the player
void Game::AddToPot(int amt, int plyr){
  if(amt > MasterGameStats.PlayerChipCounts[plyr])
    amt = MasterGameStats.PlayerChipCounts[plyr];
  MasterGameStats.PlayerChipCounts[plyr]-=amt;
  MasterGameStats.ThePot+=amt;
  MasterGameStats.Bets[plyr]+=amt;
  if(MasterGameStats.Bets[plyr]>MasterGameStats.CurrentBet)
    MasterGameStats.CurrentBet=MasterGameStats.Bets[plyr];
}

//Flop over cards for the given Game Stage
void Game::RevealCards(){
  if(!(MasterGameStats.GameStage / 1)) return;

  //Flip to begin round 1
  if((MasterGameStats.GameStage / 1)==1){
    //Kill one, reveal 3 cards
    Card *card = fDeck->DrawCard();

    if(Verbose>2){
      cerr<<"Adding to discard pile ";
      card->PrintCard();
    }

    fDiscardPile->AddCardToDiscardPile(card);
    for(int i=0; i<3; i++){
      card = fDeck->DrawCard();

      if(Verbose>2){
	cerr<<"Adding to table ";
	card->PrintCard();
      }

      //Add card to the true table
      fMasterTable.AddCardToTable(card);
      //Add card to the master stats
      Card *CardCopy = new Card(card->GetCard());
      MasterGameStats.TheTable.AddCardToTable(CardCopy);
    }
  }

  //Flip to begin round 2
  if((MasterGameStats.GameStage / 1)==2){
    //Kill one, reveal 1 card
    Card *card = fDeck->DrawCard();

    if(Verbose>2){
      cerr<<"Adding to discard pile ";
      card->PrintCard();
    }

    fDiscardPile->AddCardToDiscardPile(card);
    card = fDeck->DrawCard();
    
    if(Verbose>2){
      cerr<<"Adding to table ";
      card->PrintCard();
    }

    fMasterTable.AddCardToTable(card);
    //Add card to the master stats
    Card *CardCopy = new Card(card->GetCard());
    MasterGameStats.TheTable.AddCardToTable(CardCopy);
  }

  if((MasterGameStats.GameStage / 1)==3){
    //Kill one, reveal 3 cards
    Card *card = fDeck->DrawCard();

    if(Verbose>2){
      cerr<<"Adding to discard pile ";
      card->PrintCard();
    }

    fDiscardPile->AddCardToDiscardPile(card);
    card = fDeck->DrawCard();

    if(Verbose>2){
      cerr<<"Adding to table ";
      card->PrintCard();
    }
    fMasterTable.AddCardToTable(card);
    //Add card to the master stats
    Card *CardCopy = new Card(card->GetCard());
    MasterGameStats.TheTable.AddCardToTable(CardCopy);
  }
}

void Game::DealCards(){
  if(Verbose>0)
    cerr<<"Dealing Cards"<<endl;

  for(int i=0; i<MasterGameStats.NoOfPlayers; i++){
    Hand* hand = new Hand();
    fPlayerHands.push_back(hand);

    //If the player is still in the game
    if(!MasterGameStats.ActiveInGame[i]) continue;

    if(Verbose>2)
      cerr<<"Dealing to player "<<i+1<<endl;

    //Deal a card to the master hand
    Card* card = fDeck->DrawCard();
    fPlayerHands[i]->AddCardToHand(card);
    Card* cardcopy = new Card(card->GetCard());
    fPlayers[i].fPlayerHand.AddCardToHand(cardcopy);

    card = fDeck->DrawCard();
    fPlayerHands[i]->AddCardToHand(card);
    cardcopy = new Card(card->GetCard());
    fPlayers[i].fPlayerHand.AddCardToHand(cardcopy);
    
    fPlayers[i].LocalGameStats.UpdateGameStats(&MasterGameStats,i,&fPlayers[i].fPlayerHand);
  }

}

//Checks that the betting amount of valid and handles the bet
void Game::HandleBet(int bet, int i){
  //Handle Checking
  if(bet==CHECK)
    bet=(MasterGameStats.CurrentBet-MasterGameStats.Bets[i]);

  if(Verbose>2)
    cerr<<"Player "<<i+1<<" bets "<<bet<<endl;

  //If they fold (any negative bet is a fold)
  if(bet==FOLD){
    FoldPlayer(i);
    return; //Don't try to add to pot
  }
    
  //Add dat ca$h
  AddToPot(bet,i);

}

void Game::FoldPlayer(int index){
  //Move cards to discard
  for(int card=0; card<fPlayers[index].fPlayerHand.fHand.size(); card++){
    fDiscardPile->AddCardToDiscardPile(fPlayers[index].fPlayerHand.fHand[card]);
  }
  fPlayers[index].fPlayerHand.ClearHand();

  MasterGameStats.ActiveInHand[index]=0;//Remove player from the hand
  MasterGameStats.Bets[index]=0;//Clear player's bet
}

int Game::CheckForWinner(){
  int CurrentWinner;
  double WinningScore=0, Score=0;
  bool DrawFlag=0;
  vector <int> Draw;
  Draw.clear();

  for(int i=0; i<fPlayers.size(); i++){
    //Only for active players
    if(!MasterGameStats.ActiveInHand[i]) continue;

    //Get the score
    Score=ScoreHand(fPlayerHands[i]);
    
    if(Verbose>1)
      cerr<<"Player "<<i+1<<" has a score of "<<Score<<endl;

    //Handle ties
    if(Score==WinningScore){
      DrawFlag=1;
      if(!Draw.size())
	Draw.push_back(CurrentWinner);
      Draw.push_back(i);
    }

    //Update the max score
    if(Score>WinningScore){
      DrawFlag=0;
      Draw.clear();
      WinningScore=Score;
      CurrentWinner=i;
    }
  }

  //Update that ca$h
  if(DrawFlag){

    if(Verbose>1)
      cerr<<"Its a draw"<<endl;

    int amnt = MasterGameStats.ThePot/Draw.size(); //We can lose total game money this way, use large number of chips
    for(unsigned int i=0; i<Draw.size(); i++){
      MasterGameStats.PlayerChipCounts[Draw[i]]+=amnt;
    }
  }
  else{
    if(Verbose>1)
      cerr<<"Player "<<CurrentWinner+1<<" wins with a score of "<<WinningScore<<endl;
    MasterGameStats.PlayerChipCounts[CurrentWinner]+=MasterGameStats.ThePot;
  }    

  return CurrentWinner;
}

//Combines the passed in hand (two cards) with the table cards and finds the best possible score with that hand
double Game::ScoreHand(Hand* hand){
  Hand AllCards;
  AllCards.AddCardToHand(hand->fHand[0]);
  AllCards.AddCardToHand(hand->fHand[1]);
  for(int i=0; i<5; i++){
    AllCards.AddCardToHand(fMasterTable.fTable[i]);
  }

  double Max=0,Score=0;

  for(int a=0; a<3; a++){
    for(int s=a+1; s<4; s++){
      for(int d=s+1; d<5; d++){
	for(int f=d+1; f<6; f++){
	  for(int g=f+1; g<7; g++){
	    Score=ScoreHand(AllCards.fHand[a]->GetCard(),AllCards.fHand[s]->GetCard(),
			   AllCards.fHand[d]->GetCard(),AllCards.fHand[f]->GetCard(),
			    AllCards.fHand[g]->GetCard());
	    if(Score>Max) Max=Score;
	  }
	}
      }
    }
  }

  return Max;
}	    

//Checks the card values (3 digit number) and calculates the hand score. The score value is a 18 digit number broken up into two digit pairs for the values. ie a high card of 7 has a score of 7 and a pair of tens with ace high  has a score of 1014
  //High Card (Value of Card)
  //Pair (Value of Card)
  //TwoPair (Value of card)
  //Three (Value of card)
  //Straight (High card)
  //Flush (High Card)
  //FullHouse (Val of trip)
  //Four (Value of card)
  //Stright Flush (High Card)
double Game::ScoreHand(int a, int s, int d, int f, int g){
  int A[5]={a,s,d,f,g};

  sort(A,A+5);

  double HandScore=0;

  //CheckForHigh
  int MaxHigh=0;
  for(int i=0; i<5; i++){
    if((A[i]/10) > MaxHigh)
      MaxHigh = (A[i]/10);
  }
  HandScore+=MaxHigh;

  //CheckForMultiples
  int FH=0,Pair=0;

  int Mult[MAXVAL+2]={0};  //Counts multiples of each card type

  for(int i=0; i<5; i++){
    Mult[(A[i]/10)]++; //Card number 2 -> index 0
  }

  for(int i=MAXVAL+1; i>=2; i--){
    if(Mult[i]==4) HandScore+=(i)*10e13;  //4 of a kind
    if(Mult[i]==3){ //Three of a kind
      HandScore+=(i)*10e5;
      FH=i;  //Save for possible full house
    }
    if(Mult[i]==2){  //Found a pair
      if(Pair){
	HandScore+=(Pair)*10e3;  //Two pair
	Pair=i;
	break;
      }
      Pair=i;  //Save for possible full house
    }    
  }
  if(Pair) HandScore+=Pair*10e1; //pair
  


  if(FH && Pair) HandScore+=(FH+2)*10e11;  //Full house
 
  //Check for straight
  int Str=(A[0]/10);
  int FoundStr=1;
  for(int i=1; i<5; i++){
    if((A[i]/10)==(Str+1)){
      Str++;
      if(Str==5 && (A[i]/10)==MAXVAL) break;  //Case of Ace->5
    }
    else{
      FoundStr=0;
      break;
    }
  }
  HandScore+= FoundStr*Str*10e7;

  //Check for flush
  int Suit=(A[0]%10);
  int FoundFl = 1;
  for(int i=1; i<5; i++){
    if((A[i]%10)!=Suit){
      FoundFl=0;
      break;
    }
  }
  HandScore+= FoundFl*(A[4]/10)*10e9;

  //Check for Str-Fl
  if(FoundFl && FoundStr) HandScore += Str*10e15;

  return HandScore;
}

void Game::EndTheHand(){
  ClearCards();
  MasterGameStats.GameStage = 0;
  MasterGameStats.ThePot = 0;
  MasterGameStats.CurrentBet = 0;

  for(int i=0; i<fPlayers.size(); i++){
    if(MasterGameStats.PlayerChipCounts[i]<1){
      MasterGameStats.ActiveInGame[i]=0;
      MasterGameStats.ActiveInHand[i]=0;
    }
    else MasterGameStats.ActiveInHand[i]=1;
  }
    

  return;
}

//Safely clear all of the card locations and memory
void Game::ClearCards(){
  //Clear the table
  fMasterTable.ClearTable();
  MasterGameStats.TheTable.ClearTable();

  //Clear the discard pile
  fDiscardPile->ClearDiscardPile();

  //Clear local player hands
  for(int i=0; i<fPlayers.size(); i++){
    fPlayers[i].fPlayerHand.ClearHand();
    fPlayers[i].LocalGameStats.TheTable.ClearTable();
    fPlayers[i].LocalGameStats.YourHand.ClearHand();
  }

  //Clear master player hands
  for(int i=0; i<fPlayerHands.size(); i++){
    fPlayerHands[i]->ClearHand();
  }

  //Reshuffle deck (clears the deck before shuffling)
  fDeck->ShuffleDeck();

}
  

///////Player stuff//////
int Player::GetBet(){
  int val = fBettingStrategy(LocalGameStats);
  if(val < 1 && val != CHECK && val != FOLD)
    return FOLD;
  return val;
}


///////Game Stats stuff///////
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

void GameStats::Init(){
  YourPosition=0;
  for(int i=0; i<NoOfPlayers; i++){
    PlayerChipCounts[i] = STARTINGCHIPS;
    Bets[i]=0;
    ActiveInHand[i]=1;
    ActiveInGame[i]=1;
  }
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


int Rand(int a, int b){
  if(a==b) return a;
  return rand() % (b-a) + a;
}
