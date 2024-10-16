// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item.h"
#include "UObject/NoExportTypes.h"
#include "../Dialogue/DialogueBody.h"
#include "CardGame.generated.h"

/**
 * 
 */

class UCardGame;
class UCardPlayer;
class UCard;
class UCardWidget;
class ARepoNPC;

USTRUCT(BlueprintType)
struct FCardMods {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	int Affection;

	UPROPERTY(BlueprintReadWrite)
	int Intimidation;

	UPROPERTY(BlueprintReadWrite)
	int Empathy;

	UPROPERTY(BlueprintReadWrite)
	bool bOverrideActives;

	FCardMods operator+(const FCardMods& other)
	{
		FCardMods ret; 
		ret.Affection = Affection + other.Affection;
		ret.Intimidation = Intimidation + other.Intimidation;
		ret.Empathy = Empathy + other.Empathy;

		return ret;
	}

	FCardMods()
	{
		Affection = 0;
		Intimidation = 0;
		Empathy = 0; 
		bOverrideActives = false;
	}

	FCardMods(int inAff, int inIntim, int inEmp) : Affection(inAff), Intimidation(inIntim), Empathy(inEmp)
	{
		/*Affection = inAff;
		Intimidation = inIntim;
		Empathy = inEmp; */
	}

	FCardMods& operator+=(const FCardMods& b)
	{
		Affection += b.Affection;
		Intimidation += b.Intimidation;
		Empathy += b.Empathy;

		return *this; 
	}
};

USTRUCT(BlueprintType)
struct FCardDescription 
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (MultiLine = true))
	FText DescriptionText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FDialogueBody> DialogueLines; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UObject* AdditionalData = nullptr;
};

UENUM(BlueprintType)
enum class EAbilityModificationOfPreviousCards : uint8
{
	RemoveEffect = 0
};

USTRUCT(BlueprintType)
struct FCardAbilityPreviousCards
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int AmountOfCardsToAffect = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAbilityModificationOfPreviousCards AbilityModificationOfPreviousCards;
};

UENUM(BlueprintType)
enum class EAbilityModificationOfFutureCards : uint8
{
	Add = 0,
	Divide,
	Multiply,
	Subtract
};

USTRUCT(BlueprintType)
struct FCardAbilityModifyNextCardPlayed
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAbilityModificationOfFutureCards AbilityModificationOfFutureCards;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int AffectionChange = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int IntimidationChange = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int EmpathyChange = 0;
};

USTRUCT(BlueprintType)
struct FCardAbilityModifyFutureTurns
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCardAbilityModifyNextCardPlayed CardAbilityModifyNextCardPlayed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int AmountOfTurnToAffect = 0;
};

USTRUCT(BlueprintType)
struct FCardAbilityInCardGame
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCardAbilityPreviousCards PreviousCardsAbility;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCardAbilityModifyFutureTurns CardAbilityModifyFutureTurns;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicCardMulticastDelegate, UCard*, outCard);

