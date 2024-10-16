// Fill out your copyright notice in the Description page of Project Settings.


#include "RichTextTooltip.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/ConstructorHelpers.h"
//#include "UMG/Public/Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Framework/Text/SlateTextRun.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "Fonts/FontMeasure.h"
#include "Slate/SlateGameResources.h"
#include "Components/RichTextBlock.h"
#include "../Managers/DialogueManager.h"
//#include "Editor.h"
//#include "Engine.h"
#include "TimerManager.h"
#include "Containers/UnrealString.h"
//#include "TOPDOWN_Take1/DialogueData/NPCLocalVariables.h"
#include "../RepoManRebootGameMode.h"

#define LOCTEXT_NAMESPACE "UMG"

class SDialogueTooltip : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogueTooltip) : _LocalVar(), _ControllerPointer(), _TooltipText()
	{}
	SLATE_ARGUMENT(FString, LocalVar)
	SLATE_ARGUMENT(ARepoManRebootGameMode*, ControllerPointer)
	SLATE_ARGUMENT(FText, TooltipText)
	
	SLATE_END_ARGS()

		UUserWidget* CurrentTooltip; 

	UPROPERTY()
		FTimerHandle UnusedHandle;

public:
	SDialogueTooltip();

	ARepoManRebootGameMode* MyController;
	FString LocalString;
	FText CachedTooltipText; 
	
	void Construct(const FArguments& InArgs, const FTextBlockStyle& Style, FString InText, FText TooltipText, UUserWidget* Tooltip, bool bSyncStyle)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Creating SWidget"));
		FButtonStyle ButtonStyle;
		CurrentTooltip = Tooltip; 
		//UpdateTooltip(TooltipText);
		
		//FTimerDelegate AttackAgain = FTimerDelegate::CreateUObject(this, &ATOPDOWN_Take1PlayerController::AutoAttackStart, MyEnemy);
		TSharedPtr< SWidget > ButtonContent;
		ChildSlot
			[SAssignNew(ButtonContent, STextBlock)
			//.ShadowColorAndOpacity(FLinearColor::Black)
			.ColorAndOpacity(Style.ColorAndOpacity)
			//.ShadowOffset(FIntPoint(-1, 1))
			//.Font(FSlateFontInfo("Verdana", 16))
			.Font(Style.Font)
			.Text(FText::FromString(InText))
			//.ToolTipText(TooltipText)
			
			/*.OnMouseEnter_Lambda([this]
		{
			ShowTooltip();
		})*/
			//]
			];

		CachedTooltipText = FText::FromString(InText);
		UE_LOG(LogTemp, Warning, TEXT("SWidget() - InText = %s, TooltipText = %s"), *InText, *TooltipText.ToString());
		//UpdateTooltip(InArgs._ToolTipText.Get());

		if(InArgs._ControllerPointer && !(InArgs._LocalVar=="0"))
		{
			MyController = InArgs._ControllerPointer;
			LocalString = InArgs._LocalVar;

			ButtonContent.Get()->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
				[this, InArgs](const FGeometry& G, const FPointerEvent& E) {

				ShowTooltip();
				ChangeVar();
				UpdateTooltip(InArgs._TooltipText);
			}
			));

			UE_LOG(LogTemp, Warning, TEXT("LocalString = %s!"), *(InArgs._LocalVar));
		}
		else

		{
			UE_LOG(LogTemp, Warning, TEXT("No LocalVar"));
			
			ButtonContent.Get()->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
				[this, InArgs](const FGeometry& G, const FPointerEvent& E) {

				ShowTooltip();
				UpdateTooltip(InArgs._TooltipText);
			}
			));
		}
		
		/*ButtonContent.Get()->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
			[this](const FGeometry& G, const FPointerEvent& E, FString LocalString, ATOPDOWN_Take1PlayerController* MyController) {

			ShowTooltip();
			ChangeVar(LocalString, MyController);
		}
		));*/

		/*ButtonContent.Get()->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
			[this](const FGeometry& G, const FPointerEvent& E) {

			ShowTooltip();
			ChangeVar();
		}
		));*/

		ButtonContent.Get()->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
			[this](const FPointerEvent& E) {
			HideTooltip();
		}
		));

		if (bSyncStyle)
		{
			CurrentTooltip->SetColorAndOpacity(Style.ColorAndOpacity.GetSpecifiedColor());
		}

		
	}

	UFUNCTION(BlueprintCallable)
		void ChangeVar()
	{
		
		if (!(LocalString == "0"))
		{
			UE_LOG(LogTemp, Warning, TEXT("Changing LocalVar %s!"), *LocalString);
			if (MyController)
			{

				MyController->DialogueManager->SetGlobalVariable_Flag(FName(*LocalString),true);

				/*if (MyController->CurrentNPC)
				{
					ATestNPC1* MyNPC = MyController->CurrentNPC;
					if (MyNPC->NPCLocals)
					{
						if (FNPCLocalVariables* LOCALS = MyNPC->NPCLocals->FindRow<FNPCLocalVariables>(FName(*LocalString), TEXT("")))
						{
							(*LOCALS).Flag = true;
						}
					}
				}*/
			}
		}
		
	}
		UFUNCTION(BlueprintCallable)
		void UpdateTooltip(FText inText)
		{
			if (auto cast = Cast<UDialogueTooltip>(CurrentTooltip))
			{
				cast->TooltipTextBlock->SetText(inText);
				UE_LOG(LogTemp, Warning, TEXT("Updating TooltipText: %s"), *inText.ToString());
			}
		}


	UFUNCTION(BlueprintCallable)
	void ShowTooltip()
	{
		if (CurrentTooltip)
		{
			FVector2D mouse_coordinates;
			if (GEngine) GEngine->GameViewport->GetMousePosition(mouse_coordinates);
			CurrentTooltip->ForceLayoutPrepass();
			CurrentTooltip->AddToViewport();
			
			FVector2D TooltipSize = CurrentTooltip->GetDesiredSize();

			CurrentTooltip->ForceLayoutPrepass();
			FGeometry geometry = CurrentTooltip->GetCachedGeometry();
			FVector2D localSize = geometry.GetLocalSize();
			FVector2D screenPosition = geometry.LocalToAbsolute(FVector2D(0, 0)); //TopLeft
			FVector2D screenSize = geometry.LocalToAbsolute(localSize) - screenPosition; // BotRight-TopLeft = real size

			FVector2D ViewportSize = FVector2D(1, 1);

			CurrentTooltip->SetPositionInViewport(mouse_coordinates);
			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->GetViewportSize( /*out*/ViewportSize);
			}

			UE_LOG(LogTemp, Warning, TEXT("Viewport size: %f %f"), ViewportSize.X, ViewportSize.Y);
			UE_LOG(LogTemp, Warning, TEXT("Widget size: %f %f"), TooltipSize.X, TooltipSize.Y);
			UE_LOG(LogTemp, Warning, TEXT("Widget size (local): %f %f"), localSize.X, localSize.Y);
			UE_LOG(LogTemp, Warning, TEXT("Mouse coords: %f %f"), mouse_coordinates.X, mouse_coordinates.Y);

			if (ViewportSize.X - mouse_coordinates.X < TooltipSize.X)
			{
				UE_LOG(LogTemp, Warning, TEXT("TOO CLOSE TO RIGHT EDGE"));
				CurrentTooltip->SetPositionInViewport(mouse_coordinates - FVector2D(TooltipSize.X, 0.f) / 2);
			}
			else if (ViewportSize.Y - mouse_coordinates.Y < TooltipSize.Y)
			{
				UE_LOG(LogTemp, Warning, TEXT("TOO CLOSE TO BOTTOM EDGE"));
				CurrentTooltip->SetPositionInViewport(mouse_coordinates - FVector2D(0.f, TooltipSize.Y) / 2);
			}
			else if ((ViewportSize.X - mouse_coordinates.X < TooltipSize.X) && (ViewportSize.Y - mouse_coordinates.Y < TooltipSize.Y))
			{
				UE_LOG(LogTemp, Warning, TEXT("TOO CLOSE TO RIGHT AND BOTTOM EDGE"));
				CurrentTooltip->SetPositionInViewport(mouse_coordinates - FVector2D(TooltipSize.X, TooltipSize.Y) / 2);
			}
			else
			{
				CurrentTooltip->SetPositionInViewport(mouse_coordinates);
			}

			//CurrentTooltip->SetPosition();
			CurrentTooltip->SetVisibility(ESlateVisibility::HitTestInvisible);
			FLinearColor BaseTint = CurrentTooltip->ColorAndOpacity;
			CurrentTooltip->SetColorAndOpacity(FLinearColor(BaseTint.R, BaseTint.G, BaseTint.B, 1.0f));

			LerpTime = 0.f;
			LerpDuration = 0.5f;

			TargetTint = BaseTint;
			TargetTint.A = 1.0f;

			RampSwitch = 1;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SDialogueTooltip::ShowTooltip(): no current tooltip"));
		}
		
	}

	UPROPERTY()
	FLinearColor TargetTint;
	float LerpTime;
	float LerpDuration; 
	int RampSwitch;
	
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
		
	//UFUNCTION(BlueprintCallable)
	//	void RampOpacityUp()
	//{
	//	FLinearColor BaseTint = CurrentTooltip->ColorAndOpacity;

	//	if (BaseTint.A < 1.0f)
	//	{
	//		CurrentTooltip->SetColorAndOpacity(FLinearColor(BaseTint.R, BaseTint.G, BaseTint.B, BaseTint.A + 0.05f));
	//		//CurrentTooltip->GetWorld()->GetTimerManager().SetTimer(UnusedHandle, this, &SDialogueTooltip::RampOpacityUp, 0.05f, false);
	//	}
	//	if (BaseTint.A >= 1.0f)
	//	{
	//		RampSwitch = 0;
	//	}
	//}

	UFUNCTION(BlueprintCallable)
	void HideTooltip()
	{
		if (CurrentTooltip)
		{
			//LerpTime = 0.f;
			//LerpDuration = 0.5f;

			//FLinearColor BaseTint = CurrentTooltip->ColorAndOpacity;
			//TargetTint = BaseTint;
			//TargetTint.A = 0.f;

			//RampSwitch = 0;

			////CurrentTooltip->RemoveFromViewport();
			//UE_LOG(LogTemp, Warning, TEXT("HIDING TOOLTIP"));

			CurrentTooltip->RemoveFromParent();
		}
	}
};

