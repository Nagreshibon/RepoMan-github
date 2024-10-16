// Fill out your copyright notice in the Description page of Project Settings.


#include "CardWidget.h"

#include <string>

#include "CardGame.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "RepoManReboot/RepoManRebootCharacter.h"

int UCardWidget::ModNum(int inInt)
{
	Num += inInt;
	if (Num < 0)
	{
		Num = 0;
		return Num;
	}

	if(Num > NumMax)
	{
		Num = NumMax;
	}
	return Num; 
}

UTurnCounterWidget::UTurnCounterWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UTurnCounterWidget::Refresh()
{
	Refresh_BP(); 
}

void UCardGameWidget::Refresh()
{
	RefreshGauges();
	UpdatePlayerHands(); 
}

void UCardGameWidget::RefreshGauges()
{
	if (CurrentGame)
	{
		Affection->SetPercent(ScoreToPercentage(CurrentGame->GetAffection()));
		Intimidation->SetPercent(ScoreToPercentage(CurrentGame->GetIntimidation()));
		Empathy->SetPercent(ScoreToPercentage(CurrentGame->GetEmpathy()));

		UE_LOG(LogTemp, Warning, TEXT("RefreshGauges() - Affection: %d, Intimidation: %d, Empathy: %d"), CurrentGame->GetAffection(), CurrentGame->GetIntimidation(), CurrentGame->GetEmpathy());

		RefreshRoundCounter();
		RefreshTurnCounter(); 
	}
}

float UCardGameWidget::ScoreToPercentage(int i)
{
	if (CurrentGame)
	{
		if (i <= CurrentGame->Limit && i >= -CurrentGame->Limit)
		{
			//limit: 2, d = 0.25, 
			float d = 1.f / (2 * CurrentGame->Limit);
			return d * (i + CurrentGame->Limit);
		}
		if (i < -CurrentGame->Limit) return 0.f;
		if (i > CurrentGame->Limit) return 1.f;
		UE_LOG(LogTemp, Warning, TEXT("ScoreToPercentage() - i out of bounds (%d, limit %d)"), i, CurrentGame->Limit);
	}
	UE_LOG(LogTemp, Warning, TEXT("ScoreToPercentage() - CurrentGame = nullptr"));
	return 0.5f;
}

void UCardGameWidget::SetCurrentGame(UCardGame* inGame)
{
	if (inGame)
	{
		CurrentGame = inGame;
		InitTable(); 
	}
}

void UCardGameWidget::InitTable()
{
	if(CurrentGame)
	{
		if(CurrentGame->Players.Num() >= 2)
		{
			//set opponent

			Portrait->SetBrushFromTexture(CurrentGame->Players[1]->Portrait);
			Name->SetText(FText::FromString(CurrentGame->Players[1]->Name));

			//update hand widgets
			UpdatePlayerHands();

		}
	}
}

void UCardGameWidget::RefreshTurnCounter()
{
	RefreshTurnCounter_BP();
	/*TurnCounter->ClearChildren();

	if (TurnCounter_BP)
	{
		for (int i = 0; i < CurrentGame->TurnLimit; i++)
		{
			auto current = CreateWidget<UTurnCounterWidget>(this, TurnCounter_BP);
			TurnCounter->AddChildToHorizontalBox(current);

			if (i >= CurrentGame->Turn)
			{
				current->bActive = true; 
			}
			else
			{
				current->bActive = false; 
			}
			current->Refresh(); 
		}
	}*/
}

void UCardGameWidget::RefreshRoundCounter()
{
	RoundCounter->SetText(FText::FromString(FString::FromInt(CurrentGame->Round)));
}

