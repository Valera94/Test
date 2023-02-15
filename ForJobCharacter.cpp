// Copyright Epic Games, Inc. All Rights Reserved.

#include "ForJobCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Components/TimelineComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"


//////////////////////////////////////////////////////////////////////////
// AForJobCharacter

AForJobCharacter::AForJobCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)



	//ForPrototype
	TPS = CreateDefaultSubobject<USceneComponent>(TEXT("Scene1"));
	TPS->SetupAttachment(CameraBoom);
	FPS = CreateDefaultSubobject<USceneComponent>(TEXT("Scene2"));
	FPS->SetupAttachment(RootComponent);



}

void AForJobCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();



	MyCameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

	
	FOnTimelineFloat ProgressUpdate;
	ProgressUpdate.BindUFunction(this, FName("ViewUpdate"));

	FOnTimelineEvent FinishedEvent;
	FinishedEvent.BindUFunction(this, FName("ChangeAttach"));

	InterpolateViewChange.AddInterpFloat(InterpolateChangeCurve, ProgressUpdate);
	InterpolateViewChange.AddEvent(0.8f, FinishedEvent);

	}
}
void AForJobCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InterpolateViewChange.TickTimeline(DeltaTime);
}

///////////////////////////////////////////////////////////
//ChangeView

void AForJobCharacter::ViewUpdate(float Alpha)
{
	CameraBoom->TargetArmLength = FMath::Lerp(300.0f, 80.0f, Alpha);
	if (!AttachIsFPS)
	{
		MyCameraManager->SetManualCameraFade(Alpha, FLinearColor::Black, false);
	}

}
void AForJobCharacter::ChangeAttach()
{
	if (AttachIsFPS)
	{
		FollowCamera->AttachToComponent(TPS, FAttachmentTransformRules::KeepRelativeTransform);
		AttachIsFPS = false;
	}
	else
	{
		FollowCamera->AttachToComponent(FPS, FAttachmentTransformRules::KeepRelativeTransform);
		AttachIsFPS = true;
		MyCameraManager->StopCameraFade();
	}

}
void AForJobCharacter::StartedInterp()
{
	InterpolateViewChange.Play();
	UE_LOG(LogTemp, Error, TEXT("Start"));
}
void AForJobCharacter::StopInterp()
{
	InterpolateViewChange.Reverse();
	UE_LOG(LogTemp, Error, TEXT("Stop"));

	MyCameraManager->StartCameraFade(2.0f, 0.0f, 0.8f, FLinearColor::Black);
}

//////////////////////////////////////////////////////////////////////////
// Input
void AForJobCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AForJobCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AForJobCharacter::Look);
		
		//ChangeView
		EnhancedInputComponent->BindAction(ChangeView, ETriggerEvent::Started, this, &AForJobCharacter::StartedInterp);
		EnhancedInputComponent->BindAction(ChangeView, ETriggerEvent::Completed, this, &AForJobCharacter::StopInterp);
	}

}


////////////////////////
//Interpolate Camera

//////////////////////////////
void AForJobCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}
void AForJobCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
