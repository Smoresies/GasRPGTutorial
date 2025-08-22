// Copyright Smoresies


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(CastChecked<UAuraAttributeSet>(AttributeSet)->GetHealth());
	OnMaxHealthChanged.Broadcast(CastChecked<UAuraAttributeSet>(AttributeSet)->GetMaxHealth());
}
