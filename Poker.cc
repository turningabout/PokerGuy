
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
  