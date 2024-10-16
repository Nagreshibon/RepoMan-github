// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"

#if WITH_EDITOR
#include "EditorUtilityWidget.h"
#include <RepoManReboot/Dialogue/BigDialogueBody.h>
#include "EditorUtilityWidgetComponents.h"
#include <Factories/Factory.h>
#include <Factories/DataTableFactory.h>
#include "../RapidXML/rapidxml.hpp"
#endif

#include "ArticyToInternalCSVConverter.generated.h"


/**
 * 
 */

USTRUCT(BlueprintType)
struct FParsedDialogueFragments
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> DialogueFragmentIds;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> ConnectionIds;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> HubIds;
};

USTRUCT(BlueprintType)
struct FParsedHub
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> TargetDialogueFragmentIds;
};

USTRUCT(BlueprintType)
struct FParsedConnection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Source;

	UPROPERTY(BlueprintReadOnly)
	FString Target;
};


//UCLASS()
//class REPOMANREBOOT_API UDataTableFactoryWrapper : public UDataTableFactory
//{
//	GENERATED_BODY()
//};

using namespace rapidxml;

#if WITH_EDITORONLY_DATA
UCLASS(config = Editor)
class UArticyToInternalCSVConverter : public UEditorUtilityWidget
//class Minimal_API UArticyToInternalCSVConverter : public UEditorUtilityWidget

{
	GENERATED_BODY()
	
public:

//#if WITH_EDITOR
//	UFUNCTION(BlueprintCallable, BlueprintPure)
//	static UWorld* GetEditorWorld()
//	{
//		return GEditor->GetEditorWorldContext(false).World();
//	}
//#endif


	UFUNCTION(BlueprintCallable)
	void OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta= (BindWidget))
	UEditorUtilityMultiLineEditableTextBox* DebugTextBox;

	UPROPERTY(BlueprintReadWrite)
	FString DebugString;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> LoadedFilePaths;

	UPROPERTY(BlueprintReadOnly)
	FString LoadedFile;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> LoadedFilesArr;

	UFUNCTION(BlueprintCallable)
	void LoadMultipleFiles(TArray<FString> pathArr);

	UFUNCTION(BlueprintCallable)
	FString& LoadFile(FString path);

	UFUNCTION(BlueprintCallable)
	void ProcessLoadedFiles();

	TMap<int, FBigDialogueBody> ProcessFile(FString& inFile);

	// READ DIALOGUE NAMES

	//xml_document<> XmlDoc;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> CurrentDialogueMap;

	TMap<FString, FString> ReadDialogues(xml_document<>& doc);

	// PARSE DIALOGUE STRINGS INTO FRAGMENTS
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FParsedDialogueFragments> CurrentDialogueFragmentIds;

	UPROPERTY()
	TMap<FString, FBigDialogueBody> ParsedDialogueFragments;

	UPROPERTY()
	TMap<FString, FParsedHub> ParsedHubs;

	UPROPERTY()
	TMap<FString, FParsedConnection> ParsedConnections;

	void ParseDialogueFragmentsIntoStruct(const xml_document<>& doc, FString inID);

	void ParseHubs(const xml_document<>& doc, FString inID);

	void ParseEntities(const xml_document<>& doc);

	UPROPERTY()
	TMap<FString, FString> ParsedEntities;

	void ParseConnections(const xml_document<>& doc, FString inID);

	UFUNCTION(BlueprintCallable)
	void PrintParsedDialogueLinesToDebugString();

	// FILL DATA TABLE

	//UDataTable* OutputDataTable;

	UFUNCTION(BlueprintCallable)
	UDataTable* CreateDataTable(FString inID);

	//UPROPERTY(BlueprintReadOnly)
	TMap<FString, UDataTable*> ResultTables;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UDataTable* CurrentDataTable;

	UFUNCTION(BlueprintCallable)
	void SaveDataTables();

	UFUNCTION(BlueprintCallable)
	void SaveDataTable(FString inID);

private:
	void SaveDataTable_Internal(UDataTable* inTable, FString inPath);


	xml_document<> XmlDoc;
	


};
#endif