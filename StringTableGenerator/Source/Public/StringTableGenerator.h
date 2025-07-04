#pragma once

#include "CoreMinimal.h"

#include "StringTableGenerator.generated.h"

class UDataTable;

/**
 * Generator to add rows into a String Table based on a DataTable's data
 */
UCLASS(Abstract)
class STRINGTABLEGENERATOR_API UStringTableGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** Opens a popup to configure string table generation **/
	static void OpenSettingsPopup(UDataTable* DT);

	/** Main function to generate the content of a string table based on the Text properties of a DataTable **/
	static void GenerateStringTableContent(UStringTableGenerationSettings* Settings);

	/** Make sure the generation settings are correct before strating the generation **/
	static bool ValidateGenerationSettings(UStringTableGenerationSettings* Settings);

private:
	/** Returns the Text Property corresponding to the given chain path **/
	static FTextProperty* GetPropertyFromPropertyPath(const UScriptStruct* BaseStruct, FString& ChainPath);

	/** Create a new string table with the given name **/
	static UStringTable* CreateStringTable(FString& TableName);

	/** Returns the list of properties that need to be saved for the given row **/
	static void GetPropertiesToSave(FTableRowBase* DataTableRow, const UScriptStruct* DtStruct, TArray<FString> PropertyPaths, TArray<FText*>& OutTextsToSave);

	/** Return the list of FText matching the propertypath **/
	static TArray<FText*> GetPropertyValuesFromPropertyPath(FTableRowBase* DataTableRow, const UScriptStruct* DtStruct, FString PropertyPath);

	/** Returns the list of the variable addresses that correspond to the property path given into the given object 
	*	@StartingObjects: The address of the object we want to look into
	*	@DataStruct: The reference to the structure of the Starting object
	*	@PropertyPath: The Path to the Property we are looking for
	**/
	static void GetPropertyAddresses(void* StartingObjects, const UScriptStruct* DataStruct, FString PropertyPath, TMap<void*, FTextProperty*>& OutAddresses);

	/** Loop through all the data of the string table and return the key of the string matching the given value or nullptr if not found **/
	static FString FindValueInStringTable(UStringTable* Table, const FString& Value);

	/** Create a unique key based on the DataTable row name, property name, prefix and suffix **/
	static FString CreateNewKeyName(const FString& RowName, UStringTable* DstStringtable, UStringTableGenerationSettings* Settings);

	/** Open or Reopen the editor for the given string table to refresh it's content after the generation **/
	static void RefreshStringTableEditor(UStringTable* StringTable);
};

/*
* Settings for the string table generation
*/
UCLASS()
class UStringTableGenerationSettings : public UObject
{
	GENERATED_BODY()

public:
	/** The DataTable used to generate the String Table **/
	UDataTable* SourceDataTable;

	/** Should we save all the text properties or only PropertyName **/
	UPROPERTY(EditAnywhere)
	bool bSaveAllProperties = false;
	
	/** The Property we want to save into the string table, visible only if bSaveAllProperties is false **/
	UPROPERTY(EditAnywhere)
	FString PropertyName;

	/** List of all the Text property names for the current DataTable struct **/
	TArray<FString> AllPropertyNames;

	/** Should we create a new string table **/
	UPROPERTY(EditAnywhere)
	bool bCreateNewStringTable = false;

	/** The name of the new string table, visible only if bCreateNewStringTable is true **/
	UPROPERTY(EditAnywhere)
	FString NewStringTableName;

	/** The existing string table to use, visible only if bCreateNewStringTable is false **/
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStringTable> ExistingStringTable;

	/** The prefix to add to the new string table keys **/
	UPROPERTY(EditAnywhere)
	FString StringIdPrefix;

	/** The suffix to add to the new string table keys **/
	UPROPERTY(EditAnywhere)
	FString StringIdSuffix;

	/** Should copy the string table keys of the texts that are already localized into the current string table **/
	UPROPERTY(EditAnywhere)
	bool bCopyAlreadyLocalizedKeys;
};
