// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueTooltip.h"
#include "Components/RichTextBlockDecorator.h"
#include "../RepoManRebootGameMode.h"
#include "RichTextTooltip.generated.h"

/**
 * 
 */
UCLASS()
class URichTextTooltip : public URichTextBlockDecorator
{
	GENERATED_BODY()


	public:
		URichTextTooltip(const FObjectInitializer& ObjectInitializer);

		UPROPERTY()
		ARepoManRebootGameMode* ControllerPointer;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
			TSubclassOf<UUserWidget> DialogueTooltip_BP;

		UPROPERTY()
			UUserWidget* TooltipWidget;
	
		virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
	
};