SDialogueTooltip::SDialogueTooltip()
{
	RampSwitch = 0; 
}


void SDialogueTooltip::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

	//if (LerpTime < LerpDuration)
	//{
	//	if (CurrentTooltip)
	//	{
	//		//UE_LOG(LogTemp, Warning, TEXT("LERP TICK"));
	//		LerpTime += InDeltaTime;

	//		FLinearColor BaseTint = CurrentTooltip->ColorAndOpacity;

	//		FLinearColor Color = FLinearColor(BaseTint.R, BaseTint.G, BaseTint.B, FMath::Lerp<float>(BaseTint.A, TargetTint.A, LerpTime / LerpDuration));

	//		CurrentTooltip->SetColorAndOpacity(Color);

	//		if (RampSwitch == 0 && BaseTint.A == 0.f) CurrentTooltip->RemoveFromParent();
	//	}
	//}

}


class FDefaultRichTextDecorator : public ITextDecorator
{
public:
	FDefaultRichTextDecorator(URichTextBlockDecorator* Decorator, const FSlateFontInfo& DefaultFont, const FLinearColor& DefaultColor, URichTextBlock* InOwner, UUserWidget* TooltipWidgetPassed, ARepoManRebootGameMode* InControllerPointer);

	virtual ~FDefaultRichTextDecorator();

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;

	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
		TSubclassOf<UUserWidget> DialogueTooltip_BP;*/

