#include "StringTableGenerator.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "SStringTableGeneratorSettings.h"
#include "AssetToolsModule.h"
#include "Factories/StringTableFactory.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "StringTableEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/Text.h"

void UStringTableGenerator::OpenSettingsPopup(UDataTable* DT)
{
	if (DT == nullptr)
	{
		return;
	}

	//Create window
	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("String Table Generation Settings")))
		.SizingRule(ESizingRule::FixedSize)
		.ClientSize(FVector2D(450.0, 300.0))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TSharedPtr<SStringTableGeneratorSettings> WindowContent = SNew(SStringTableGeneratorSettings)
		.ParentWindow(Window)
		.SourceDataTable(DT);

	//Set window content
	Window->SetContent(WindowContent.ToSharedRef());

	//Show window
	GEditor->EditorAddModalWindow(Window.ToSharedRef());
}

void UStringTableGenerator::GenerateStringTableContent(UStringTableGenerationSettings* Settings)
{
	//Create or load string
	UStringTable* StringTable;
	if (Settings->bCreateNewStringTable)
	{
		StringTable = CreateStringTable(Settings->NewStringTableName);
	}
	else
	{
		StringTable = Settings->ExistingStringTable.Get();
	}

	if (StringTable == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to create or load String Table.")));
		return;
	}

	//Get DataTable rows
	TArray<FName> TableRowsName = Settings->SourceDataTable->GetRowNames();

	const UScriptStruct* DtStruct = Settings->SourceDataTable->GetRowStruct();
	//Loop over rows
	for (FName& RowName : TableRowsName)
	{
		FTableRowBase* TableRow = Settings->SourceDataTable->FindRow<FTableRowBase>(RowName, TEXT("String Table Generation"));
		if (TableRow == nullptr)
		{
			continue;
		}

		//Get the properties to save
		TArray<FText*> PropertiesToSave;
		if (Settings->bSaveAllProperties)
		{
			GetPropertiesToSave(TableRow, DtStruct, Settings->AllPropertyNames, PropertiesToSave);
		}
		else
		{
			GetPropertiesToSave(TableRow, DtStruct,{ Settings->PropertyName }, PropertiesToSave);
		}

		for (FText* Text : PropertiesToSave)
		{
			//Check if the property is already localized
			FString ExistingKey; 
			FName TableID;
			if (FTextInspector::GetTableIdAndKey(*Text, TableID, ExistingKey))
			{
				//The localize string is already in the correct String Table
				if (!Settings->bCopyAlreadyLocalizedKeys || TableID == StringTable->GetStringTableId())
				{
					continue;
				}

				//Copy the Key into the new String Table and link the text to the new string table
				StringTable->GetMutableStringTable()->SetSourceString(ExistingKey, Text->ToString());
				*Text = FText::FromStringTable(StringTable->GetStringTableId(), ExistingKey);
			}
			else //Property not localized
			{
				//Check if a string exists with the same text, if not we create a new one
				FString Key = FindValueInStringTable(StringTable, Text->ToString());
				if (Key.IsEmpty())
				{
					//Create a new string table entry for this Text
					Key = CreateNewKeyName(RowName.ToString(), StringTable, Settings);
					StringTable->GetMutableStringTable()->SetSourceString(Key, Text->ToString());
				}

				//Link the text to the matching key from the string table
				*Text = FText::FromStringTable(StringTable->GetStringTableId(), Key);
			}
		}
	}

	RefreshStringTableEditor(StringTable);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("String Table Generation Completed!")));
}

bool UStringTableGenerator::ValidateGenerationSettings(UStringTableGenerationSettings* Settings)
{
	bool bIsValid = true;
	FString InvalidSettingsMsg = TEXT("Invalid settings, see issues below:\n\n");
	
	//Property name input is correct
	if (!Settings->bSaveAllProperties)
	{
		if (Settings->PropertyName.IsEmpty())
		{
			InvalidSettingsMsg.Append(TEXT("- Property name shouldn't be empty\n"));
			bIsValid = false;
		}
		else
		{
			//Check that there's a property with this name is the DataTable's data structure
			const UScriptStruct* DTStruct = Settings->SourceDataTable->GetRowStruct();
			if (DTStruct->FindPropertyByName(FName(Settings->PropertyName)) == nullptr)
			{
				InvalidSettingsMsg.Append(TEXT("- No property found with the given Property Name.\n"));
			}
		}
	}
	
	//Check new asset name
	if (Settings->bCreateNewStringTable)
	{
		if (Settings->NewStringTableName.IsEmpty())
		{
			InvalidSettingsMsg.Append(TEXT("- New String Table name shouldn't be empty\n"));
			bIsValid = false;
		}
		else
		{
			//Check that the name is valid to create an asset
			FString PackagePath = TEXT("/Game/CrisisUnit/StringTables/" + Settings->NewStringTableName);
			if (!FPackageName::IsValidObjectPath(PackagePath))
			{
				InvalidSettingsMsg.Append(TEXT("- New String Table name is not a valid asset name. It shouldn't contain any whitespace or special characters\n"));
				bIsValid = false;
			}
		}
	}
	else //Use existing String Table
	{
		if (Settings->ExistingStringTable == nullptr)
		{
			InvalidSettingsMsg.Append(TEXT("- Existing String Table not selected\n"));
			bIsValid = false;
		}
	}

	//Prefix or suffix as to be filled
	if (Settings->StringIdPrefix.IsEmpty() && Settings->StringIdSuffix.IsEmpty())
	{
		InvalidSettingsMsg.Append(TEXT("- Prefix and Suffix can't be both empty.\n"));
		bIsValid = false;
	}

	if (!bIsValid)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(InvalidSettingsMsg));
	}

	return bIsValid;
}

