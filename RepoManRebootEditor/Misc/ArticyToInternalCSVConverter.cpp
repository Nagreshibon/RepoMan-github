// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_EDITOR

#include "ArticyToInternalCSVConverter.h"

#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
//#include "FileOps.h"
//#endif
#include <Factories/BlueprintFactory.h>
#include <AssetToolsModule.h>
#include <ContentBrowserModule.h>
#include <FindInBlueprintManager.h>
#include "UObject/SavePackage.h"
#include "IContentBrowserSingleton.h"
#include <AssetRegistry/AssetRegistryModule.h>
#include "UObject/Class.h"
#include "../RapidXML/rapidxml.hpp"

#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Editor/UnrealEd/Public/DataTableEditorUtils.h"
#include "Factories/ReimportDataTableFactory.h"

#endif

using namespace rapidxml;

void UArticyToInternalCSVConverter::OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames)
{
#if WITH_EDITORONLY_DATA

    void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        uint32 SelectionFlag = 1; //A value of 0 represents single file selection while a value of 1 represents multiple file selection
        DesktopPlatform->OpenFileDialog(ParentWindowPtr, DialogTitle, DefaultPath, FString(""), FileTypes, SelectionFlag, OutFileNames);
    }
#endif
}

void UArticyToInternalCSVConverter::LoadMultipleFiles(TArray<FString> pathArr)
{
#if WITH_EDITOR
    LoadedFilesArr.Empty();
    DebugString.Empty();

    for (auto& p : pathArr)
    {
        LoadedFilesArr.Add(LoadFile(p));
    }
#endif
}

FString& UArticyToInternalCSVConverter::LoadFile(FString path)
{
    if (FFileHelper::LoadFileToString(LoadedFile, *path))
    {
        UE_LOG(LogTemp, Warning, TEXT("File %s successfuly loaded"),*path);

        return LoadedFile;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid path"));

        LoadedFile.Empty();
        return LoadedFile;
    }

}

void UArticyToInternalCSVConverter::ProcessLoadedFiles()
{
    ResultTables.Empty();
    for (auto& f : LoadedFilesArr)
    {
        ProcessFile(f);
    }
}

TMap<int, FBigDialogueBody> UArticyToInternalCSVConverter::ProcessFile(FString& inFile)
{
    TMap<int, FBigDialogueBody> ret;

    inFile.ReplaceInline(TEXT("<![CDATA["),TEXT(" "));
    inFile.ReplaceInline(TEXT("]]>"), TEXT(" "));

    //xml_document<> XmlDoc;
    XmlDoc.clear();
    XmlDoc.parse<0>(TCHAR_TO_UTF8(*inFile));

    DebugString.Empty();
    int i = 0;
    for (xml_node<>* n = XmlDoc.first_node(); n; n = n->next_sibling())
    {
        DebugString.Append(TEXT("NODE ") + FString::FromInt(i) + TEXT(" LEVEL 0") + TEXT("\n"));

        for (xml_node<>* s1 = n->first_node(); s1; s1 = s1->next_sibling())
        {
            DebugString.Append(TEXT("   s1: "));
            DebugString.Append(s1->name());
            DebugString.Append(TEXT("\n"));

            for (xml_node<>* s2 = s1->first_node(); s2; s2 = s2->next_sibling())
            {
                DebugString.Append(TEXT("      s2: "));
                DebugString.Append(s2->name());
                DebugString.Append(TEXT("\n"));

                for (xml_attribute<>* attr = s2->first_attribute(); attr; attr = attr->next_attribute())
                {
                    DebugString.Append(TEXT("         attr: "));
                    DebugString.Append(attr->value());
                    DebugString.Append(TEXT("\n"));
                }
            }
        }
        i++;
    }

    /*auto r = ReadDialogues(XmlDoc);
    CurrentDialogueMap = r;*/

    CurrentDialogueMap.Empty();
    CurrentDialogueMap = ReadDialogues(XmlDoc);

    for (auto& d : CurrentDialogueMap)
    {
        UE_LOG(LogTemp, Warning, TEXT("Parsing dialogue - %s"), *d.Key);

        ParseEntities(XmlDoc);
        ParseDialogueFragmentsIntoStruct(XmlDoc, d.Value);
        ParseHubs(XmlDoc, d.Value);
        ParseConnections(XmlDoc, d.Value);          
        PrintParsedDialogueLinesToDebugString();

        ResultTables.FindOrAdd(d.Value,CreateDataTable(d.Value));
    }

    return ret;
}

