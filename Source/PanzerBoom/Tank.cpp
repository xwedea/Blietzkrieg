// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Tower.h"
#include "CollisionQueryParams.h"
#include "Missile.h"
#include "Components/CapsuleComponent.h"
#include "TankPlayerController.h"

ATank::ATank() {
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
	TankController = Cast<ATankPlayerController>(GetController());
}

void ATank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATank::Move);
	PlayerInputComponent->BindAxis("Turn", this, &ATank::Turn);
	PlayerInputComponent->BindAxis("TurretRight");
	PlayerInputComponent->BindAxis("TurretForward");
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATank::Fire);
	PlayerInputComponent->BindAction("LaunchMissile", IE_Pressed, this, &ATank::LaunchMissile);
	PlayerInputComponent->BindAction("AimLock", IE_Pressed, this, &ATank::AimLock);
}

// Called every frame
void ATank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleAllCountdowns();
	Aim();
	RotateTurret();

	if (LockedActor) {
		HandleSwitchTarget();
		if (LockedActor->bAlive) {
			DrawSphere(LockedActor->GetActorLocation(), FColor::Red);
			SetSpringArmRotationYaw(GetTurretRotation().Yaw);
		}
	}
}

void ATank::HandleAllCountdowns() {
	Super::HandleAllCountdowns();
	Countdown(MissileRate, MissileCountdown);
}


void ATank::AimLock() {
	if (LockedActor) {
		if (AimedActor == LockedActor) {
			HandleTargetUnlock();
		}
		else {
			LockedActor = AimedActor;
		}
		return;
	}

	if (AimedActor) {
		LockedActor = AimedActor;
		SetSpringArmRotationYaw(GetTurretRotation().Yaw);
	}
	return;
}

void ATank::LaunchMissile() {
	if (MissileCountdown != 0.f) return;
	MissileCountdown = MissileRate;

	FVector Location = ProjectileSpawnPoint->GetComponentLocation() + 
		ProjectileSpawnPoint->GetForwardVector() * 70;
	FRotator Rotation = ProjectileSpawnPoint->GetComponentRotation();
	if (!MissileClass) {
		UE_LOG(LogTemp, Error, TEXT("%s: No Missile Class!"), *GetActorNameOrLabel());
		return;
	}
	AMissile * Missile = GetWorld()->SpawnActor<AMissile>(
		MissileClass,
		Location,
		Rotation
	);
	Missile->SetOwner(this);
	
}

void ATank::HandleSwitchTarget() {
	if (IsCoolingDown(SwitchTargetRate, SwitchTargetCountdown)) {
		return;
	}

	float controllerX = GetInputAxisValue(TEXT("TurretRight"));
	float controllerY = GetInputAxisValue("TurretForward");
	if (abs(controllerX) < 0.5 && abs(controllerY) < 0.5) {
		return;
	}

	FRotator FinalRotation = TankController->GetRightTSRotation(controllerX, controllerY);
	FVector SweepUnitVector = FinalRotation.Vector();
	
	FVector CollisionBoxVector = FVector(
		1,
		SweepCollisionBoxLength,
		100
	);
	FVector SweepStart = LockedActor->GetActorLocation() + SweepUnitVector * 50;
	FVector SweepEnd = SweepStart + SweepUnitVector * SwitchTargetRange;

	DrawDebugBox(
		GetWorld(),
		SweepEnd,
		CollisionBoxVector,
		SweepUnitVector.Rotation().Quaternion(),
		FColor::Red,
		true
	);
	
	FCollisionShape CollisionBox = FCollisionShape::MakeBox(CollisionBoxVector);
	FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(SwitchTargetRadius);
	FCollisionQueryParams TraceParams(FName(TEXT("Platform Trace")), true, LockedActor);
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		SweepStart,
		SweepEnd,
		SweepUnitVector.Rotation().Quaternion(),
		ECC_GameTraceChannel2,
		CollisionBox,
		TraceParams
	);

	if (bHit) {
		AActor * HitActor = HitResult.GetActor();
		UE_LOG(LogTemp, Display, TEXT("Switch to %s"), *HitActor->GetActorNameOrLabel());
		LockedActor = Cast<ABasePawn>(HitActor);
		return;
	}

	// second sweep
	CollisionBoxVector = FVector(
		1,
		SweepCollisionBoxLength * 4,
		100
	);
	CollisionBox = FCollisionShape::MakeBox(CollisionBoxVector);
	DrawDebugBox(
		GetWorld(),
		SweepEnd,
		CollisionBoxVector,
		SweepUnitVector.Rotation().Quaternion(),
		FColor::Blue,
		true
	);
	bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		SweepStart,
		SweepEnd,
		SweepUnitVector.Rotation().Quaternion(),
		ECC_GameTraceChannel2,
		CollisionBox,
		TraceParams
	);
	if (bHit) {
		AActor * HitActor = HitResult.GetActor();
		UE_LOG(LogTemp, Display, TEXT("Switch to %s"), *HitActor->GetActorNameOrLabel());
		LockedActor = Cast<ABasePawn>(HitActor);
		return;
	}

}



// void ATank::HandleSwitchTarget() {
// 	if (IsCoolingDown(SwitchTargetRate, SwitchTargetCountdown)) {
// 		return;
// 	}

// 	float controllerX = GetInputAxisValue(TEXT("TurretRight"));
// 	if (abs(controllerX) < 0.5) return;

// 	// Collision
// 	bool toRight = (controllerX > 0) ? true : false;
// 	FVector SweepUnitVector = (toRight) ? TurretMesh->GetRightVector() : -TurretMesh->GetRightVector();	
// 	float DistanceToTarget = (LockedActor->GetActorLocation() - GetActorLocation()).Length();
	
