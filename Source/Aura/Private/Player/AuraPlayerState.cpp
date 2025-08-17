// Copyright Smoresies


#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

AAuraPlayerState::AAuraPlayerState()
{
	/*
	 * The player's version of the Ability System Component and Attribute Set
	 * Defined here in Player State. Each player will (likely) have a pointer
	 * to these, but they are stored here as to not be lost on Pawn destruction
	 */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	/**
	 *  How often the server will try to update clients.
	 *  As changes occur on server for player state, server will send updates out
	 *  Normally quite low, like half a second. If we had the attribute set on state, we would make higher
	 *  Apparently 100 is high? 100 updates per second
	**/
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