	UPROPERTY()
		UUserWidget* TooltipWidget;

	UPROPERTY()
		ARepoManRebootGameMode* ControllerPointer; 


protected:

	virtual TSharedRef<ISlateRun> CreateRun(const TSharedRef<FTextLayout>& TextLayout, const FRunInfo& InRunInfo, const TSharedRef< FString >& InText, const FTextBlockStyle& Style, const FTextRange& InRange, FText& TooltipText, FString ContentText, bool& bSyncStyle, FString& LocalVar);

	FTextBlockStyle CreateTextBlockStyle(const FRunInfo& InRunInfo, FText& TooltipText, bool& bSyncStyle, FString& LocalVar) const;

	void ExplodeRunInfo(const FRunInfo& InRunInfo, FSlateFontInfo& OutFont, FLinearColor& OutFontColor, FText& OutTooltipText, bool& bSyncStyle, FString& LocalVar) const;

	UFUNCTION()
	void ShowTooltip()
	{
		UE_LOG(LogTemp, Warning, TEXT("SHOWING TOOLTIP"));
	}

protected:

	FSlateFontInfo DefaultFont;
	FLinearColor DefaultColor;
	
	//TESTING
	UFUNCTION()
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FRunInfo& RunInfo, const FTextBlockStyle& TextStyle, FString InText, FText TooltipText, UUserWidget* TooltipWidgetPass2, bool& bSyncStyle, FString LocalVar)
	{

		//FSimpleDelegate Show = FSimpleDelegate::CreateLambda([]()
		//{
		//	FDefaultRichTextDecorator::ShowTooltip();
		//});

		/*FSimpleDelegate Show = FSimpleDelegate::CreateLambda([]()
		{
			ShowTooltip();
		});*/
		
		return SNew(SDialogueTooltip, TextStyle, InText, TooltipText, TooltipWidget, bSyncStyle).LocalVar(LocalVar).ControllerPointer(ControllerPointer).TooltipText(TooltipText);
	}
	//TESTING