FTextProperty* UStringTableGenerator::GetPropertyFromPropertyPath(const UScriptStruct* BaseStruct, FString& PropertyPath)
{
	TArray<FString> PropertyNames;
	PropertyPath.ParseIntoArray(PropertyNames, TEXT("."));

	const UScriptStruct* CurrentStruct = BaseStruct;
	FProperty* CurrentProperty = nullptr;
	for (FString PropertyName : PropertyNames)
	{
		CurrentProperty = CurrentStruct->FindPropertyByName(FName(PropertyName));
		
		if (FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			CurrentStruct = StructProperty->Struct;
		}
		else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(CurrentProperty))
		{
			CurrentProperty = ArrayProperty->Inner;

			if (FStructProperty* ArrayStructProperty = CastField<FStructProperty>(CurrentProperty))
			{
				CurrentStruct = ArrayStructProperty->Struct;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find Property with path: %s"), *PropertyPath);
		}
	}

	if (FTextProperty* TextProperty = CastField<FTextProperty>(CurrentProperty))
	{
		return TextProperty;
	}

	return nullptr;
}

UStringTable* UStringTableGenerator::CreateStringTable(FString& TableName)
{
	//Try to load the asset in case it already exist
	FString AssetPath = TEXT("/Game/CrisisUnit/StringTables/" + TableName + "." + TableName);
	UClass* AssetClass = UStringTable::StaticClass();

	UObject* LoadedAsset = StaticLoadObject(AssetClass, nullptr, *AssetPath);

	if (LoadedAsset)
	{
		return Cast<UStringTable>(LoadedAsset);
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// Create the factory for the string table
	UStringTableFactory* StringTableFactory = NewObject<UStringTableFactory>();
	StringTableFactory->AddToRoot(); // Ensure factory does not get GC'd during this operation

	// Create the asset
	UObject* CreatedAsset = AssetTools.CreateAsset(TableName, TEXT("/Game/CrisisUnit/StringTables/"), UStringTable::StaticClass(), StringTableFactory);

	StringTableFactory->RemoveFromRoot();

	return Cast<UStringTable>(CreatedAsset);
}

void UStringTableGenerator::GetPropertiesToSave(FTableRowBase* DataTableRow, const UScriptStruct* DtStruct, TArray<FString> PropertyPaths, TArray<FText*>& OutTextsToSave)
{
	for (FString& PropertyPath : PropertyPaths)
	{
		for (FText* TextValue : GetPropertyValuesFromPropertyPath(DataTableRow, DtStruct, PropertyPath))
		{
			if (TextValue != nullptr)
			{
				OutTextsToSave.Add(TextValue);
			}
		}
	}
}

TArray<FText*> UStringTableGenerator::GetPropertyValuesFromPropertyPath(FTableRowBase* DataTableRow, const UScriptStruct* DtStruct, FString PropertyPath)
{
	//Get the address of all the properties we want to edit
	TMap<void*, FTextProperty*> PropertiesAddresses;
	GetPropertyAddresses(DataTableRow, DtStruct, PropertyPath, PropertiesAddresses);

	//Get the values from the addresses
	TArray<FText*> Texts;
	for (TPair<void*, FTextProperty*> PropertyAddress : PropertiesAddresses)
	{
		if (FTextProperty* TextProperty = CastField<FTextProperty>(PropertyAddress.Value))
		{
			FText* Text = TextProperty->GetPropertyValuePtr(PropertyAddress.Key);
			if (Text != nullptr && !Text->IsEmpty())
			{
				Texts.Add(Text);
			}
		}
	}

	return Texts;
}

void UStringTableGenerator::GetPropertyAddresses(void* StartingObjects, const UScriptStruct* DataStruct, FString PropertyPath, TMap<void*, FTextProperty*>& OutAddresses)
{
	//Split the property path string to get the name of each property
	TArray<FString> PropertyNames;
	PropertyPath.ParseIntoArray(PropertyNames, TEXT("."));

	//Initialize variables
	const UScriptStruct* CurrentStruct = DataStruct;
	FProperty* CurrentProperty = nullptr;
	void* CurrentAddress = StartingObjects;
	FString CurrentPropertyPath = "";

	//Loop over each property to get to the bottom of the PropertyPath and retrieve the address of each text
	for (FString PropertyName : PropertyNames)
	{
		CurrentProperty = CurrentStruct->FindPropertyByName(FName(PropertyName));
		CurrentPropertyPath = CurrentPropertyPath.IsEmpty() ? PropertyName : CurrentPropertyPath + "." + PropertyName;

		//It's a text we add the address of the object to get the value later
		if (FTextProperty* TextProperty = CastField<FTextProperty>(CurrentProperty))
		{
			void* TextAddress = TextProperty->ContainerPtrToValuePtr<void>(CurrentAddress);
			OutAddresses.Add(TextAddress, TextProperty);
		}
		//It's a struct we get the address and the struct for this property
		else if (FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			CurrentAddress = StructProperty->ContainerPtrToValuePtr<void>(CurrentAddress);
			CurrentStruct = StructProperty->Struct;
		}
		//If it's an array, loop though all the items and get the requested property for each of them
		else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(CurrentProperty))
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(CurrentAddress));
			CurrentProperty = ArrayProperty->Inner;

			//It's an array of FText we add all of them to the address array
			if (FTextProperty* ArrayTextProperty = CastField<FTextProperty>(CurrentProperty))
			{
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					void* ItemAddress = ArrayHelper.GetRawPtr(i);
					OutAddresses.Add(ItemAddress, ArrayTextProperty);
				}
			}
			//It's an array of structs, go through each of them to get the requested property
			else if (FStructProperty* ArrayStructProperty = CastField<FStructProperty>(CurrentProperty))
			{
				CurrentStruct = ArrayStructProperty->Struct;

				//Loop through array items
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					FString CutPropertyPath = PropertyPath;
					CutPropertyPath.RemoveFromStart(CurrentPropertyPath);
					CutPropertyPath.RemoveAt(0);
					
					void* ItemAddress = ArrayHelper.GetRawPtr(i);
					GetPropertyAddresses(ItemAddress, CurrentStruct, CutPropertyPath, OutAddresses);
				}

				//The recursive call went through the rest of the property chain, stop here
				break;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find Property with path: %s"), *PropertyPath);
		}
	}
}

