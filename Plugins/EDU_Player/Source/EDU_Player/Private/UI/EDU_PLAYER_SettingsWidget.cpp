// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/EDU_CORE_SettingsWidget.h"

#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Framework/UI/EDU_CORE_SelectionBase.h"
#include "Framework/Data/FlowLog.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------
void UEDU_CORE_SettingsWidget::NativeConstruct()
{ FLOW_LOG
	//Super::NativeConstruct();

	// Let's assign that nullptr
	GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameUserSettings) { FLOW_LOG_ERROR("GameUserSettings is nullptr.") return; }

	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();
	InitializeQualitySettings();
	//InitializeShadingQualityOptions(); <- Deprecated, use InitializeQualitySettings()
	
	// Bind our Auto Detect Quality Button to functionality
	if (Button_AutoDetect){ Button_AutoDetect->OnClicked.AddDynamic(this, &ThisClass::AutoDetectQuality); }
	
}

UWidget* UEDU_CORE_SettingsWidget::NativeGetDesiredFocusTarget() const
{ FLOW_LOG
	// Will focus on this when the widget is activated <!> 
	return ResolutionComboBox;
}

void UEDU_CORE_SettingsWidget::InitializeResolutionComboBox()
{ FLOW_LOG
	if (!ResolutionComboBox) { FLOW_LOG_ERROR("ResolutionComboBox is nullptr.") return; }
	
	// Reset is the same as empty, but doesn't change memory allocations, unless the new size is larger than the current array.
	ResolutionsArray.Reset();

	// Clear the UI box
	ResolutionComboBox->ClearOptions();

	// Get a list of support fullscreen resolutions and fill the ResolutionsArray with it.
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(ResolutionsArray);
	
	// The ComboBox holds strings, so we need toTurn each FIntPoint struct in ResolutionsArray into a string,
	// and put the string into the ComboBox.
	for (const FIntPoint& Resolution : ResolutionsArray)
	{
		const auto ResolutionString = FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
		ResolutionComboBox->AddOption(ResolutionString);
	}
	
	/* -------------------- IndexOfByPredicate -------------------------
	  We want to find out CurrentResolution and set it in the combobox.

	  Our Combobox holds an array, and we want to display the index of
	  array to that which corresponds to our resolution, so we'll look
	  for the first index that matches CurrentResolution and return it.	  
	-------------------------------------------------------------------*/
	// Find the current resolution
	const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	
	// Return Index to the first matching element to &CurrentResolution, or INDEX_NONE if none is found.
	const int32 SelectedIndex = ResolutionsArray.IndexOfByPredicate 
	( // Lambda
		[&CurrentResolution](const FIntPoint& InResolution)
		{
			return InResolution == CurrentResolution;
		}
	);
	
	if(SelectedIndex < 0){ FLOW_LOG_ERROR("SelectedIndex is less than 0?") return;}
	ResolutionComboBox->SetSelectedIndex(SelectedIndex);

	// OnSelectionChanged is Called when a new item is selected in the combobox
	ResolutionComboBox->OnSelectionChanged.Clear();
	ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnResolutionChanged);
} 

void UEDU_CORE_SettingsWidget::InitializeVSync()
{ FLOW_LOG
	if(!VSyncCheckBox) { FLOW_LOG_ERROR("VSyncCheckBox is nullptr.")return; }
	
	VSyncCheckBox->SetIsChecked(GameUserSettings->IsVSyncEnabled());
	VSyncCheckBox->OnCheckStateChanged.Clear();
	VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnVSyncChanged);
} 

//------------------------------------------------------------------------------
// Anonymous namespace, only used by this .cpp file
//------------------------------------------------------------------------------
namespace 
{ 
	// constexpr allows us to define expressions that can be evaluated at compile time.
	// This capability provides several benefits, such as improved performance and more efficient use of memory.
	
	//------------------------------------------------------------------------------
	// Used by InitializeFramerate()
	//------------------------------------------------------------------------------
	constexpr EFramerate FramerateOptionsArray[] = {
		EFramerate::FPS_30,
		EFramerate::FPS_45,
		EFramerate::FPS_60,
		EFramerate::FPS_75,
		EFramerate::FPS_120,
		EFramerate::FPS_240,
		EFramerate::FPS_Uncapped,
	};