private:
	TWeakObjectPtr<URichTextBlockDecorator> Decorator;
};

class FDefaultRichTextRun : public FSlateTextRun
{
public:
	FDefaultRichTextRun(URichTextBlockDecorator* InDecorator, const TSharedRef<FTextLayout>& InTextLayout, const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FTextBlockStyle& InStyle, const FTextRange& InRange)
		: FSlateTextRun(InRunInfo, InText, InStyle, InRange)
		, TextLayout(InTextLayout)
		, Decorator(InDecorator)
	{
			
	}

	virtual int32 OnPaint(const FPaintArgs& PaintArgs, const FTextArgs& TextArgs, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		FSlateTextRun::OnPaint(PaintArgs, TextArgs, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		//FSlateTextRun::OnPaint(Args, Line, Block, DefaultStyle, AllottedGeometry, ClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

		return LayerId;
	}

private:
	const TSharedRef<FTextLayout> TextLayout;
	TWeakObjectPtr<URichTextBlockDecorator> Decorator;
	
};

FDefaultRichTextDecorator::FDefaultRichTextDecorator(URichTextBlockDecorator* InDecorator, const FSlateFontInfo& InDefaultFont, const FLinearColor& InDefaultColor, URichTextBlock* InOwner, UUserWidget* TooltipWidgetPassed, ARepoManRebootGameMode* InControllerPointer)
	: DefaultFont(InDefaultFont)
	, DefaultColor(InDefaultColor)
	, Decorator(InDecorator)
{
	TooltipWidget = TooltipWidgetPassed;
	ControllerPointer = InControllerPointer;

	//UE_LOG(LogTemp, Warning, TEXT("Tooltip widget passed"));

}

FDefaultRichTextDecorator::~FDefaultRichTextDecorator()
{
}

bool FDefaultRichTextDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const
{
	return (RunParseResult.Name == TEXT("TOOLTIP"));
}

TSharedRef<ISlateRun> FDefaultRichTextDecorator::Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style)
{
	FRunInfo RunInfo(RunParseResult.Name);
	//FTextRunInfo RunInfo2(RunParseResult.Name, FText::FromString(OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex)));
	for (const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData)
	{
		RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid(Pair.Value.BeginIndex, Pair.Value.EndIndex - Pair.Value.BeginIndex));
	}

	FString ContentText = OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex);
	
	FTextRange ModelRange;
	ModelRange.BeginIndex = InOutModelText->Len();
	//*InOutModelText += OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex);
	//*InOutModelText += TEXT('\u200B');
	*InOutModelText += TEXT('\u200B');// Zero-Width Breaking Space
	ModelRange.EndIndex = InOutModelText->Len();

	/*ModelRange.BeginIndex = RunParseResult.ContentRange.BeginIndex;
	ModelRange.EndIndex = RunParseResult.ContentRange.EndIndex;*/
	
	//ModelRange.EndIndex = ModelRange.BeginIndex + 1; 
	FText TooltipText;

	
	//ContentText += TEXT('\u200B');
	UE_LOG(LogTemp, Warning, TEXT("Content Text = %s"), *ContentText);
	
	//const FTextBlockStyle& TextStyle = CreateTextBlockStyle(RunInfo, TooltipText);
	bool bSyncStyle = false;
	FString LocalVar = "0"; 
	
	return CreateRun(TextLayout, RunInfo, InOutModelText, CreateTextBlockStyle(RunInfo, TooltipText, bSyncStyle, LocalVar), ModelRange, TooltipText, ContentText, bSyncStyle, LocalVar);
}

