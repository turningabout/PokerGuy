#include "Poker.h"
#include <iomanip>


namespace Alan{
  bool Debug=0;
  double HandValue(int A[], int N);
  double CurrentBestScore(GameStats Stats);
  void PossibleScores(GameStats Stats);
  void OpponentsPossibleScores(Table table, int MyCard1, int MyCard2);
}



int Alan::AlansBettingStrategy(GameStats Stats){
  int MyIndex=Stats.YourPosition;

  if(Verbose>-1){
    cerr<<"What I see: "<<endl;
    Stats.DumpStats();

    if(Stats.TheTable.fTable.size()){
      cerr<<"I see "<<Stats.TheTable.fTable.size()<<" cards on the table:"<<endl;
      for(int i=0; i<Stats.TheTable.fTable.size(); i++){
	Stats.TheTable.fTable[i]->PrintCard();
      }
    }
    if(Stats.YourHand.fHand.size()){
      cerr<<"I have "<<Stats.YourHand.fHand.size()<<" cards:"<<endl;
      for(int i=0; i<Stats.YourHand.fHand.size(); i++){
	Stats.YourHand.fHand[i]->PrintCard();
      }
    }
  }

  vector <int> Table;
  vector <int> Hand;

  for(int i=0; i<Stats.YourHand.fHand.size(); i++){
    Hand.push_back(Stats.YourHand.fHand[i]->GetCard());
  }

  for(int i=0; i<Stats.TheTable.GetCardsOnTable(); i++){
    Table.push_back(Stats.TheTable.fTable[i]->GetCard());
  }

  ///For debugging
  if(Table.size()<3 && Stats.GameStage>=1)
    cerr<<"There aren't enough cards on the table!\nGame stage is: "
	<<Stats.GameStage<<"\nCards on table: "<<Table.size()<<endl;
  if(Hand.size()<2)
    cerr<<"I have no hand!\n"<<endl;


  double CurrentScore = CurrentBestScore(Stats);
  cerr<<"Current best score is: "<<CurrentScore<<endl;
  

  if((Stats.GameStage-1 < 0)) return CHECK;
  return CHECK;
} 

int Alan::BotBettingStrategy(GameStats stats){
  return CHECK;
  if(stats.GameStage==0) return 7;
  if(stats.GameStage-1< 0) return CHECK;
  return CHECK;
}

int Alan::Bot2BettingStrategy(GameStats stats){
  return CHECK;
  if(stats.GameStage==0) return 20;
  if(stats.GameStage-1 <0) return CHECK;
  if(stats.GameStage==1) return 13;
  //if(stats.GameStage==3) return FOLD;
  return CHECK;
}


double Alan::CurrentBestScore(GameStats stats){
  vector <int> Table;
  vector <int> Hand;

  for(int i=0; i<stats.YourHand.fHand.size(); i++){ //Get the cards from your hand
    
    Hand.push_back(stats.YourHand.fHand[i]->GetCard());
  }

  for(int i=0; i<stats.TheTable.GetCardsOnTable(); i++){ //Get the cards on table
    Table.push_back(stats.TheTable.fTable[i]->GetCard());
  }

  const int N=Table.size()+Hand.size();

  int CombinedArray[N];

  for(int i=0; i<Hand.size(); i++){
    CombinedArray[i]=Hand[i];
  }

  for(int i=0; i<Table.size(); i++){
    CombinedArray[i+Hand.size()]=Table[i];
  }

  if(N < 6){ //Cases when I only have 2 or 5 cards
    return HandValue(CombinedArray,N);
  }
  else{//Cases when I have more than 5 cards
    int Cards[5];
    double MaxScore,Score=0;
    for(int a=0; a<N; a++){
      for(int s=a+1; s<N; s++){
	for(int d=s+1; d<N; d++){
	  for(int f=d+1; f<N; f++){
	    for(int g=f+1; g<N; g++){
	      Cards[0]=CombinedArray[a];
	      Cards[1]=CombinedArray[s];
	      Cards[2]=CombinedArray[d];
	      Cards[3]=CombinedArray[f];
	      Cards[4]=CombinedArray[g];
	      cerr<<"Trying: ";
	      for(int i=0; i<5; i++)
		cerr<<Cards[i]<<' ';
	      Score=HandValue(Cards,5);
	      cerr<<"\t which gives: "<<setprecision(14)<<Score<<endl;
	      if(Score>MaxScore) MaxScore=Score;
	    }
	  }
	}
      }
    }
    return MaxScore;
  }

  return 0;
}


double Alan::HandValue(int A[], int N){

  sort(A,A+N);

  double HandScore=0;

  //CheckForHigh
  int MaxHigh=0;
  for(int i=0; i<N; i++){
    if((A[i]/10) > MaxHigh)
      MaxHigh = (A[i]/10);
  }
  HandScore+=MaxHigh;

  //CheckForMultiples
  int FH=0,Pair=0,Double=0,Trip=0;

  int Mult[MAXVAL+2]={0}; //

  for(int i=0; i<N; i++){
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

  if(FH && Pair) HandScore+=(FH+2)*10e11;
 
  //Check for straight
  int Str=(A[0]/10);
  int FoundStr=1;
  if(N==5){
    for(int i=1; i<5; i++){
      if((A[i]/10)==(Str+1)){
	Str++;
	if(Str==5 && (A[i]/10)==13) break;  //Case of Ace->5
      }
      else{
	FoundStr=0;
	break;
      }
    }
  }
  else FoundStr=0;
  HandScore+= FoundStr*Str*10e7;

  //Check for flush
  int Suit=(A[0]%10);
  int FoundFl = 1;
  if(N==5){
    for(int i=1; i<5; i++){
      if((A[i]%10)!=Suit){
	FoundFl=0;
	break;
      }
    }
  }
  else FoundFl=0;
  HandScore+= FoundFl*(A[4]/10)*10e9;

  //Check for Str-Fl
  if(FoundFl && FoundStr) HandScore += Str*10e15;
  
  return HandScore;
}

