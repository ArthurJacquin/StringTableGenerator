#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "StringTableGenerator.h"

class UStringTable;

DECLARE_DELEGATE_OneParam(FGenerateEvent, UStringTableGenerationSettings&);

/**
 * Slate widget for String Table Generator Settings
 */
class STRINGTABLEGENERATOR_API SStringTableGeneratorSettings : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStringTableGeneratorSettings)
		:_ParentWindow(nullptr),
		_SourceDataTable(nullptr)
		{
		}

		/** The popup window this widget is in */
		SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)

		/** Called when the button OK button is clicked */
		SLATE_EVENT(FGenerateEvent, OnGenerateClicked)

		/** The DataTable used to generate the String Table **/
		SLATE_ARGUMENT(UDataTable*, SourceDataTable)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Add the new node to the graph */
	FReply OnGenerateButtonClicked();

	/** Close the window */
	FReply OnCancelButtonClicked();

	/** Called when we select another Property Name **/
	void OnPropertyNameSelectionChanged(TSharedPtr<FString> SelectedName, ESelectInfo::Type SelectionType);

	/** Make a combo box widget element for the Property Name **/
	TSharedRef<SWidget> MakePropertyNameComboWidget(TSharedPtr<FString> InItem);

	/** Conditions to show widgets **/
	EVisibility ShowPropertyNames() const;
	EVisibility ShowCreateNewStringTable() const;
	EVisibility ShowExistingStringTable() const;

	/** Get the currently selected property name **/
	FText GetPropertyNameComboBoxContent() const;

	/** Retrieve all the text properties from the current DataTable struct **/
	void GetAllTextPropertiesRecursive(const UScriptStruct* Struct, FString PropertyPath, TArray<FString>& OutTextProperties) const;

	/** The Popup window this widget is in */
	TSharedPtr<SWindow> ParentWindow;

	/** Delegate for the OK button event */
	FGenerateEvent OnGenerateClicked;

	/** The Data set by the widget **/
	UStringTableGenerationSettings* Settings;

	/** The name of all the text properties of the source structure **/
	TArray<TSharedPtr<FString>> PropertyNames;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> PropertyNameComboBox;
};