TSharedRef<ISlateRun> FDefaultRichTextDecorator::CreateRun(const TSharedRef<FTextLayout>& TextLayout, const FRunInfo& InRunInfo, const TSharedRef< FString >& InText, const FTextBlockStyle& Style, const FTextRange& InRange, FText& TooltipText, FString ContentText, bool& bSyncStyle, FString& LocalVar)
{
	//FTextRange ModelRange;
	//ModelRange.BeginIndex = InText->Len();

	//FTextRange ModRange = InRange; 
	
	TSharedPtr<ISlateRun> SlateRun;
	TSharedPtr<SWidget> DecoratorWidget = CreateDecoratorWidget(InRunInfo, Style, ContentText, TooltipText, TooltipWidget, bSyncStyle, *LocalVar);

	if (DecoratorWidget.IsValid())
	{
	    //*InText += TEXT('\u200B'); // Zero-Width Breaking Space
		//ModelRange.EndIndex = InText->Len();

		// Calculate the baseline of the text within the owning rich text
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		int16 WidgetBaseline = (FontMeasure->GetBaseline(Style.Font) - FMath::Min(0.0f, Style.ShadowOffset.Y));
		//TSharedRef < FString > Convert = TSharedRef<FString>(&ContentText); 
		
		FSlateWidgetRun::FWidgetRunInfo WidgetRunInfo(DecoratorWidget.ToSharedRef(), WidgetBaseline);
		SlateRun = FSlateWidgetRun::Create(TextLayout, InRunInfo, InText, WidgetRunInfo, InRange);
	}
	else
	{
		//Assume there's a text handler if widget is empty, if there isn't one it will just display an empty string
		FTextBlockStyle TempStyle = Style;
		//CreateDecoratorText(InRunInfo, TempStyle, *InText);

		//InRange.EndIndex = InText->Len();
		SlateRun = FSlateTextRun::Create(InRunInfo, InText, TempStyle, InRange);
	}

	return SlateRun.ToSharedRef();
	//return MakeShareable(new FDefaultRichTextRun(Decorator.Get(), TextLayout, InRunInfo, InText, Style, ModelRange));
}

FTextBlockStyle FDefaultRichTextDecorator::CreateTextBlockStyle(const FRunInfo& InRunInfo, FText& TooltipText, bool& bSyncStyle, FString& LocalVar) const
{
	FSlateFontInfo Font;
	FLinearColor FontColor;
	ExplodeRunInfo(InRunInfo, Font, FontColor, TooltipText, bSyncStyle, LocalVar);

	FTextBlockStyle TextBlockStyle;
	TextBlockStyle.SetFont(Font);
	TextBlockStyle.SetColorAndOpacity(FontColor);

	return TextBlockStyle;
}

