// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDU_USER_SelectionWidget.generated.h"

class UCommonTextBlock;
class UButton;

/*------------------------------------------------------------------------------
  Selection Widget
--------------------------------------------------------------------------------
  This is a 1D Selection widget that allows you to switch setting.
  Looks like this:  <|Label|>

TODO:
  this class need to save its settings.

   Since this acts like a button, functionality for styles should be added.
   Check https://youtu.be/Rvw-YCEyUTI?si=ULh8hYfhmGNKUup4 11:00
   Also: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/CommonUI/UCommonButtonBase/OnFocusLost
------------------------------------------------------------------------------*/

USTRUCT(BlueprintType)
struct F_EDU_USER_SelectionOption // Used by the Array of SelectionOptions
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Option")
  FText Label;
};


UCLASS(Blueprintable, BlueprintType)
class EDU_USER_API UEDU_USER_SelectionWidget : public UCommonUserWidget
{
	GENERATED_BODY()
//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
  UEDU_USER_SelectionWidget();

  // This acts as "OnConstructed()", called through: UUserWidget::OnWidgetRebuilt().
  // It notifies that the widget that it has been constructed.
  virtual void NativeConstruct() override;

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------
public:
  // Takes an IntParam for the Index
  DECLARE_DELEGATE_OneParam(FOnSelectionChange, int)
  FOnSelectionChange OnSelectionChange;

protected:
  // Array of SelectionOptions
  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  TArray<F_EDU_USER_SelectionOption> OptionsArray;

  // SmartPointer to Widget TextBlock
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
  TObjectPtr<UCommonTextBlock> ButtonLabel;

  // Just to keep track of where we are in the OptionsArray.
  int32 CurrentSelection;

//------------------------------------------------------------------------------
// Data: Buttons
//------------------------------------------------------------------------------
protected:
  UPROPERTY(meta = (BindWidget))
  UButton* Button_Left;
  
  UPROPERTY(meta = (BindWidget))
  UButton* Button_Right;
  
//------------------------------------------------------------------------------
// Functionality: OnNavigation
//------------------------------------------------------------------------------
protected:
  // Used for overloading left and right arrow keys, using slate enums.
  // This way we use the same functionality independent of mouse keyboard or controller.
  UFUNCTION()
  UWidget* OnNavigation(EUINavigation InNavigation);

public:
  // Left Arrow Functionality
  UFUNCTION(BlueprintCallable)
  void SelectPrevious();

  // Right Arrow Functionality
  UFUNCTION(BlueprintCallable)
  void SelectNext();

//------------------------------------------------------------------------------
// Functionality: Selection
//------------------------------------------------------------------------------	
public:
  // Get current selection
  FORCEINLINE int32 GetCurrentSelection() const { return CurrentSelection; };
  
  // Clear current selection
  void ClearCurrentSelection();
  
  // Add selection
  void AddOption(const F_EDU_USER_SelectionOption& inOption);

  // Set current selection
  void SetCurrentSelection(int InIndex);

protected:
  // Updates the button label text
  void UpdateCurrentSelection();

};
