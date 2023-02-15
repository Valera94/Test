// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/TimelineComponent.h"
#include "ForJobCharacter.generated.h"


class UCurveFloat;
class USceneComponent;
class APlayerCameraManager;

UCLASS(config=Game)
class AForJobCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Start
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** ChangeView */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ChangeView;

	// USceneComponent  ChangeTo SocketBody. For Prototype.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* TPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* FPS;

#pragma endregion

public:
	AForJobCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma region InterpolateFpsTps
public:
	//Just Curve from  Viewport
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InterpolateFpsTps")
	UCurveFloat* InterpolateChangeCurve = nullptr;

	//TimeLine Name
	FTimeline InterpolateViewChange;

	//Logics change functions.
	UFUNCTION(BlueprintCallable)
	void ViewUpdate(float Alpha);
	UFUNCTION(BlueprintCallable)
	void ChangeAttach();

	//JustStartInterpAndStopFor EnhancedInputComponent
	void StartedInterp();
	void StopInterp();

	bool AttachIsFPS = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ThisValue;

	UPROPERTY()
	APlayerCameraManager* MyCameraManager;

#pragma endregion
};