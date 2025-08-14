// Copyright Smoresies


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Runtime/ApplicationCore/Internal/GenericPlatform/CursorUtils.h"

AAuraPlayerController::AAuraPlayerController()
{
	// if the player changes on a server it will replicate to the clients
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// check will halt execution if AuraContext Input Map has not been set
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// CastChecked will both Cast and Check, crashing if this does not work
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Bound MoveAction to the Move function whenever our MoveAction is Triggered (including held down)
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const struct FInputActionValue& InputActionValue)
{
	// Get our InputAxisValue's data in the form of a Vector2D, this allows for 3D Horizontal movement.
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	// One line example: const FRotator YawRotation(0.f, GetControlRotation.Yaw(), 0.f);
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Gives the normalized forward vector that corresponds to our yaw rotation
	// Should be the direction from camera to character, parallel to the ground
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Same as above, but Y gives us the right direction vector
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

	// Can declare and set in if. Scope exists only within this block.
	// Not an assert/check as Move might be called before the pawn is valid
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// Add input for our pawn. The InputAxisVector allows us to scale (specifically allows to flip to negative if going back)
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;

	// If this cast succeeds then the actor we hit (is under cursor in this case) implements IEnemyInterface
	// Cast<IEnemyInterface>(CursorHit.GetActor()); This is no longer used with the Interface Pointers

	// With the Interface-specific pointer we can just directly set equal
	ThisActor = CursorHit.GetActor();

	/**
	 * Line Trace from cursor - There are several possible scenarios:
	 * A. LastActor AND ThisActor are both null
	 *		- Whatever we hit last frame was not an EnemyInterface, and isn't this frame either.
	 *		- We do nothing here (likely early return)
	 * B. LastActor is null but ThisActor is not
	 *		- Means we are hovering over this actor for the first time.
	 *		- Call Highlight on ThisActor
	 * C. LastActor is valid AND ThisActor is null
	 *		- Means we were previously hovering on an EnemyInterace but are no longer
	 *		- UnHighlight LastActor
	 * D. LastActor is valid AND ThisActor is valid, but LastActor != ThisActor
	 *		- We were hovering over one Enemy/Hoverable last time, but now we are hovering another
	 *		- UnHighlight LastActor, Highlight ThisActor
	 * E. LastActor is valid AND ThisActor is valid, LastActor == ThisActor
	 *		- Hovering over same enemy as previous frame. Do nothing.
	 **/

	// Case A, commented in case of other code down the line
	// if (LastActor == nullptr && ThisActor == nullptr) return;

	// LastActor NOT Valid:
	if (LastActor == nullptr)
	{
		// Case B
		if (ThisActor != nullptr)
			ThisActor->HighlightActor();
		// If gotten here then Case A (do nothing)
	}
	// LastActor Valid
	else
	{
		if (ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		// Both Actors valid
		else
		{
			// Case D
			if (LastActor != ThisActor)
			{
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			// Case E - Do Nothing
		}
	}
}