void FDefaultRichTextDecorator::ExplodeRunInfo(const FRunInfo& InRunInfo, FSlateFontInfo& OutFont, FLinearColor& OutFontColor, FText& OutTooltipText, bool& bSyncStyle, FString& LocalVar) const
{
	OutFont = DefaultFont;

	const FString* const FontFamilyString = InRunInfo.MetaData.Find(TEXT("font"));
	const FString* const FontSizeString = InRunInfo.MetaData.Find(TEXT("size"));
	const FString* const FontStyleString = InRunInfo.MetaData.Find(TEXT("style"));
	const FString* const FontColorString = InRunInfo.MetaData.Find(TEXT("color"));
	const FString* const TooltipString = InRunInfo.MetaData.Find(TEXT("content"));
	const FString* const SyncstyleString = InRunInfo.MetaData.Find(TEXT("syncstyle"));
	const FString* const LocalVarString = InRunInfo.MetaData.Find(TEXT("localvar"));

	if (LocalVarString)
	{
		LocalVar = *LocalVarString;
		UE_LOG(LogTemp, Warning, TEXT("LocalVar ID = %s"), **LocalVarString);
	}
	
	if (TooltipString)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tooltip content = %s"), **TooltipString);
		OutTooltipText = FText::FromString(*TooltipString);

		if (UDialogueTooltip* MyWidget = Cast<UDialogueTooltip>(TooltipWidget))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Tooltip widget cast succesful, tooltip content = %s"), **TooltipString);
			MyWidget->TooltipTextBlock->SetText(OutTooltipText);
			//MyWidget->AddToViewport();
			//MyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	if (FontFamilyString)
	{
		FSoftObjectPath Font(**FontFamilyString);
		if (UObject* FontAsset = Font.TryLoad())
		{
			OutFont.FontObject = FontAsset;
		}
	}

	if (SyncstyleString)
	{
		int HelperInt = static_cast<uint8>(FPlatformString::Atoi(**SyncstyleString));
		if (HelperInt == 1) bSyncStyle = true;
		else bSyncStyle = false;
		
	}
	
	if (FontSizeString)
	{
		OutFont.Size = static_cast<uint8>(FPlatformString::Atoi(**FontSizeString));
	}

	if (FontStyleString)
	{
		OutFont.TypefaceFontName = FName(**FontStyleString);
	}

	OutFontColor = DefaultColor;
	if (FontColorString)
	{
		const FString& FontColorStringRef = *FontColorString;

		// Is Hex color?
		if (!FontColorStringRef.IsEmpty() && FontColorStringRef[0] == TCHAR('#'))
		{
			OutFontColor = FLinearColor(FColor::FromHex(FontColorStringRef));
		}
		else if (OutFontColor.InitFromString(*FontColorString))
		{
			// Initialized
		}
		else
		{
			OutFontColor = DefaultColor;
		}
	}
}

URichTextTooltip::URichTextTooltip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*static ConstructorHelpers::FObjectFinder<UBlueprint>WidgetBlueprintObject(TEXT("WidgetBlueprint'/Game/TopDownCPP/UI/DialogueTooltip_BP.DialogueTooltip_BP'"));
	if (WidgetBlueprintObject.Succeeded())
	{
		DialogueTooltip_BP = WidgetBlueprintObject.Object->GetClass();
	}*/

	//TEST

	/*static ConstructorHelpers::FClassFinder<UUserWidget> Cool_use(TEXT("/Game/UI/DialogueTooltip_BP"));
	if (Cool_use.Succeeded())
	{
		DialogueTooltip_BP = Cool_use.Class;
	}*/




	//TEST
}

TSharedPtr<ITextDecorator> URichTextTooltip::CreateDecorator(URichTextBlock* InOwner)
{
	

	if (auto MyInstance = InOwner->GetGameInstance())
		{
			if (auto MyController = Cast<ARepoManRebootGameMode>(UGameplayStatics::GetGameMode(MyInstance->GetWorld())))
			{
				UE_LOG(LogTemp, Warning, TEXT("Controller cast succesful"));
				ControllerPointer = MyController; 
			}
		}

	
		
	if (DialogueTooltip_BP)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Blueprint"));



		FString CurrentTooltipName = "Tooltip" + FString::FromInt(FMath::RandRange(0,100000));
		TooltipWidget = CreateWidget<UDialogueTooltip>(InOwner, DialogueTooltip_BP, FName(*CurrentTooltipName));

		
		if (TooltipWidget)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Tooltip widget created"));
			if (UDialogueTooltip* MyTooltip = Cast<UDialogueTooltip>(TooltipWidget))
			{


			}
			//TooltipWidget->AddToViewport(); 
			TooltipWidget->RemoveFromViewport();
		}
		//}
	}

	FSlateFontInfo DefaultFont = InOwner->GetDefaultTextStyle().Font;
	FLinearColor DefaultColor = InOwner->GetDefaultTextStyle().ColorAndOpacity.GetSpecifiedColor();
	return MakeShareable(new FDefaultRichTextDecorator(this, DefaultFont, DefaultColor, InOwner, TooltipWidget, ControllerPointer));
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
