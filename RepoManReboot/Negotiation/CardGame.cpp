// Fill out your copyright notice in the Description page of Project Settings.


#include "CardGame.h"
#include "CardWidget.h"

bool UCard::PreCardEffect()
{
	return PreCardEffect_BP(true); 
}

bool UCard::PreCardEffect_BP_Implementation(bool inReturn)
{
	return inReturn; 
}

void UCard::CardEffect()
{
	ApplyMods(ProcessActives());
	if(OnCardPlayed.IsBound())
	{
		OnCardPlayed.Broadcast(this);
	}
	CardEffect_BP(); 
}

FCardMods UCard::ProcessActives(bool bIsSimulated)
{
	auto effect_temp = FCardMods();
	if (bIsSimulated) { effect_temp = CardEffect_BP(true); }

	if (!effect_temp.bOverrideActives)
	{
		auto basemods = FCardMods(Affection, Intimidation, Empathy);
		TempCardMods_Internal = basemods;

		if (!CurrentGame || !this) return basemods;

		for (const auto& c : CurrentGame->ActiveCards)
		{
			auto temp = c->OnCardPlayedEffect(this);
			basemods += temp;
			TempCardMods_Internal = temp;
		}

		if(bIsSimulated) TempCardMods_Internal += CardEffect_BP(true);

		return TempCardMods_Internal;
	}
	else return effect_temp; 
}


void UCard::PostCardEffect()
{
	PostCardEffect_BP();
}

FCardMods UCard::OnCardPlayedEffect(UCard* inCard)
{
	return OnCardPlayedEffect_BP(inCard);
}

void UCard::OnCardAddedToInventoryEffect()
{
	OnCardAddedToInventoryEffect_BP();

	if (OnCardAddedToInventory.IsBound())
	{
		OnCardAddedToInventory.Broadcast(this);
	}

}

void UCard::NextCardEffect(UCard* inCard)
{
	NextCardEffect_BP(inCard);
	if(CurrentGame)
	{
		CurrentGame->RefreshWidget(); 
	}
}

void UCard::TickCard()
{
	if (bActive)
	{
		if (Countdown > 0)
		{
			TickingCardEffect();
			Countdown--;
		}
		else
		{
			Die();
		}
	}

}

void UCard::TickingCardEffect()
{
	TickingCardEffect_BP();
}

void UCard::ApplyMods(int inAffection, int inIntimidation, int inEmpathy)
{
	if(CurrentGame)
	{
		CurrentGame->ModAffection(inAffection);
		CurrentGame->ModIntimidation(inIntimidation); 
		CurrentGame->ModEmpathy(inEmpathy);
	}
}

void UCard::ApplyMods(FCardMods inMods)
{
	if (CurrentGame)
	{
		CurrentGame->ModAffection(inMods.Affection);
		CurrentGame->ModIntimidation(inMods.Intimidation);
		CurrentGame->ModEmpathy(inMods.Empathy);
	}
}

void UCard::Die()
{
	PostCardEffect(); 

	Countdown = 0;

	if(CurrentGame)
	{
		CurrentGame->ActiveCards.Remove(this);
		if (Player)
		{
			Player->CurrentHand.Remove(this);
			Player->ActiveCards.Remove(this);
			Player->CardHistory.Add(this);
		}

		if(CurrentGame->CurrentWidget)
		{
			CurrentGame->CurrentWidget->UpdatePlayerHands(); 
		}
	}
}

void UCard::RemoveFromHand()
{
	if(CurrentGame && Player)
	{
		Player->CurrentHand.Remove(this);
	}
}

bool UCard::OnEquipped()
{
	bEquipped = true;

	return true;
}

bool UCard::OnUnequipped()
{
	bEquipped = false;

	return true;
}

void UCard::Tick(float DeltaTime)
{
	Tick_BP(DeltaTime);
}

bool UCard::OnPreCast()
{
	return true;
}

bool UCard::OnPreCast_BP_Implementation()
{
	return true;
}

void UCard::OnCast()
{
}

UCardPlayer::UCardPlayer()
{
	HandLimit = 4;
	bPassing = false; 
}

void UCardPlayer::OnAIPlayerActivated()
{
	OnAIPlayerActivated_BP();
}

