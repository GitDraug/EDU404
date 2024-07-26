// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Components/EDU_CORE_SelectionWidget.h"

#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_UI.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------
UEDU_CORE_SelectionWidget::UEDU_CORE_SelectionWidget()
{ FLOW_LOG
	// Reset selection
		CurrentSelection = 0;
	// Making focusable will allow us to use arrow keys and a controller.
		SetIsFocusable(true);

	/*---------------------- SetVisibility ---------------------------
	  <!> Normal SetVisibility is virtual, and shouldn't be used
	  in a constructor. Visibility allows us to use a widget. Use
	  this when you want the widget to be shown and interactive.

	  (EVisibility::Visible): The widget is rendered and can
	  interact with user input. Useful for conditionally displaying
	  elements without affecting the layout of other widgets.

	  (EVisibility::Collapsed): The widget is not rendered and does
	  not take up any space in the layout. It is also not interactive.
	  
	  (EVisibility::Hidden): The widget is not rendered but still
	  occupies space in the layout. It is also not interactive.
	  Handy for toggling the visibility without rearranging
	  the UI layout.

	  (EVisibility::HitTestInvisible): The widget is rendered but
	  does not respond to user input. Events like clicks will pass
	  through this widget to widgets underneath it. Ideal for
	  decorative elements or backgrounds where you don't want to
	  block interactions with underlying widgets.

	  (EVisibility::Collapsed): The widget is not rendered and does
	  not take up any space in the layout. It is also not interactive.
	  Use this when you need the widget itself to be non-interactive,
	  but still want its children to respond to input.
	-------------------------------------------------------------------*/
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void UEDU_CORE_SelectionWidget::NativeConstruct()
{ FLOW_LOG
	// Super::NativeConstruct(); // <- Don't

	if(OptionsArray.Num() == 0){ FLOW_LOG_WARNING("No Options Provided") }
	
	// Show the first label
	UpdateCurrentSelection();

	/*--------------- FCustomWidgetNavigationDelegate -------------------
	  FCustomWidgetNavigationDelegate is a delegate type that allows
	  for custom navigation rules to be set. When the user tries to
	  navigate left or right within the widget, the OnNavigation
	  method of the USelectionBase class will be called to handle
	  the navigation logic.

	  SetNavigationRuleCustom is a method that sets a custom navigation
	  rule for a given direction. EUINavigation::Left and Right are
	  enums that represent the left and right navigation directions,
	  respectively.

	  By passing NavigationDelegate to SetNavigationRuleCustom, the code
	  specifies that the OnNavigation function should handle navigation
	  when the left or right keys are pressed.
	-------------------------------------------------------------------*/
	FCustomWidgetNavigationDelegate NavigationDelegate;
	NavigationDelegate.BindDynamic(this, &UEDU_CORE_SelectionWidget::OnNavigation);
	
	SetNavigationRuleCustom(EUINavigation::Left, NavigationDelegate);
	SetNavigationRuleCustom(EUINavigation::Right, NavigationDelegate);

	//--------------------------------------------------
	// Delegate Binding for buttons
	//--------------------------------------------------
	if (Button_Left){ Button_Left->OnClicked.AddDynamic(this, &ThisClass::SelectPrevious); }
	if (Button_Right){ Button_Right->OnClicked.AddDynamic(this, &ThisClass::SelectNext); }

	FLOW_LOG_WARNING("NativeConstruct Completed")
}
//------------------------------------------------------------------------------
// Functionality: OnNavigation
//------------------------------------------------------------------------------

UWidget* UEDU_CORE_SelectionWidget::OnNavigation(EUINavigation InNavigation)
{
	FLOW_LOG
	  if(!(InNavigation == EUINavigation::Left || InNavigation == EUINavigation::Right)){ FLOW_LOG_WARNING("Wrong EUINavigation; returning null") return nullptr; }

	const int Direction = InNavigation == EUINavigation::Left ? -1 : 1 ; 

	CurrentSelection += Direction;
	if(CurrentSelection < 0)
	{
		// Loop to end
		CurrentSelection = OptionsArray.Num() -1;
	}
	else if (CurrentSelection >= OptionsArray.Num())
	{
		// Loop to start
		CurrentSelection = 0;
	}

	// Broadcast change to anyone listening.
	// ReSharper disable once CppExpressionWithoutSideEffects
	OnSelectionChange.ExecuteIfBound(CurrentSelection);

	// Update Label
	UpdateCurrentSelection();

	// We need to return this as a result of OnNavigationWidget.
	// We have handled navigation internally and don't want to switch to another widget.
	return this;
}

void UEDU_CORE_SelectionWidget::SelectPrevious()
{ FLOW_LOG
	OnNavigation(EUINavigation::Left);
}

void UEDU_CORE_SelectionWidget::SelectNext()
{ FLOW_LOG
	OnNavigation(EUINavigation::Right);
}

//------------------------------------------------------------------------------
// Functionality: Selection
//------------------------------------------------------------------------------

void UEDU_CORE_SelectionWidget::ClearCurrentSelection()
{ FLOW_LOG
	OptionsArray.Reset();
}

void UEDU_CORE_SelectionWidget::AddOption(const FSelectionOption& inOption)
{ FLOW_LOG
	OptionsArray.Add(inOption);
	UpdateCurrentSelection();
}

void UEDU_CORE_SelectionWidget::SetCurrentSelection(int Index)
{ FLOW_LOG
	// We want to make sure the InIndex is actually within the array.
	if(Index < 0 || Index > OptionsArray.Num()) { FLOW_LOG_ERROR("Index is out of bounds") return; }

	CurrentSelection = Index;
	UpdateCurrentSelection();
}

void UEDU_CORE_SelectionWidget::UpdateCurrentSelection()
{ FLOW_LOG
	if (CurrentSelection < 0 || CurrentSelection >= OptionsArray.Num()) { FLOW_LOG_ERROR("Index is out of bounds") return; }
	
	// Set text to current selected label
	ButtonLabel->SetText(OptionsArray[CurrentSelection].Label);
}