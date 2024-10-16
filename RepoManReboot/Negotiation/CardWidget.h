// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardWidget.generated.h"

class UDeckEditor;
class UUniformGridPanel;
class UCardPlayer;
class UTextBlock;
class UCardGameWidget;
class UImage;
class UCanvasPanel;
/**
 * 
 */
class UProgressBar; 
class URichTextBlock;
class UCard; 

UCLASS()
class REPOMANREBOOT_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	URichTextBlock* Description;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UImage* Art;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UTextBlock* NameBlock;

	UPROPERTY(BlueprintReadWrite)
		bool bBackFront; 

	UPROPERTY(BlueprintReadWrite)
		UCardGameWidget* CurrentWidget;

	UPROPERTY(BlueprintReadWrite)
		UDeckEditor* CurrentEditor; 

	UPROPERTY(BlueprintReadWrite)
		UCard* CurrentCard;

	UFUNCTION(BlueprintCallable)
		int& GetNum() { return Num; }

	UFUNCTION(BlueprintCallable)
		int ModNum(int inInt);

private:
		int Num;

public:
	UPROPERTY(BlueprintReadWrite)
		int NumMax;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UTextBlock* NumBlock;

	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<UCard> CardClass;
	
};

UCLASS()
class REPOMANREBOOT_API UTurnCounterWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UTurnCounterWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly)
		bool bActive;

	UFUNCTION(BlueprintCallable)
		void Refresh();

	UFUNCTION(BlueprintImplementableEvent)
		void Refresh_BP(); 
};

class UCardGame;
class UHorizontalBox; 

UCLASS()
class REPOMANREBOOT_API UCardGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void Refresh(); 

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UImage* Portrait;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UTextBlock* Name; 

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UProgressBar* Affection;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UProgressBar* Intimidation;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UProgressBar* Empathy;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* TurnCounter;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UTextBlock* RoundCounter;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* PlayerHand;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* EnemyHand;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* PlayerActives;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* EnemyActives;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UHorizontalBox* NeutralActives;

	UFUNCTION(BlueprintCallable)
		void RefreshGauges();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float ScoreToPercentage(int i);

	UFUNCTION(BlueprintCallable)
	void SetCurrentGame(UCardGame* inGame);

	UFUNCTION(BlueprintImplementableEvent)
		void EndGame_BP(int inScore);

	UFUNCTION(BlueprintCallable)
		void InitTable();

	UFUNCTION(BlueprintCallable)
		void RefreshTurnCounter();

		UFUNCTION(BlueprintImplementableEvent)
		void RefreshTurnCounter_BP();

	UFUNCTION(BlueprintCallable)
		void RefreshRoundCounter(); 

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerHands();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<UTurnCounterWidget> TurnCounter_BP;


	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UCanvasPanel* MainCanvas;

	UPROPERTY(BlueprintReadWrite)
		UCardGame* CurrentGame; 


};

UCLASS()
class REPOMANREBOOT_API UDeckEditor : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
		UCardPlayer* Player;

	UFUNCTION(BlueprintCallable)
		virtual void Refresh();

	UFUNCTION(BlueprintImplementableEvent)
		void Refresh_BP();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Deck")
		int DeckLimit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Deck")
		int Columns = 3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Deck")
		TSubclassOf<UCardWidget> CardWidget_BP;

		bool CommitDeck(TMap<TSubclassOf<UCard>, int>& inMap);

	UFUNCTION(BlueprintCallable)
		bool CommitDeck();

	UFUNCTION(BlueprintCallable)
		void PopulateFromDeck(TMap<TSubclassOf<UCard>, int> inMap);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
		UUniformGridPanel* Grid;

	UFUNCTION(BlueprintCallable)
		void LoadDeck(TMap<TSubclassOf<UCard>, int> inMap);

	UPROPERTY(BlueprintReadWrite)
		int CurrentNum; 

};