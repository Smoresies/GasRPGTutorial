// Copyright Smoresies


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	// if the player changes on a server it will replicate to the clients
	bReplicates = true;
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