// 	FVector CollisionBoxVector = FVector(
// 		1,
// 		DistanceToTarget * SweepCollisionBoxConst,
// 		100
// 	);
// 	FVector SweepStart = GetActorLocation() + 
// 		TurretMesh->GetForwardVector() * DistanceToTarget * SweepCollisionBoxConst;
// 	FVector SweepEnd = SweepStart + SweepUnitVector * SwitchTargetRange;

// 	// DrawDebugBox(
// 	// 	GetWorld(),
// 	// 	SweepEnd,
// 	// 	CollisionBoxVector,
// 	// 	SweepUnitVector.Rotation().Quaternion(),
// 	// 	FColor::Red,
// 	// 	true
// 	// );
	
// 	FCollisionShape CollisionBox = FCollisionShape::MakeBox(CollisionBoxVector);
// 	FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(SwitchTargetRadius);
// 	FCollisionQueryParams TraceParams(FName(TEXT("Platform Trace")), true, LockedActor);
// 	FHitResult HitResult;
// 	bool bHit = GetWorld()->SweepSingleByChannel(
// 		HitResult,
// 		SweepStart,
// 		SweepEnd,
// 		SweepUnitVector.Rotation().Quaternion(),
// 		ECC_GameTraceChannel2,
// 		CollisionBox,
// 		TraceParams
// 	);

// 	if (bHit) {
// 		AActor * HitActor = HitResult.GetActor();
// 		UE_LOG(LogTemp, Display, TEXT("Switch to %s"), *HitActor->GetActorNameOrLabel());
// 		LockedActor = Cast<ABasePawn>(HitActor);
// 	}
// }

void ATank::SwitchTargetAfterKill() {
	FVector SweepStart = LockedActor->GetActorLocation();
	FVector SweepEnd = SweepStart + FVector(1);

	float SweepSphereRadius = 500.f;

	FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(SweepSphereRadius);
	FCollisionQueryParams TraceParams(FName(TEXT("Platform Trace")), true, LockedActor);
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		CollisionSphere,
		TraceParams
	);

	if (bHit) {
		ABasePawn * HitActor = Cast<ABasePawn>(HitResult.GetActor());
		LockedActor = HitActor;
	}
	else {
		HandleTargetUnlock();
	}
}

void ATank::HandleTargetUnlock() {
	LockedActor = nullptr;
	SetSpringArmRotationYaw(GetActorRotation().Yaw);
}

void ATank::Aim() {

	FHitResult HitResult;
	FVector EndLoc = ProjectileSpawnPoint->GetComponentLocation() + \
		ProjectileSpawnPoint->GetForwardVector() * AimRange;

	FCollisionShape CollisionBox = FCollisionShape::MakeBox(FVector(10, 10, 1));
	FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(AimRadius);
	bAiming = GetWorld()->SweepSingleByChannel(
		HitResult,
		ProjectileSpawnPoint->GetComponentLocation() + \
			FVector(0, 0, 100) + ProjectileSpawnPoint->GetForwardVector() * 100,
		EndLoc,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		CollisionSphere
	);

	if (!bAiming) {
		AimedActor = nullptr;
		return;
	}

	AActor * HitActor = HitResult.GetActor();
	AimedActor = Cast<ABasePawn>(HitResult.GetActor());
	if (AimedActor) {
		if (AimedActor->ActorHasTag("Enemy")) {
		
			if (AimedActor != LockedActor) {
				DrawSphere(AimedActor->GetActorLocation(), FColor::Green);
			}
		}

	}
	else {
		HandleTargetUnlock();
	}

}

void ATank::RotateTurret() {
	FRotator NewRotation;

	if (!LockedActor) {
		float controllerX = GetInputAxisValue(TEXT("TurretRight"));
		float controllerY = GetInputAxisValue("TurretForward");
		if (abs(controllerX) < 0.5 && abs(controllerY) < 0.5) return;

		FRotator FinalRotation = TankController->GetRightTSRotation(controllerX, controllerY);

		NewRotation = FMath::RInterpConstantTo(
			TurretMesh->GetComponentRotation(),
			FinalRotation,
			GetWorld()->GetDeltaSeconds(),
			TurretTurnRate
		);
	}
	else {
		FVector ToTarget = LockedActor->GetActorLocation() - GetActorLocation();
		NewRotation = FRotator(0, ToTarget.Rotation().Yaw, 0);
	}


	TurretMesh->SetWorldRotation(NewRotation);
}

void ATank::Move(float Value) {
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	float ForwardOffset = Value * Speed * DeltaTime;

	FVector DeltaLocation(ForwardOffset, 0.0f, 0.0f);
	AddActorLocalOffset(DeltaLocation, true);
}

void ATank::Turn(float Value) {
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	float TurnVal = Value * TurnRate * DeltaTime;

	FRotator TurnOffset(0, TurnVal, 0);
	AddActorLocalRotation(TurnOffset, true);

}

void ATank::HandleDestruction() {
	Super::HandleDestruction();

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void ATank::DrawSphere(FVector Loc, const FColor &Color) {
	DrawDebugSphere(	
		GetWorld(),
		Loc,
		50,
		30,
		Color
	);
}

void ATank::SetSpringArmRotationYaw(float Yaw) {
	FRotator SpringArmRot = SpringArm->GetComponentRotation();
	SpringArmRot.Yaw = Yaw;
	SpringArm->SetWorldRotation(SpringArmRot);
}


FRotator ATank::GetSpringArmRotation() const {
	return SpringArm->GetComponentRotation();
}