	//------------------------------------------------------------------------------
	// Used by InitializeQualitySettings()
	//------------------------------------------------------------------------------
	constexpr EQuality QualityOptionsArray[] = {
		EQuality::Lowest,
		EQuality::Low,
		EQuality::Medium,
		EQuality::High,
		EQuality::Highest,
	};
	
	// Create a type alias for a member function pointer in UGameUserSettings that returns an int32 and is const-qualified.
	typedef int32 (UGameUserSettings::*GetQuality)() const;

	// Create a type alias for a member function pointer in UGameUserSettings that takes an int as an argument and returns void.
	typedef void (UGameUserSettings::*SetQuality)(int);

	// Define a structure to hold UI widget and associated get/set functions.
	struct FSelectionElement
	{
		UEDU_CORE_SelectionBase* SelectionWidget;

		// Member function pointer to a getter function in UGameUserSettings.
		GetQuality GetQuality;

		// Member function pointer to a setter function in UGameUserSettings.
		SetQuality SetQuality;
	};
}

void UEDU_CORE_SettingsWidget::InitializeFramerate() const
{ FLOW_LOG
	if (!FramerateSelectionWidget) { FLOW_LOG_ERROR("FramerateSelectionWidget is nullptr.") return;	}
	FramerateSelectionWidget->ClearCurrentSelection();

	int FramerateOptionIndex = 0;
	const auto CurrentFramerate = GameUserSettings->GetFrameRateLimit();

	// Add available options to the widget labels
	for (const auto& Framerate : FramerateOptionsArray)
	{
		FramerateSelectionWidget->AddOption({
			// AddOption expect a struct&, so we wrap this class in {}, converting it to a struct
			FText::FromString(FEnumUtilities::FramerateToString(Framerate))
		});

		if(CurrentFramerate == FEnumUtilities::EFramerateToValue(Framerate))
		{
			FramerateSelectionWidget->SetCurrentSelection(FramerateOptionIndex);
		}
		
		FramerateOptionIndex++ ;
	}

	FramerateSelectionWidget->OnSelectionChange.BindLambda([this](const int InSelection)
		{ // Lambda Scope
			GameUserSettings->SetFrameRateLimit(FEnumUtilities::EFramerateToValue(FramerateOptionsArray[InSelection]));
			GameUserSettings->ApplySettings(false);
		}
	); // end params
}

/*-------------------InitializeShadingQualityOptions() ------------------------
  Deprecated, we use the Quality Settings loop instead
--------------------------------------------------------------------------------
void UEDU_CORE_SettingsWidget::InitializeShadingQualityOptions() 
{ FLOW_LOG
	if(!ShadingQualitySelectionWidget) { FLOW_LOG_ERROR("ShadingQualitySelectionWidget is null") return; }
	ShadingQualitySelectionWidget->ClearCurrentSelection();

	int QualityOptionIndex = 0;
	const auto CurrentQuality = GameUserSettings->GetShadingQuality();
	ShadowQualitySelectionWidget->SetCurrentSelection(CurrentQuality);

	// Add available options to the widget labels
	for (const auto& Quality : QualityOptionsArray)
	{
		ShadingQualitySelectionWidget->AddOption({
			// AddOption expect a struct&, so we wrap this class in {}, converting it to a struct
			FEnumUtilities::EQualityToString(Quality)
		});
		
		QualityOptionIndex++ ;
	}

	ShadingQualitySelectionWidget->OnSelectionChange.BindLambda([this](const int Quality)
		{ // Lambda Scope
			GameUserSettings->SetShadingQuality(Quality);
				// UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%d"), Quality);
			GameUserSettings->ApplySettings(false);
		}
	); // end params
}
------------------------------------------------------------------------------*/