TMap<FString, FString> UArticyToInternalCSVConverter::ReadDialogues(xml_document<>& doc)
{
    TMap<FString, FString> ret;

    //MAPPING
    DebugString.Empty();
    CurrentDialogueFragmentIds.Empty();

    xml_node<>* n = doc.first_node();
    if (n)
    {
        xml_node<>* s1 = n->first_node("Content");

        if (s1)
        {
            for (xml_node<>* s2 = s1->first_node("Dialogue"); s2; s2 = s2->next_sibling("Dialogue"))
            {

                DebugString.Append(TEXT("      s2: "));
                DebugString.Append(s2->name());
                DebugString.Append(TEXT("\n"));

                auto s3 = s2->first_node("DisplayName");
                if (s3)
                {
                    auto s4 = s3->first_node("LocalizedString");
                    if (s4)
                    {
                       ret.Add(s4->value(), s2->first_attribute("Id")->value());
                    }
                }
            }
        }
    }

    //PARSING DIALOGUE DATA

    TMap<FString, FParsedDialogueFragments> temp_CurrentDialogueFragmentIds;
    //n = doc.first_node();
    if (n)
    {
        xml_node<>* s1 = n->first_node("Hierarchy");

        if (s1)
        {
            for (xml_node<>* s2 = s1->first_node("Node"); s2; s2 = s2->next_sibling("Node"))
            {
                if (strcmp(s2->first_attribute("Type")->value(), "Project") == 0)
                {
                    for (xml_node<>* s3 = s2->first_node("Node"); s3; s3 = s3->next_sibling("Node"))
                    {
                        if (strcmp(s3->first_attribute("Type")->value(), "Flow") == 0)
                        {
                            for (xml_node<>* s4 = s3->first_node("Node"); s4; s4 = s4->next_sibling("Node"))
                            {
                                if (strcmp(s4->first_attribute("Type")->value(), "FlowFragment") == 0)
                                {
                                    for (xml_node<>* s5 = s4->first_node("Node"); s5; s5 = s5->next_sibling("Node"))
                                    {
                                        if (strcmp(s5->first_attribute("Type")->value(), "Dialogue") == 0)
                                        {
                                            for (xml_node<>* s6 = s5->first_node("Node"); s6; s6 = s6->next_sibling("Node"))
                                            {
                                                FString str = s5->first_attribute("IdRef")->value();
                                                FString str2 = s6->first_attribute("IdRef")->value();

                                                if (strcmp(s6->first_attribute("Type")->value(), "DialogueFragment") == 0)
                                                {
                                                    temp_CurrentDialogueFragmentIds
                                                        .FindOrAdd(str)                                                  
                                                        .DialogueFragmentIds
                                                        .Add(str2);
                                                }

                                                if (strcmp(s6->first_attribute("Type")->value(), "Connection") == 0)
                                                {
                                                    temp_CurrentDialogueFragmentIds
                                                        .FindOrAdd(str)
                                                        .ConnectionIds
                                                        .Add(str2);
                                                }

                                                if (strcmp(s6->first_attribute("Type")->value(), "Hub") == 0)
                                                {
                                                    temp_CurrentDialogueFragmentIds
                                                        .FindOrAdd(str)
                                                        .HubIds
                                                        .Add(str2);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    CurrentDialogueFragmentIds = temp_CurrentDialogueFragmentIds; 

    DebugString.Append(TEXT("CurrentDialogueFragmentIds: \n"));
    for (auto& c : CurrentDialogueFragmentIds)
    {
        DebugString.Append(c.Key + "\n");
    }

    int i = 0;
    for (auto& d : ret)
    {
        DebugString.Append(TEXT("LINE ") + FString::FromInt(i) + "\n");
        DebugString.Append(d.Key + " " + d.Value + "\n");

        
        if (auto parsedstruct = CurrentDialogueFragmentIds.Find(d.Value))
        {
            DebugString.Append(TEXT("DialogueFragments: \n"));
            for (auto& k : parsedstruct->DialogueFragmentIds)
            {
                DebugString.Append(k + "\n");
            }

            DebugString.Append(TEXT("Connections: \n"));
            for (auto& k : parsedstruct->ConnectionIds)
            {
                DebugString.Append(k + "\n");
            }

            DebugString.Append(TEXT("Hubs: \n"));
            for (auto& k : parsedstruct->HubIds)
            {
                DebugString.Append(k + "\n");
            }
        }

        i++;
    }

    return ret;
}

void UArticyToInternalCSVConverter::ParseDialogueFragmentsIntoStruct(const xml_document<>& doc, FString inID)
{
    DebugString.Empty();
    ParsedDialogueFragments.Empty();
    int FallbackIndex = 0;
    TSet<int> UsedIndices;
    TArray<FString> UsedSpeakers;

    xml_node<>* n = doc.first_node();
    if (n)
    {
        UE_LOG(LogTemp, Warning, TEXT("   ParseDialogueFragmentsIntoStruct() - if(n)"));
        xml_node<>* s1 = n->first_node("Content");

        if (s1)
        {
            UE_LOG(LogTemp, Warning, TEXT("   ParseDialogueFragmentsIntoStruct() - if(s1)"));
            for (xml_node<>* s2 = s1->first_node("DialogueFragment"); s2; s2 = s2->next_sibling("DialogueFragment"))
            {
                auto NewLine = FBigDialogueBody();

                FString id = s2->first_attribute("Id")->value();

                if (auto fragments = CurrentDialogueFragmentIds.Find(inID))
                {
                    if (fragments->DialogueFragmentIds.Contains(id))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("      ParseDialogueFragmentsIntoStruct() - found fragment %s in parsed fragments"), *id);

                        DebugString.Append(TEXT("      s2: "));
                        DebugString.Append(s2->name());
                        DebugString.Append(TEXT("\n"));
                        DebugString.Append(s2->first_attribute("Id")->value());
                        DebugString.Append(TEXT("\n"));

                        auto s3 = s2->first_node("Text");
                        if (s3)
                        {
                            auto s4 = s3->first_node("LocalizedString");
                            if (s4)
                            {
                                //FString dstring = FString("\"").Append(s4->value());
                                //dstring.Append(FString("\""));
                                NewLine.DialogueText = FText::FromString(s4->value());
                            }
                        }

                        auto s3_1 = s2->first_node("Speaker");
                        if (s3_1)
                        {
                            auto SpeakerId = s3_1->first_attribute("IdRef");
                            if (SpeakerId)
                            {
                                auto fp = ParsedEntities.Find(SpeakerId->value());
                                if (fp)
                                {
                                    FString f = *fp;
                                    if (f.Compare(TEXT("REPO")) == 0 || f.Compare(TEXT("Repo")) == 0 || f.Compare(TEXT("The Repo")) == 0)
                                    {
                                        NewLine.NPC = 0;
                                    }
                                    else
                                    {
                                        //int i = 1;
                                        if (auto k = UsedSpeakers.Find(f))
                                        {
                                            NewLine.NPC = k;
                                        }
                                        else
                                        {
                                            UsedSpeakers.Add(f);
                                            NewLine.NPC = UsedSpeakers.Num();
                                        }

                                        //NewLine.NPC = 1;
                                    }
                                }
                                else
                                {
                                    NewLine.NPC = 1;
                                }
                            }
                        }

                        auto s3_3 = s2->first_node("MenuText");
                        if (s3_3)
                        {
                            for (xml_node<>* txt = s3_3->first_node("LocalizedString"); txt; txt = txt->next_sibling("LocalizedString"))
                            {
                                if (txt)
                                {
                                    TArray<FString> ParsedArr;
                                    FString cpy = txt->value();
                                    cpy.ParseIntoArray(ParsedArr, TEXT(","));

                                    for (auto& k : ParsedArr)
                                    {
                                        NewLine.DialogueAdditional.Add(k);
                                        UE_LOG(LogTemp, Warning, TEXT("      ParseDialogueFragmentsIntoStruct() - adding DialogueAdditional: %s, num = %d"), *k, NewLine.DialogueAdditional.Num());
                                    }
                                    
                                    
                                }
                            }

                        }

                        auto s3_2 = s2->first_node("StageDirections");
                        if (s3_2)
                        {
                            auto idx = s3_2->first_node("LocalizedString");
                            if (idx)
                            {
                                FString idx_string = idx->value();

                                if (!idx_string.IsEmpty())
                                {
                                    NewLine.IDName = idx->value();
                                    UsedIndices.Add(FCString::Atoi(*idx_string));
                                    ParsedDialogueFragments.FindOrAdd(id) = NewLine;
                                }
                                /*else
                                {
                                    while (UsedIndices.Contains(FallbackIndex))
                                    {
                                        FallbackIndex++;
                                    }
                                    NewLine.IDName = FString::FromInt(FallbackIndex);
                                    UsedIndices.Add(FallbackIndex);
                                    
                                    ParsedDialogueFragments.FindOrAdd(id) = NewLine;
                                }*/
                            }
                            else
                            {
                                while (UsedIndices.Contains(FallbackIndex))
                                {
                                    FallbackIndex++;
                                }
                                NewLine.IDName = FString::FromInt(FallbackIndex);
                                UsedIndices.Add(FallbackIndex);

                                ParsedDialogueFragments.FindOrAdd(id) = NewLine;
                            }

                        }                                                                        
                    }
                }
            }
        }
    }
}

void UArticyToInternalCSVConverter::ParseHubs(const xml_document<>& doc, FString inID)
{
    DebugString.Empty();
    ParsedHubs.Empty();

    xml_node<>* n = doc.first_node();
    if (n)
    {
        xml_node<>* s1 = n->first_node("Content");

        if (s1)
        {
            for (xml_node<>* s2 = s1->first_node("Hub"); s2; s2 = s2->next_sibling("Hub"))
            {
                FString id = s2->first_attribute("Id")->value();

                if (auto fragments = CurrentDialogueFragmentIds.Find(inID))
                {
                    UE_LOG(LogTemp, Warning, TEXT("      ParseHubs() - found hub %s in parsed fragments"), *id);

                    if (fragments->HubIds.Contains(id))
                    {
                        for (xml_node<>* s3 = s2->first_node("Pins"); s3; s3 = s3->next_sibling("Hub"))
                        {
                            for (xml_node<>* s4 = s3->first_node("Pin"); s4; s4 = s4->next_sibling("Hub"))
                            {
                                UE_LOG(LogTemp, Warning, TEXT("      ParseHubs() - %s"), *id);
                                ParsedHubs.Add(id);
                                //ParsedHubs.FindOrAdd(id);//.TargetDialogueFragmentIds.Add(s4->first_attribute("Id")->value());
                            }

                        }
                    }
                }
            }
        }
    }
}

void UArticyToInternalCSVConverter::ParseEntities(const xml_document<>& doc)
{
    DebugString.Empty();
    ParsedHubs.Empty();

    xml_node<>* n = doc.first_node();
    if (n)
    {
        xml_node<>* s1 = n->first_node("Content");

        if (s1)
        {
            for (xml_node<>* s2 = s1->first_node("Entity"); s2; s2 = s2->next_sibling("Entity"))
            {
                FString id = s2->first_attribute("Id")->value();
                auto s3 = s2->first_node("DisplayName");

                if (s3)
                {
                    auto s4 = s3->first_node("LocalizedString");
                    if (s4)
                    {
                        FString name = s4->value();
                        ParsedEntities.Add(id, name);
                    }
                }
            }
        }
    }
}

void UArticyToInternalCSVConverter::ParseConnections(const xml_document<>& doc, FString inID)
{
    DebugString.Empty();
    ParsedConnections.Empty();

    xml_node<>* n = doc.first_node();
    if (n)
    {
        xml_node<>* s1 = n->first_node("Content");

        if (s1)
        {
            for (xml_node<>* s2 = s1->first_node("Connection"); s2; s2 = s2->next_sibling("Connection"))
            {
                FString id = s2->first_attribute("Id")->value();

                if (auto fragments = CurrentDialogueFragmentIds.Find(inID))
                {
                    if (fragments->ConnectionIds.Contains(id))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("      ParseConnections() - found connection %s in parsed fragments"), *id);
                        if (xml_node<>* s3 = s2->first_node("Source"))
                        {
                            ParsedConnections.FindOrAdd(id).Source = s3->first_attribute("IdRef")->value();
                        }

                        if (xml_node<>* s3 = s2->first_node("Target"))
                        {
                            ParsedConnections.FindOrAdd(id).Target = s3->first_attribute("IdRef")->value();
                        }
                    }
                }
            }
        }
    }

    //Main processing

    TSet<FString> SourcesSet;

    for (auto& c : ParsedConnections)
    {
        auto& source = c.Value.Source;
        auto& target = c.Value.Target;

        SourcesSet.Add(source);
        if (auto srchub = ParsedHubs.Find(source))
        {
            UE_LOG(LogTemp, Warning, TEXT("      ParseConnections() - adding target to hub: %s"), *source);
            srchub->TargetDialogueFragmentIds.Add(target);
        }
    }

    for (auto& c : ParsedConnections)
    {
        auto& source = c.Value.Source;
        auto& target = c.Value.Target;

        //SourcesSet.Add(source);

        if (auto src = ParsedDialogueFragments.Find(source))
        {
            if (auto trg = ParsedDialogueFragments.Find(target))
            {
                //dialogue fragment to dialogue fragment
                src->NextIndex = FCString::Atoi(*trg->IDName);
            }
            else if (auto trg2 = ParsedHubs.Find(target))
            {
                src->bAnswers = true;

                for (auto& a : trg2->TargetDialogueFragmentIds)
                {
                    if (auto f = ParsedDialogueFragments.Find(a))
                    {
                        src->AnswersIndexArr.Add(FCString::Atoi(*f->IDName));
                    }
                }
            }
        }
    }

    //identify stubs
    for (auto& d : ParsedDialogueFragments)
    {
        if (SourcesSet.Contains(d.Key))
        {
            d.Value.bEnd = false;
        }
        else
        {
            d.Value.bEnd = true;
        }
    }

}

void UArticyToInternalCSVConverter::PrintParsedDialogueLinesToDebugString()
{
    DebugString.Empty();
    for (auto& d : ParsedDialogueFragments)
    {
        DebugString.Append("LINE: ");
        DebugString.Append(d.Key);
        DebugString.Append(" | ");
        DebugString.Append(d.Value.IDName);
        DebugString.Append("\n");
        DebugString.Append(d.Value.DialogueText.ToString());
        DebugString.Append("\n bEnd =");
        DebugString.Append(d.Value.bEnd ? "True" : "False");
        DebugString.Append(", bAnswers =");
        DebugString.Append(d.Value.bAnswers ? "True, arr:" : "False");
        for (auto& a : d.Value.AnswersIndexArr)
        {
            DebugString.Append(" ");
            DebugString.AppendInt(a);
        }

        if (d.Value.DialogueAdditional.Num() > 0)
        {
            DebugString.Append(", DialogueAdditional =\n");

            for (auto& a : d.Value.DialogueAdditional)
            {
                DebugString.Append("   ");
                DebugString.Append(a);
                DebugString.Append("\n");
            }
        }

        DebugString.Append("\n");


    }

    DebugTextBox->SetText(FText::FromString(DebugString));

}

UDataTable* UArticyToInternalCSVConverter::CreateDataTable(FString inID)
{
    UDataTable* NewTable = NewObject<UDataTable>(GetTransientPackage());
    NewTable->RowStruct = FBigDialogueBody::StaticStruct();

    if (auto d = CurrentDialogueFragmentIds.Find(inID))
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateDataTable() - ID match"));

        for (auto& fragmentId : d->DialogueFragmentIds)
        {
            if (auto fragment = ParsedDialogueFragments.Find(fragmentId))
            {
                UE_LOG(LogTemp, Warning, TEXT("CreateDataTable() - adding row %s, %s"), *fragment->IDName, *fragment->DialogueText.ToString());
                NewTable->AddRow(FName(fragment->IDName), *fragment);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("CreateDataTable() - no %s fragment ID in ParsedDialogueFragments"), *fragmentId);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateDataTable() - no matching ID"));
    }

    FString path = "dummy";
    DebugString.Empty();
    TArray<FBigDialogueBody*> arr;
    NewTable->GetAllRows<FBigDialogueBody>(TEXT(""), arr);

    for (auto& row : arr)
    {
        DebugString.Append(row->DialogueText.ToString() + "\n");
    }

    //test
    /*{
        UDataTable* TestTable = NewObject<UDataTable>(GetTransientPackage());
        TestTable->RowStruct = FBigDialogueBody::StaticStruct();

        auto row = FBigDialogueBody();
        row.IDName = 0;
        row.DialogueText = FText::FromString("test string");
        TestTable->AddRow(FName("0"), row);

        if (TestTable)
        {
            FString str;
            str = TestTable->GetTableAsCSV();
            DebugString = str;
            FFileHelper::SaveStringToFile(str, *(FPaths::ProjectContentDir() + TEXT("/Dialogue/converted/") + inID));
        }
    }*/

    /*if (NewTable)
    {
        FString str;
        str = NewTable->GetTableAsCSV();
        DebugString = str;
        FFileHelper::SaveStringToFile(str, *(FPaths::ProjectContentDir() + TEXT("/Dialogue/converted/") + inID + TEXT(".csv")));
    }*/

    return NewTable;

    //SaveDataTable(NewTable, path);
}

void UArticyToInternalCSVConverter::SaveDataTables()
{
    TArray<FString> arr;

    for (auto& t : ResultTables)
    {
        //CreateDataTable(d.Value);
        SaveDataTable_Internal(t.Value, t.Key);
    }
}

void UArticyToInternalCSVConverter::SaveDataTable(FString inName)
{
    if (auto ID = CurrentDialogueMap.Find(inName))
    {
        if (auto t = ResultTables.Find(*ID))
        {
            SaveDataTable_Internal(*ResultTables.Find(*ID), inName);
        }
    }
}

void UArticyToInternalCSVConverter::SaveDataTable_Internal(UDataTable* inTable, FString inPath)
{
    //// Load necessary modules
    //FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
    //FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    //FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    //IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    //// Generate a unique asset name
    //FString Name, PackageName;
    //AssetToolsModule.Get().CreateUniqueAssetName(TEXT("/Game/Dialogue/converted/")+inPath, TEXT(""), PackageName, Name);
    //const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

    //// Create object and package
    //UPackage* package = CreatePackage(*PackageName);
    ////UBlueprintFactory* MyFactory = NewObject<UBlueprintFactory>();
    //UReimportDataTableFactory* MyFactory = NewObject<UReimportDataTableFactory>(UReimportDataTableFactory::StaticClass()); // Can omit, and a default factory will be used
    ////UObject* NewObject = inTable;
    //UObject* NewObject = AssetToolsModule.Get().CreateAsset(Name, PackagePath, UDataTable::StaticClass(), MyFactory);

    //if (auto DataTable = Cast<UDataTable>(NewObject))
    //{
    //    DataTable->CreateTableFromOtherTable(inTable);
    //}

    //FSavePackageArgs SaveArgs = FSavePackageArgs();
    //UPackage::Save(package, NewObject, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);

    //// Inform asset registry
    //AssetRegistry.AssetCreated(NewObject);

    //// Tell content browser to show the newly created asset
    //TArray<UObject*> Objects;
    //Objects.Add(NewObject);
    //ContentBrowserModule.Get().SyncBrowserToAssets(Objects);

    /*if (inTable)
    {
        auto str = inTable->GetTableAsString();
        FFileHelper::SaveStringToFile(str, *(FPaths::ProjectContentDir() + TEXT("/Dialogue/converted/") + inPath));
    }*/

    if (inTable)
    {
        FString str;
        str = inTable->GetTableAsCSV();
        DebugString = str;
        FFileHelper::SaveStringToFile(str, *(FPaths::ProjectContentDir() + TEXT("/DialogueData/converted/") + inPath + TEXT(".csv")));
    }

    
}
