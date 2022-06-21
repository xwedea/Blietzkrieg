// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePawn.h"
#include "Tank.generated.h"
/**
 * 
 */
UCLASS()
class PANZERBOOM_API ATank : public ABasePawn
{
	GENERATED_BODY()

public:
	ATank();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Move(float Value);
	void Turn(float Value);
	void HandleDestruction();
	void HandleTargetUnlock();
	
	

	void RotateTurret();

	APlayerController * GetTankController() const { return TankController; }
	AActor * GetLockedActor() const {return LockedActor;}
	void SetLockedActor(ABasePawn * Value) {LockedActor = Value;}
	void SetSpringArmRotation(FRotator Rotation);
	void SetSpringArmRotationYaw(float Yaw);
	FRotator GetTurretRotation() const {return TurretMesh->GetComponentRotation();}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"));
		class USpringArmComponent * SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"));
		class UCameraComponent * Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement",meta = (AllowPrivateAccess = "true"));
		float Speed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement",meta = (AllowPrivateAccess = "true"));
		float TurnRate = 100.f;
	UPROPERTY(EditAnywhere, Category = "Movement");
		float TurretTurnRate = 200.f;
	UPROPERTY(EditAnywhere, Category = "Combat");
		float AimRadius = 100.f;
	UPROPERTY(EditAnywhere, Category="Combat")
		float AimRange = 1000.f;
	UPROPERTY(EditAnywhere, Category="Combat")
		float SwitchTargetRadius = 500.f;
	UPROPERTY(EditAnywhere, Category="Combat")
		float SwitchTargetRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"));
		APlayerController * TankController;

	ECollisionChannel AimTraceChannel = ECC_GameTraceChannel1;
	ECollisionChannel SwitchTargetTraceChannel = ECC_GameTraceChannel2;
	


	void Aim();
	bool bAiming = false;
	void AimLock();

	ABasePawn * AimedActor = nullptr;
	ABasePawn * LockedActor = nullptr;
	
	void HandleSwitchTarget();
	FTimerHandle SwitchTargetTimerHandle;
	UPROPERTY(EditAnywhere)
		float SwitchTargetRate = 0.2f;
	float SwitchTargetTimer = SwitchTargetRate;

	void DrawSphere(FVector Loc, const FColor &Color);

	const float SweepCollisionBoxConst = 2.f/3.f;

};