void UEDU_CORE_SettingsWidget::InitializeQualitySettings()
{ FLOW_LOG
	/* ------------------- InitializeQualitySettings -----------------------
	  This function manages the Quality Settings in UGameUserSettings,
	  which all use an array between 0-4.

	  Since all the settings use the same span, we initialize them
	  using a template rather than making a separate function for each.

	  The template loops over all the functions in SelectionElementsArray[]
	  and use them to initialize our custom SelectionWidget.
	-----------------------------------------------------------------------*/
	
	const FSelectionElement SelectionElementsArray[] = {
		{AntiAliasingQualitySelectionWidget, &UGameUserSettings::GetAntiAliasingQuality, &UGameUserSettings::SetAntiAliasingQuality},
		{FoliageQualitySelectionWidget, &UGameUserSettings::GetFoliageQuality, &UGameUserSettings::SetFoliageQuality},
		{ GlobalIlluminationQualitySelectionWidget, &UGameUserSettings::GetGlobalIlluminationQuality, &UGameUserSettings::SetGlobalIlluminationQuality},
		{ PostProcessingQualitySelectionWidget, &UGameUserSettings::GetPostProcessingQuality, &UGameUserSettings::SetPostProcessingQuality},
		{ReflectionQualitySelectionWidget, &UGameUserSettings::GetReflectionQuality, &UGameUserSettings::SetReflectionQuality},
		{ ShadingQualitySelectionWidget, &UGameUserSettings::GetShadingQuality, &UGameUserSettings::SetShadingQuality},
		{ShadowQualitySelectionWidget, &UGameUserSettings::GetShadowQuality, &UGameUserSettings::SetShadowQuality},
		{TextureQualitySelectionWidget, &UGameUserSettings::GetTextureQuality, &UGameUserSettings::SetTextureQuality},
		{ VFXQualitySelectionWidget, &UGameUserSettings::GetVisualEffectQuality, &UGameUserSettings::SetVisualEffectQuality},
		{ViewDistanceQualitySelectionWidget, &UGameUserSettings::GetViewDistanceQuality, &UGameUserSettings::SetViewDistanceQuality},
	};
	
	for (const auto& [Widget, GetQuality, SetQuality]: SelectionElementsArray)
	{
		if (!Widget) { FLOW_LOG_ERROR("Widget in SelectionElementsArray is nullptr.") return; }

		// No need to fill the array with options if options already exist.
		if ( Widget->GetCurrentSelection() <= 0 )
		{
			// Add available options to the SelectionWidget labels
			int QualityOptionIndex = 0;
			for (const auto& Quality : QualityOptionsArray)
			{
				Widget->AddOption({
					// AddOption() expect a struct&, so we wrap this class in {}, converting it to a struct
					FEnumUtilities::EQualityToString(Quality)
				});
				// Next label
				QualityOptionIndex++ ;
			}
		}

		// Use std::invoke to call the getter function (GetQuality) on the GameUserSettings object and store the result in CurrentSelection.
		const int CurrentSelection = std::invoke(GetQuality, GameUserSettings);
		
		// Set the current selection of the SelectionWidget to the value obtained from the getter function
		Widget->SetCurrentSelection(CurrentSelection);
		
		// Bind a lambda function to the OnSelectionChange event of the Widget, passing Selection from OnSelectionChange.Broadcast(int)
		Widget->OnSelectionChange.BindLambda([this, LambdaSetQuality = SetQuality](int Selection)
		{
			// Use std::invoke to call the setter function (SetFunc) on the GameUserSettings object with Selection as the argument
			std::invoke(LambdaSetQuality, GameUserSettings, Selection);
			
			// Applies all current user settings to the game and saves to permanent storage (e.g. file),
			// optionally checking for command line overrides.
			GameUserSettings->ApplySettings(false);
		});
	}
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void UEDU_CORE_SettingsWidget::OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType)
{ FLOW_LOG
	// Get the Array Index through the Combo Index
	const FIntPoint SelectedResolution = ResolutionsArray[ResolutionComboBox->GetSelectedIndex()];

	// Set the resolution but don't apply it. Wait for the User to do that.
	GameUserSettings->SetScreenResolution(SelectedResolution);
	GameUserSettings->ApplySettings(false);
	
}

void UEDU_CORE_SettingsWidget::OnVSyncChanged(bool IsChecked)
{ FLOW_LOG
	GameUserSettings->SetVSyncEnabled(IsChecked);
	GameUserSettings->ApplySettings(false);
}

void UEDU_CORE_SettingsWidget::AutoDetectQuality()
{
	GameUserSettings->RunHardwareBenchmark();
	GameUserSettings->ApplyHardwareBenchmarkResults();
	InitializeQualitySettings();
}