void UCardPlayer::PlayCard(UCard* inCard)
{
	if (!CurrentGame || CurrentGame->GetActivePlayer() != this) return;

	if (inCard->ManaCost >= Mana)
	{
		if(OnCardPlayed.IsBound())
		{
			OnCardPlayed.Broadcast(inCard, this); 
		}

		if (inCard->PreCardEffect())
		{
			//CurrentPrevCard = nullptr;
			//CurrentGame->ApplyMods(inCard);

			if (CurrentPrevCard)
			{
				CurrentPrevCard->NextCardEffect(inCard);
				CurrentPrevCard = nullptr;
			}
			else
			{
				inCard->CardEffect();


				if (inCard->bTicker)
				{
					ActiveCards.Add(inCard);
					CurrentGame->ActiveCards.Add(inCard);
					inCard->bActive = true;
					inCard->Countdown = inCard->TickTimer;
					//inCard->OnCardPlayed.AddDynamic(inCard, &UCard::OnCardPlayedEffect);
				}

				Mana += -inCard->ManaCost;
			}

			if (inCard->bPlayAnother)
			{
				PlayAnother(inCard);
				if(CurrentHand.Num() == 1)
				{
					CurrentPrevCard = nullptr;
					CurrentGame->NextRound();
				}
			}
			else
			{
				CurrentPrevCard = nullptr;
				CurrentGame->NextRound();
			}
		}

		inCard->Player->Graveyard.Add(inCard);
		CardHistory.EmplaceAt(0, inCard);
		inCard->RemoveFromHand();
	}
}

void UCardPlayer::PlayAnother(UCard* prevCard)
{
	CurrentPrevCard = prevCard;

	if(bAIPlayer)
	{
		OnAIPlayerActivated(); 
	}
}

void UCardPlayer::PlayRandomCard()
{
	auto k = FMath::RandRange(0, CurrentHand.Num() - 1);

	PlayCard(CurrentHand[k]);
}

UCard* UCardPlayer::GetLastPlayedCard()
{
	if(CardHistory.Num()>0)
	{
		return CardHistory[0]; 
	}

	return nullptr; 
}

void UCardPlayer::PopulateDeckFromBP()
{
	CurrentDeck.Empty(); 
	if (CurrentGame)
	{
		for (auto& card_bp : DeckBlueprint)
		{
			if (card_bp)
			{
				if (auto card = NewObject<UCard>(this, card_bp))
				{
					card->CurrentGame = CurrentGame;
					card->Player = this;
					CurrentDeck.Add(card);
				}
			}
			
			/*if(card)
			{
				UE_LOG(LogTemp, Warning, TEXT("PopulateDeckFromBP() - created card %s"), *card->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PopulateDeckFromBP() - couldn't create card"));
			}*/
		}
	}
}

void UCardPlayer::InitPlayer(UCardGame* InCardGame) noexcept
{
	CurrentGame = InCardGame;
	CurrentHand.Empty(); 
	PopulateDeckFromBP();
	DrawHand();
}

void UCardPlayer::DrawHand()
{
	CurrentHand.Empty(); 
	const int ToDraw = HandLimit - CurrentHand.Num();

	for (int i = 0; i < ToDraw; i++)
	{
		if (auto c = DrawCard())
		{
			CurrentHand.Add(c);
		}
		else
		{
			break;
		}
	}
}

UCard* UCardPlayer::DrawCard()
{
	if (GetCurrentDeckSize() > 0)
	{
		int rand = 0;
		if(!bDeterministic) rand = FMath::RandRange(0, GetCurrentDeckSize() - 1);

		

		auto drawn = CurrentDeck[rand];
		//CurrentHand.Add(drawn);
		//UE_LOG(LogTemp, Warning, TEXT("DrawCard() - rand: %d, card: %s"), rand, *drawn->GetClass()->GetDisplayNameText().ToString());
		CurrentDeck.RemoveAt(rand);

		return drawn;
	}
	return nullptr;
}

bool UCardPlayer::DrawRandomCard()
{
	if (auto c = DrawCard())
	{
		CurrentHand.Add(c);
		if(CurrentGame->CurrentWidget)
		{
			CurrentGame->CurrentWidget->UpdatePlayerHands(); 
		}
		return true;
	}
	return false;
}

UCardGame::UCardGame()
{
	// xd
	Affection = 0;
	Intimidation = 0;
	Empathy = 0;

	Turn = 1;
	TurnLimit = 7; 
	Limit = 6;
	CardsPerTurnLimit = 1; 
}

void UCardGame::NextTurn()
{
	Turn++;
	Round = 0;
	ActivatePlayer(0);
	

	UE_LOG(LogTemp, Warning, TEXT("Affection %d Intimidation %d Empathy %d Limit %d"), Affection, Intimidation, Empathy, Limit);
	if(Empathy == Limit || Empathy == -Limit ||
		Affection == Limit || Affection == -Limit ||
		Intimidation == Limit || Intimidation == -Limit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending game, stat limit reached"));
		LoseGame();
		return; 
	}

	if (Turn >= TurnLimit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending game, turn limit exceeded"));
		EndGame();
		return;
	}

	for(auto& c : ActiveCards)
	{
		c->TickCard(); 
	}

	StartTurn();

}

void UCardGame::StartTurn()
{
	for (auto& p : Players)
	{
		auto c = p->DrawCard();
		if (c)
		{
			p->CurrentHand.Add(c);
			if(p->CurrentHand.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Ending game, player %d has 0 cards on hand"), GetActivePlayerIndex());
				EndGame(); 
			}
		}
	}

	NewTurn_BP();

	if (CurrentWidget)
	{
		CurrentWidget->Refresh();
	}
}

