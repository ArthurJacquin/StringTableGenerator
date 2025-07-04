#include "SStringTableGeneratorSettings.h"
#include "PropertyEditorModule.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "ISinglePropertyView.h"
#include "Widgets/Input/SComboBox.h"

void SStringTableGeneratorSettings::Construct(const FArguments& InArgs)
{
	ParentWindow = InArgs._ParentWindow;
	OnGenerateClicked = InArgs._OnGenerateClicked;

	Settings = NewObject<UStringTableGenerationSettings>();
	Settings->SourceDataTable = InArgs._SourceDataTable;

	//Get the names of all the properties that are a text
	const UScriptStruct* DTStruct = InArgs._SourceDataTable->GetRowStruct();
	TArray<FString> TextProperties;
	GetAllTextPropertiesRecursive(DTStruct, "", TextProperties);

	for (FString Text : TextProperties)
	{
		PropertyNames.Add(MakeShared<FString>(Text));
		Settings->AllPropertyNames.Add(Text);
	}

	//Combo box to select a property name
	SAssignNew(PropertyNameComboBox, SComboBox< TSharedPtr<FString>>)
		.OptionsSource(&PropertyNames)
		.OnGenerateWidget(this, &SStringTableGeneratorSettings::MakePropertyNameComboWidget)
		.OnSelectionChanged(this, &SStringTableGeneratorSettings::OnPropertyNameSelectionChanged)
		.Content()
		[
			SNew(STextBlock)
				.Text(this, &SStringTableGeneratorSettings::GetPropertyNameComboBoxContent)
		];

	//Set selected property name
	if (PropertyNames.Num() > 0)
	{
		PropertyNameComboBox->SetSelectedItem(PropertyNames[0]);
	}

	//Create property views
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<ISinglePropertyView> SaveAllPropertiesView = PropertyModule.CreateSingleProperty(Settings, TEXT("bSaveAllProperties"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> NewStringTableNameView = PropertyModule.CreateSingleProperty(Settings, TEXT("NewStringTableName"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> CreateNewStringTableView = PropertyModule.CreateSingleProperty(Settings, TEXT("bCreateNewStringTable"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> ExistingStringTableView = PropertyModule.CreateSingleProperty(Settings, TEXT("ExistingStringTable"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> StringIdPrefixView = PropertyModule.CreateSingleProperty(Settings, TEXT("StringIdPrefix"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> StringIdSuffixView = PropertyModule.CreateSingleProperty(Settings, TEXT("StringIdSuffix"), FSinglePropertyParams());
	TSharedPtr<ISinglePropertyView> IncludeLocalizedTextsView = PropertyModule.CreateSingleProperty(Settings, TEXT("bCopyAlreadyLocalizedKeys"), FSinglePropertyParams());

	//Create buttons Generate and Cancel
	TSharedPtr<SUniformGridPanel> ButtonsGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(10.f, 10.f, 10.f, 10.f))
		.MinDesiredSlotWidth(FAppStyle::Get().GetFloat("StandardDialog.MinDesiredSlotWidth"))
		.MinDesiredSlotHeight(FAppStyle::Get().GetFloat("StandardDialog.MinDesiredSlotHeight"));

	//OK Button
	ButtonsGrid->AddSlot(0, 0)
	[
		SNew(SButton)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString(TEXT("Generate")))
		.OnClicked(this, &SStringTableGeneratorSettings::OnGenerateButtonClicked)
	];

	//Cancel Button
	ButtonsGrid->AddSlot(1, 0)
	[
		SNew(SButton)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString(TEXT("Cancel")))
		.OnClicked(this, &SStringTableGeneratorSettings::OnCancelButtonClicked)
	];

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				CreateNewStringTableView.ToSharedRef()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.Visibility(this, &SStringTableGeneratorSettings::ShowCreateNewStringTable)
				[
					NewStringTableNameView.ToSharedRef()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.Visibility(this, &SStringTableGeneratorSettings::ShowExistingStringTable)
				[
					ExistingStringTableView.ToSharedRef()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				SaveAllPropertiesView.ToSharedRef()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				//Property Names
				SNew(SHorizontalBox)
				.Visibility(this, &SStringTableGeneratorSettings::ShowPropertyNames)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0, 0.0, 0.0, 0.0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Property Name")))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(0.0, 0.0, 20.0, 0.0)
				[
					PropertyNameComboBox.ToSharedRef()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				IncludeLocalizedTextsView.ToSharedRef()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0, 0.0, 0.0, 5.0)
			[
				StringIdPrefixView.ToSharedRef()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				StringIdSuffixView.ToSharedRef()
			]
		]

		//Buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			ButtonsGrid.ToSharedRef()
		]
	];
}

FReply SStringTableGeneratorSettings::OnGenerateButtonClicked()
{
	if (!UStringTableGenerator::ValidateGenerationSettings(Settings))
	{
		return FReply::Handled();
	}

	UStringTableGenerator::GenerateStringTableContent(Settings);

	ParentWindow->RequestDestroyWindow();
	return FReply::Handled();
}

FReply SStringTableGeneratorSettings::OnCancelButtonClicked()
{
	ParentWindow->RequestDestroyWindow();
	return FReply::Handled();
}

void SStringTableGeneratorSettings::OnPropertyNameSelectionChanged(TSharedPtr<FString> SelectedName, ESelectInfo::Type SelectionType)
{
	Settings->PropertyName = *SelectedName;
}

TSharedRef<SWidget> SStringTableGeneratorSettings::MakePropertyNameComboWidget(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem));
}

EVisibility SStringTableGeneratorSettings::ShowPropertyNames() const
{
	return Settings->bSaveAllProperties ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SStringTableGeneratorSettings::ShowCreateNewStringTable() const
{
	return Settings->bCreateNewStringTable ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SStringTableGeneratorSettings::ShowExistingStringTable() const
{
	return !Settings->bCreateNewStringTable ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SStringTableGeneratorSettings::GetPropertyNameComboBoxContent() const
{
	if (PropertyNames.Num() > 0)
	{
		return FText::FromString(*PropertyNameComboBox->GetSelectedItem().Get());
	}

	return FText();
}

void SStringTableGeneratorSettings::GetAllTextPropertiesRecursive(const UScriptStruct* Struct, FString ChainString, TArray<FString>& OutTextProperties) const
{
	FString DotChar = ChainString.IsEmpty() ? "" : ".";
	// Iterate all properties of the struct
	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		FProperty* Property = *It;
		FString CurrentString = ChainString + DotChar + Property->GetName();

		// If this property is a text property, add it to the output array
		if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
		{
			OutTextProperties.Add(CurrentString);
		}
		// If this property is a struct property, recursively search in it
		else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			GetAllTextPropertiesRecursive(StructProperty->Struct, CurrentString, OutTextProperties);
		}
		else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			FProperty* InnerProperty = ArrayProperty->Inner;

			// If this property is a text property, add it to the output array
			if (FTextProperty* ArrayTextProperty = CastField<FTextProperty>(InnerProperty))
			{
				OutTextProperties.Add(CurrentString);
			}
			// If this property is a struct property, recursively search in it
			else if (FStructProperty* ArrayStructProperty = CastField<FStructProperty>(InnerProperty))
			{
				GetAllTextPropertiesRecursive(ArrayStructProperty->Struct, CurrentString, OutTextProperties);
			}
		}
	}

}
