#include "StringTableGeneratorModule.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "StringTableGenerator.h"

#define LOCTEXT_NAMESPACE "FStringTableGenerator"

void FStringTableGeneratorModule::StartupModule()
{
	//Extend the DataTable editor toolbar
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("AssetEditor.DataTableEditor.Toolbar");
	FToolMenuSection& Section = Menu->FindOrAddSection("CrisisUnit");

	Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([this](FToolMenuSection& Section)
		{
			UAssetEditorToolkitMenuContext* Context = Section.FindContext<UAssetEditorToolkitMenuContext>();

			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				NAME_None,
				FUIAction(FExecuteAction::CreateLambda([Context]()
					{
						const TArray<UObject*>& Objects = Context->GetEditingObjects();
						if (Objects.IsEmpty())
						{
							UE_LOG(LogTemp, Error, TEXT("No object found"));
							return;
						}

						if (UDataTable* DT = Cast<UDataTable>(Objects[0]))
						{
							UStringTableGenerator::OpenSettingsPopup(DT);
						}
					})),
				TAttribute<FText>(FText::FromString(TEXT("String Table Generation"))),
				TAttribute<FText>(),
				FSlateIcon()
			));
		}));
}

void FStringTableGeneratorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStringTableGeneratorModule, StringTableGenerator)