FString UStringTableGenerator::FindValueInStringTable(UStringTable* Table, const FString& Value)
{
	FString FoundKey = "";
	Table->GetStringTable()->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString)
	{
		if (InSourceString == Value)
		{
			FoundKey = InKey;
			return false;
		}

		return true; // continue enumeration
	});

	return FoundKey;
}

FString UStringTableGenerator::CreateNewKeyName(const FString& RowName, UStringTable* DstStringtable, UStringTableGenerationSettings* Settings)
{
	FString Prefix = Settings->StringIdPrefix.IsEmpty() ? "" : Settings->StringIdPrefix + "_";
	FString Suffix = Settings->StringIdSuffix.IsEmpty() ? "" : "_" + Settings->StringIdSuffix;
	
	TArray<FString> Parts;
	Settings->PropertyName.ParseIntoArray(Parts, TEXT("."), true);
	if (Parts.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse Property name"));
		return "";
	}

	FString PropertyName = Parts.Last();
	FString Key = Prefix + RowName + "_" + PropertyName + Suffix;

	//Check if the key already exist, if it does we add an incrementing number to the name
	FString TempString;
	int32 Counter = 0;
	while (DstStringtable->GetStringTable()->GetSourceString(Key, TempString))
	{
		Counter++;
		Key = Prefix + RowName + "_" + PropertyName + FString::FromInt(Counter) + Suffix;
	}

	return Key;
}

void UStringTableGenerator::RefreshStringTableEditor(UStringTable* StringTable)
{
	UAssetEditorSubsystem* AssetEditorSubSystem = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
	if (!AssetEditorSubSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find AssetEditorSubsystem"));
		return;
	}

	//Close any opened editor
	AssetEditorSubSystem->CloseAllEditorsForAsset(StringTable);

	//Open the editor
	FStringTableEditorModule& StringTableEditorModule = FModuleManager::LoadModuleChecked<FStringTableEditorModule>("StringTableEditor");
	StringTableEditorModule.CreateStringTableEditor(EToolkitMode::Standalone, nullptr, StringTable);
}