void UCardGame::RefreshWidget()
{
	if (CurrentWidget)
	{
		CurrentWidget->Refresh();
	}
}

void UCardGame::EndGame()
{
	RefreshWidget();
	ActivePlayer = nullptr; 
	auto score = Affection + Intimidation + Empathy;

	UE_LOG(LogTemp, Warning, TEXT("EndGame() - Affection %d Intimidation %d Empathy %d score %d"), Affection, Intimidation, Empathy, score);

	EndGame_BP(score);

	if(CurrentWidget)
	{
		CurrentWidget->EndGame_BP(score);
	}

	if(OnEndGame.IsBound())
	{
		OnEndGame.Broadcast(score);
	}
}

void UCardGame::LoseGame()
{
	RefreshWidget();
	ActivePlayer = nullptr;

	LoseGame_BP();

	if (CurrentWidget)
	{
		CurrentWidget->EndGame_BP(-999);
	}

	if (OnEndGame.IsBound())
	{
		OnEndGame.Broadcast(-999);
	}
}

void UCardGame::NextRound()
{
	Round++;

	UE_LOG(LogTemp, Warning, TEXT("Round counter: %d"), Round);
	RefreshWidget();

	if (Round >= Players.Num() * CardsPerTurnLimit)
	{
		NextTurn();
		return; 
	}

	ActivatePlayer(GetNextPlayerIndex());

	if (ActivePlayer->CurrentHand.Num() == 0 || ActivePlayer->bPassing) NextRound(); 
}

void UCardGame::PlayCard(UCard* inCard)
{
	if(inCard && inCard->Player)
	{
		inCard->Player->PlayCard(inCard); 
	}
}

void UCardGame::ApplyMods(UCard* InCard) noexcept
{
	if (!InCard) return;

	ModAffection(InCard->Affection);
	ModIntimidation(InCard->Intimidation);
	ModEmpathy(InCard->Empathy);
}


void UCardGame::InitGame(UCardPlayer* player1, UCardPlayer* player2, TArray<UCard*> specialCards)
{
	if (!player1 || !player2) return; 
	Players.Add(player1);
	Players.Add(player2);
	ActiveCards.Append(specialCards); 

	for (const auto& c : specialCards)
	{
		c->CurrentGame = this;
	}

	for (auto& c : ActiveCards)
	{
		ApplyMods(c); 
	}

	player1->InitPlayer(this);
	player2->InitPlayer(this);

	ActivatePlayer(0);
}

bool UCardGame::ActivatePlayer(int i)
{
	check(i < Players.Num());

	UE_LOG(LogTemp, Warning, TEXT("Activating player %d with CardLimit %d"), i, CardsPerTurnLimit);

	PrevPlayer = ActivePlayer; 

	if(const auto player = Players[i])
	{
		ActivePlayer = player;
		ActivePlayerIndex = i;
		player->Mana = player->MaxMana; 

		ActivatePlayer_BP(i);

		if(player->bAIPlayer)
		{
			player->OnAIPlayerActivated(); 
		}

		if (OnNextRound.IsBound())
		{
			OnNextRound.Broadcast(ActivePlayer);
		}

		return true;
	}
	return false; 
}

int UCardGame::GetNextPlayerIndex() const noexcept
{
	check(ActivePlayerIndex < Players.Num());

	int i = ActivePlayerIndex;

	return ++i == Players.Num() ? 0 : i;
}

void UCardGame::PlayNeutralCard(TSubclassOf<UCard> inCard)
{
	if (inCard)
	{
		auto current = NewObject<UCard>(inCard); 

		if (current->bTicker)
		{
			ActiveCards.Add(current);
			current->bActive = true;
			current->Countdown = current->TickTimer;
			//current->OnCardPlayed.AddDynamic(current, &UCard::OnCardPlayedEffect);
		}
	}
}

int UCardGame::ModAffection(int m) noexcept
{
	Affection += m;
	CheckLimits(Affection);

	return Affection;
}

int UCardGame::ModIntimidation(int m) noexcept
{
	Intimidation += m;
	CheckLimits(Intimidation);

	return Intimidation;
}

int UCardGame::ModEmpathy(int m) noexcept
{
	Empathy += m;
	CheckLimits(Empathy);

	return Empathy; 
}

void UCardGame::SetAffection(int m) noexcept
{
	Affection = m;
	CheckLimits(Affection);
}

void UCardGame::SetIntimidation(int m) noexcept
{
	Intimidation = m;
	CheckLimits(Intimidation);
}

void UCardGame::SetEmpathy(int m) noexcept
{
	Empathy = m;
	CheckLimits(Empathy);
}



