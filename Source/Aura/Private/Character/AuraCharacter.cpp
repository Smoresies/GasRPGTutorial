// Copyright Smoresies


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Should be done in editor but default can be set here
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0);

	// Typical for top-down game
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init Ability Actor Info for the SERVER
	InitAbilityActorInfo();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init Ability Info for the Client
	InitAbilityActorInfo();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState); // Should NOT be null at this point, if so VERY BAD

	// We set the Player State as the owner, and ourselves at the Avatar
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);

	// Set our variables
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// Due to the fact that only the Player on the Client will have a valid PlayerController
	// (other controlled ones will have a PlayerController on their Client, but that is not replicated)
	// We accept that AuraPlayerController MIGHT be null, but should not for our local client
	// Therefore we use an if rather than a check here
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	
}
