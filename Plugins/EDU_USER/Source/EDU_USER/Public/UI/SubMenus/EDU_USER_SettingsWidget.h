// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDU_USER_SettingsWidget.generated.h"

class UCommonTextBlock;
class UEDU_USER_SelectionBase;
class UWidget;
class UComboBoxString;
class UCheckBox;
class UButton;

/*------------------------------------------------------------------------------
  Setting Menu
--------------------------------------------------------------------------------
  
  
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// Enum for Framerate settings
//------------------------------------------------------------------------------	
UENUM(BlueprintType)
enum class EFramerate: uint8
{
	FPS_30 UMETA(DisplayName = "30 FPS"),
	FPS_45 UMETA(DisplayName = "45 FPS"),
	FPS_60 UMETA(DisplayName = "60 FPS"),
	FPS_75 UMETA(DisplayName = "75 FPS"),
	FPS_120 UMETA(DisplayName = "120 FPS"),
	FPS_240 UMETA(DisplayName = "240 FPS"),
	FPS_Uncapped UMETA(DisplayName = "No limit")
};

//------------------------------------------------------------------------------
// Enum for Quality settings
//------------------------------------------------------------------------------	
UENUM(BlueprintType)
enum class EQuality: uint8
{
	Lowest	UMETA(DisplayName = "Lowest"),
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
	Highest	UMETA(DisplayName = "Highest"),
};

// This Enum can't easily be converted to strings or Int, so we use a helper class
class FEnumUtilities
{
public:
	// -------------------------------------------------------------------
	// Convert Framerate to Int
	// -------------------------------------------------------------------
	static int EFramerateToValue(const EFramerate& Framerate)
	{
		switch (Framerate)
		{
			case EFramerate::FPS_30: return 30;
			case EFramerate::FPS_45: return 45;
			case EFramerate::FPS_60: return 60;
			case EFramerate::FPS_75: return 75;
			case EFramerate::FPS_120: return 120;
			case EFramerate::FPS_240: return 240;
			case EFramerate::FPS_Uncapped: return 0;
			default: return 60;
		}
	}
	// -------------------------------------------------------------------
	// Convert FPS To String
	// -------------------------------------------------------------------
	static FString FramerateToString(const EFramerate& Framerate)
	{
		const int FPS = EFramerateToValue(Framerate);
		return FPS > 0 ? FString::Printf(TEXT("%d FPS"), FPS) : FString::Printf(TEXT("No limit)"));
	}

	// -------------------------------------------------------------------
	// Convert Quality to String
	// -------------------------------------------------------------------
	static FText EQualityToString(const EQuality& Quality)
	{
		switch (Quality)
		{
			case EQuality::Lowest:	return FText::FromString(TEXT("Lowest"));
			case EQuality::Low:		return FText::FromString(TEXT("Low"));
			case EQuality::Medium:	return FText::FromString(TEXT("Medium"));
			case EQuality::High:	return FText::FromString(TEXT("High"));
			case EQuality::Highest:	return FText::FromString(TEXT("Highest"));
		}
		return FText::FromString(TEXT("Unknown"));
	}
};

UCLASS()
class EDU_USER_API UEDU_USER_SettingsWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	// This acts as "OnConstructed()", called through: UUserWidget::OnWidgetRebuilt().
	// It notifies that the widget that it has been constructed.
	virtual void NativeConstruct() override;
	
	// The NativeGetDesiredFocusTarget function is used to determine which widget within
	// a composite widget (like a panel or user-defined widget) should receive focus
	// by default when the parent widget is focused. This is particularly useful in managing
	// keyboard or gamepad navigation within complex UIs.
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void InitializeResolutionComboBox();
	void InitializeVSync();
	void InitializeFramerate() const;
	void InitializeQualitySettings();

//------------------------------------------------------------------------------
// Data & Components
//------------------------------------------------------------------------------
protected:
	// Stores user settings for a game (for example, graphics and sound settings),
	// with the ability to save and load to and from a file.
	UPROPERTY()
	TObjectPtr<UGameUserSettings> GameUserSettings;

	//-------------------------------------------------------------
	// Resolution
	//-------------------------------------------------------------
	// FIntPoint is a Structure for integer points in 2-d space.
	// We use it to hold all resolutions available.
	TArray<FIntPoint> ResolutionsArray;

	//-------------------------------------------------------------
	// ComboBox with string values for resolutions
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;
	
	//-------------------------------------------------------------
	// VSync
	//-------------------------------------------------------------
	// CheckBox 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;
	
	//-------------------------------------------------------------
	// Framerate
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> FramerateSelectionWidget;

	
	/* TODO ----------------------- FPS Meter ---------------------------
	  An FPS Meter needs an outside Tick, because UserWidgets don't
	  have a tick. A solution is to use the HUDs tick, and and bind
	  a delegate, or just set it here from the HUD.
	-------------------------------------------------------------------*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> FPSMeter;
	
	/*-------------------- Scalability Settings ------------------------
	  Quality (scaling) settings refer to the various adjustable
	  parameters that determine the visual quality and performance
	  of the game.
	-------------------------------------------------------------------*/
	
	//-------------------------------------------------------------
	// Auto Detect Best Quality Button
	//-------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UButton* Button_AutoDetect;
	
	//-------------------------------------------------------------
	// AntiAliasing Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> AntiAliasingQualitySelectionWidget;
	
	//-------------------------------------------------------------
	// Foliage Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> FoliageQualitySelectionWidget;

	//-------------------------------------------------------------
	// Global Illumination Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> GlobalIlluminationQualitySelectionWidget;

	//-------------------------------------------------------------
	// PostProcessing Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> PostProcessingQualitySelectionWidget;
	
	//-------------------------------------------------------------
	// Reflection Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> ReflectionQualitySelectionWidget;

	//-------------------------------------------------------------
	// ShadingQuality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> ShadingQualitySelectionWidget;

	//-------------------------------------------------------------
	// Shadow Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> ShadowQualitySelectionWidget;
	
	//-------------------------------------------------------------
	// Texture Quality
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> TextureQualitySelectionWidget;
	
	
	//-------------------------------------------------------------
	// Visual Effects
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> VFXQualitySelectionWidget;

	//-------------------------------------------------------------
	// View Distance
	//-------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEDU_USER_SelectionBase> ViewDistanceQualitySelectionWidget;
	//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
private:
	UFUNCTION()
	void OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType);
	
	UFUNCTION()
	void OnVSyncChanged(bool IsChecked);

	// Lets the player Auto Detect the best settings through a button press.
	UFUNCTION()
	void AutoDetectQuality();
};