UCLASS(Blueprintable, BlueprintType)
class REPOMANREBOOT_API UCard : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	FLinearColor NameColor; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	UTexture2D* FrontArt;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	UTexture2D* BackArt;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	UTexture2D* Border;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	int ManaCost;

	UPROPERTY(BlueprintReadWrite, Category = "Card")
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly)
	UCardGame* CurrentGame;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCardAbilityInCardGame CardAbilityInCardGame;

	UPROPERTY(BlueprintReadOnly)
	UCardPlayer* Player;

	UPROPERTY(BlueprintAssignable)
	FDynamicCardMulticastDelegate OnCardPlayed;

	UFUNCTION()
	virtual bool PreCardEffect();

	UFUNCTION(BlueprintNativeEvent)
	bool PreCardEffect_BP(bool inReturn);

	UFUNCTION()
	virtual void CardEffect();

	UFUNCTION()
	virtual void PostCardEffect();

	UFUNCTION()
	virtual FCardMods OnCardPlayedEffect(UCard* inCard);

	UFUNCTION(BlueprintCallable)
	FCardMods ProcessActives(bool bIsSimulated = false); 

	UPROPERTY(BlueprintReadOnly)
	FCardMods TempCardMods_Internal;

	UFUNCTION(BlueprintImplementableEvent)
		FCardMods OnCardPlayedEffect_BP(UCard* inCard);

	UFUNCTION(BlueprintImplementableEvent)
		void PostCardEffect_BP();

	UFUNCTION(BlueprintImplementableEvent)
		FCardMods CardEffect_BP(bool bSimulated = false);

		UPROPERTY(BlueprintAssignable)
		FDynamicCardMulticastDelegate OnCardAddedToInventory;

		virtual void OnCardAddedToInventoryEffect();

		UFUNCTION(BlueprintImplementableEvent)
		void OnCardAddedToInventoryEffect_BP();

	UFUNCTION()
		virtual void NextCardEffect(UCard* inCard);

	UFUNCTION(BlueprintImplementableEvent)
		void NextCardEffect_BP(UCard* inCard);

	void TickCard();

	UFUNCTION()
		virtual void TickingCardEffect();

	UFUNCTION(BlueprintImplementableEvent)
		void TickingCardEffect_BP();

	UFUNCTION(BlueprintCallable)
		void ApplyMods(int inAffection, int inIntimidation, int inEmpathy);

		void ApplyMods(FCardMods inMods);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
		int Affection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
		int Empathy;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
		int Intimidation;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Card")
		bool bTicker;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Card")
		bool bPlayAnother;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
		int TickTimer; 

	UPROPERTY(BlueprintReadOnly, Category = "Card")
		int Countdown; 

	UFUNCTION(BlueprintCallable)
	void Die();
	void RemoveFromHand();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	TSubclassOf<UCardWidget> CardWidget_BP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card")
	TSubclassOf<UCardWidget> CardWidget_BP_ActivesAndrzejowe;


	//INGAME

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
		bool bEquipped;

	UFUNCTION(BlueprintCallable)
		virtual bool OnEquipped();

	UFUNCTION(BlueprintImplementableEvent)
		void OnEquipped_BP();

	UFUNCTION(BlueprintCallable)
		virtual bool OnUnequipped();

	UFUNCTION(BlueprintCallable)
	virtual void Tick(float DeltaTime); 

	UFUNCTION(BlueprintImplementableEvent)
		void Tick_BP(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent)
		void OnUnequipped_BP();

	UFUNCTION(BlueprintCallable)
		virtual bool OnPreCast(); 

	UFUNCTION(BlueprintNativeEvent)
		bool OnPreCast_BP();

	UFUNCTION()
		virtual bool OnPreCast_BP_Implementation();

	UFUNCTION(BlueprintCallable)
		virtual void OnCast();

		UFUNCTION(BlueprintImplementableEvent)
		void OnCast_BP();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	TMap<FString, FCardDescription> CardTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability", meta = (MultiLine=true))
	FText AbilityDescription; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UItemRarity Rarity;
};

class UCardGame;
class UCardGameWidget; 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicPlayerMulticastDelegate, UCardPlayer*, outPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCardPlayedDelegate, UCard*, outCard, UCardPlayer*, outPlayer);

UCLASS(Blueprintable, BlueprintType)
class REPOMANREBOOT_API UCardPlayer : public UObject
{
	GENERATED_BODY()

		UCardPlayer();

public:
	UPROPERTY(BlueprintReadWrite)
		bool bPassing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
		bool bAIPlayer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
		bool bDeterministic;

	UFUNCTION()
	virtual void OnAIPlayerActivated();

	UFUNCTION(BlueprintImplementableEvent)
		void OnAIPlayerActivated_BP();

	UPROPERTY(BlueprintReadOnly)
		int HandLimit;

	UPROPERTY(BlueprintReadOnly)
	UCardGame* CurrentGame;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
		FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
		UTexture2D* Portrait; 	

		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
		USoundBase* ThemeMusic; 

	UFUNCTION(BlueprintCallable)
		void PlayCard(UCard* inCard);

	UFUNCTION(BlueprintCallable)
		void PlayRandomCard();

	UFUNCTION(BlueprintCallable)
		void PlayAnother(UCard* prevCard);

	UFUNCTION(BlueprintCallable)
		UCard* GetLastPlayedCard();

	UFUNCTION(BlueprintCallable)
	void PopulateDeckFromBP();

	UPROPERTY(BlueprintReadWrite)
		TArray<UCard*> CardHistory;

	UPROPERTY(BlueprintReadWrite)
		UCard* CurrentPrevCard; 

	UPROPERTY(BlueprintAssignable)
	FDynamicPlayerMulticastDelegate OnPlayerActivated;

	UPROPERTY(BlueprintAssignable)
		FCardPlayedDelegate OnCardPlayed; 

	void DrawHand();

	void InitPlayer(UCardGame* InCardGame) noexcept;

	UCard* DrawCard();

