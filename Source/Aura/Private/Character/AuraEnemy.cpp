// Copyright Smoresies


#include "Character/AuraEnemy.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/Aura.h"

AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	/*
	 * Setup the AbilitySystem and AttributeSet to be owned by each Enemy
	 * Player will use PlayerState, hence why we do this in AuraEnemy instead of AuraCharacterBase
	 */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	/*
     * SetReplicationMode takes an Enum of EGameplayEffectReplicationMode
     * This has Three possible Modes:
     *		Full - Gameplay effects replicated to ALL CLIENTS.
     *			   Use for Single Player
     *		Mixed - Gameplay Effects replicated to owning client only. Gameplay Cues and Gameplay Tags replicated to all clients
     *			   Multiplayer (Player-Controlled)
     *		Minimal - Gameplay Effects are NOT Replicated. Gameplay Cues and Gameplay Tags replicated to all clients
     *			   Multiplayer (AI-Controlled) - Chosen here, as this is for AI
     */
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