void UCardGameWidget::UpdatePlayerHands()
{
	if (!CurrentGame)
	{
		return;
	}

	PlayerHand->ClearChildren();
	EnemyHand->ClearChildren();
	PlayerActives->ClearChildren();
	EnemyActives->ClearChildren();
	NeutralActives->ClearChildren(); 


	// Only 2 players
	for (int i = 0; i < 2; i++)
	{
		for (const auto& CardInHand : CurrentGame->Players[i]->CurrentHand)
		{
			if (CurrentGame->ActiveCards.Contains(CardInHand))
			{
				continue;
			}

			if (CardInHand->CardWidget_BP)
			{
				auto current = CreateWidget<UCardWidget>(this, CardInHand->CardWidget_BP);
				current->CurrentWidget = this;
				current->CurrentCard = CardInHand;
				current->Description->SetText(CardInHand->Description);

				if (i == 0)
				{
					current->Art->SetBrushFromTexture(CardInHand->FrontArt);
					PlayerHand->AddChildToHorizontalBox(current);
				}
				else
				{
					current->Art->SetBrushFromTexture(CardInHand->BackArt);
					EnemyHand->AddChildToHorizontalBox(current);
				}
			}
		}
	}

	for (const auto& c : CurrentGame->ActiveCards)
	{
		if (c->CardWidget_BP)
		{
			auto current = CreateWidget<UCardWidget>(this, c->CardWidget_BP);
			current->Art->SetBrushFromTexture(c->FrontArt);
			current->CurrentWidget = this;
			current->CurrentCard = c;
			current->Description->SetText(c->Description);

			if(!c->Player)
			{
				NeutralActives->AddChildToHorizontalBox(current);
			}
			else if(c->Player == CurrentGame->Players[0])
			{
				PlayerActives->AddChildToHorizontalBox(current); 
			}
			else if(c->Player == CurrentGame->Players[1])
			{
				EnemyActives->AddChildToHorizontalBox(current); 
			}
		}
	}

	PlayerHand->ForceLayoutPrepass();
	EnemyHand->ForceLayoutPrepass();
	PlayerActives->ForceLayoutPrepass();
	EnemyActives->ForceLayoutPrepass();
	NeutralActives->ForceLayoutPrepass();
	
}

void UDeckEditor::Refresh()
{
	Refresh_BP();
	ForceLayoutPrepass(); 
}

bool UDeckEditor::CommitDeck(TMap<TSubclassOf<UCard>, int>& inMap)
{
	if (inMap.Num() <= DeckLimit)
	{
		if (Player)
		{
			Player->DeckBlueprint.Empty();

			for (const auto& c : inMap)
			{
				for (int i = 0; i < c.Value; i++)
				{
					Player->DeckBlueprint.Add(c.Key);
				}
			}
			return true;
		}
		
	}
	return false;
}

bool UDeckEditor::CommitDeck()
{
	auto map = Grid->GetAllChildren();
	if(CurrentNum <= DeckLimit && CurrentNum > 0)
	{
		ARepoManRebootCharacter* pawn = nullptr;
		pawn = Cast<ARepoManRebootCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
		if(pawn)
		{
			pawn->GetLastDeck().Empty();
		}

		if (Player)
		{
			Player->DeckBlueprint.Empty();
			for (const auto& w : map)
			{
				if (auto cast = Cast<UCardWidget>(w))
				{
					if (pawn)
					{
						pawn->GetLastDeck().Add(cast->CardClass, cast->GetNum());
					}

					for (int i = 0; i < cast->GetNum(); i++)
					{
						Player->DeckBlueprint.Add(cast->CardClass);
					}
				}
			}
			return true;
		}
		
	}
	return false;
}

void UDeckEditor::PopulateFromDeck(TMap<TSubclassOf<UCard>, int> inMap)
{
	Grid->ClearChildren(); 
	int k = 0;
	for(const auto& c: inMap)
	{
		UCardWidget* temp = nullptr;

		if (CardWidget_BP)
		{
			temp = CreateWidget<UCardWidget>(this, CardWidget_BP);
		}

		if (temp)
		{
			temp->Art->SetBrushFromTexture(c.Key.GetDefaultObject()->FrontArt);
			temp->CardClass = c.Key; 
			temp->Description->SetText(c.Key.GetDefaultObject()->Description);
			temp->NumBlock->SetText(FText::FromString("0"));
			temp->CurrentEditor = this; 

			Grid->AddChildToUniformGrid(temp, k / Columns, k % Columns);
			temp->NumMax = c.Value;
			k++;
		}
	}
}

void UDeckEditor::LoadDeck(TMap<TSubclassOf<UCard>, int> inMap)
{
	CurrentNum = 0;
	for(const auto& w : Grid->GetAllChildren())
	{
		if(const auto cast = Cast<UCardWidget>(w))
		{
			cast->GetNum() = 0;

			if(const auto i = inMap.Find(cast->CardClass))
			{
				CurrentNum += *i; 
				cast->GetNum() = *i;
				cast->NumBlock->SetText(FText::FromString(FString::FromInt(*i))); 
			}
		}
	}
	Refresh(); 
}