	UFUNCTION(BlueprintCallable)
		bool DrawRandomCard(); 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Deck")
		TArray<TSubclassOf<UCard>> DeckBlueprint;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetCurrentDeckSize() const { return CurrentDeck.Num(); }

	UPROPERTY(BlueprintReadWrite)
		TArray<UCard*> Graveyard;

	UPROPERTY(BlueprintReadWrite)
		TArray<UCard*> CurrentHand;

	UPROPERTY(BlueprintReadWrite)
		TArray<UCard*> ActiveCards;

	UPROPERTY(BlueprintReadWrite)
		int Mana;

	UPROPERTY(BlueprintReadWrite)
		int MaxMana;

	UPROPERTY(BlueprintReadWrite)
		ARepoNPC* CurrentNPC; 


protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<UCard*> CurrentDeck;	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicRoundMulticastDelegate, UCardPlayer*, activePlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicEndGameDelegate, int, score);

UCLASS(Blueprintable)
class REPOMANREBOOT_API UCardGame : public UObject
{
	GENERATED_BODY()

public:
	UCardGame();

	//game flow
	UFUNCTION(BlueprintCallable)
	void InitGame(UCardPlayer* player1, UCardPlayer* player2, TArray<UCard*> specialCards);

	UFUNCTION(BlueprintCallable)
	void NextTurn();

	void StartTurn();

	UFUNCTION(BlueprintCallable)
	void RefreshWidget();

	UFUNCTION(BlueprintCallable)
	void EndGame();

	UFUNCTION(BlueprintCallable)
		void LoseGame();

	UFUNCTION(BlueprintImplementableEvent)
		void LoseGame_BP();

	UFUNCTION(BlueprintImplementableEvent)
		void EndGame_BP(int score);

	UPROPERTY(BlueprintAssignable)
		FDynamicEndGameDelegate OnEndGame;

	UFUNCTION(BlueprintCallable)
	void NextRound();

	UPROPERTY(BlueprintAssignable)
	FDynamicRoundMulticastDelegate OnNextRound; 

	UFUNCTION(BlueprintCallable)
		void PlayCard(UCard* inCard);

	UFUNCTION(BlueprintCallable)
		bool ActivatePlayer(int i);

	UFUNCTION(BlueprintImplementableEvent)
		void ActivatePlayer_BP(int i);

	UFUNCTION(BlueprintCallable)
		int GetNextPlayerIndex() const noexcept;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetActivePlayerIndex() const { return ActivePlayerIndex; }

	UFUNCTION(BlueprintCallable)
		FORCEINLINE UCardPlayer* GetActivePlayer() const { return ActivePlayer; }

private:
		UPROPERTY()
		int CardsPerTurnLimit;

		UPROPERTY()
		int ActivePlayerIndex; 

		UPROPERTY()
		UCardPlayer* ActivePlayer;

		UPROPERTY()
			UCardPlayer* PrevPlayer;

		void CheckLimits(int& ValueToCheck) const noexcept
		{
			if (ValueToCheck < -Limit) { ValueToCheck = -Limit; }
			else if (ValueToCheck > Limit) { ValueToCheck = Limit; }
		}

public:
	UFUNCTION(BlueprintImplementableEvent)
		void NewTurn_BP();

	UFUNCTION(BlueprintCallable)
		void PlayNeutralCard(TSubclassOf<UCard> inCard);

	UFUNCTION(BlueprintCallable)
		FORCEINLINE UCardPlayer* GetPrevPlayer() { return PrevPlayer; }

	UFUNCTION(BlueprintCallable)
	int ModAffection(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	int ModIntimidation(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	int ModEmpathy(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	void SetAffection(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	void SetIntimidation(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	void SetEmpathy(int m) noexcept;

	UFUNCTION(BlueprintCallable)
	void ApplyMods(UCard* InCard) noexcept;

	//game state

	UPROPERTY(BlueprintReadWrite, Category = "Game")
		TArray<UCardPlayer*> Players; 

	UPROPERTY(BlueprintReadWrite)
	TArray<UCard*> ActiveCards;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetAffection() const { return Affection; }

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetIntimidation() const { return Intimidation; }

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetEmpathy() const { return Empathy; }

	UPROPERTY(BlueprintReadWrite, Category = "Game")
		int Limit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Card Game")
		int TurnLimit;

	UPROPERTY(BlueprintReadOnly)
		int Turn;

	UPROPERTY(BlueprintReadOnly)
		int Round;

	UPROPERTY(BlueprintReadWrite)
		UCardGameWidget* CurrentWidget;

private:
	int Affection;
	int Intimidation;
	int Empathy;

};