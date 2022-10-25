// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectUFOCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Utils/ProceduralComponent.h"
#include "ProjectUFOPrivate.h"
#include "UFOBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjectUFOGameMode.h"

//////////////////////////////////////////////////////////////////////////
// AProjectUFOCharacter

AProjectUFOCharacter::AProjectUFOCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 1000.0f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->SetWorldRotation(FQuat(FVector(0.0f, 1.0f, 0.0f), 3.14f * 0.44444f)); // rotate 80 degree around x axis
	//FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	GameTeamMate = nullptr;
	EnableToMove = true;

	SkillComponent = MakeShareable(new FProceduralComponent());
	SkillComponent->AddProcedure("MakeShell");
	SkillComponent->AddProcedure("Dizzy");

	SkillComponent->GetProcedure("MakeShell")->ProcedureDelegate.AddUObject(this, &AProjectUFOCharacter::OnMakeShell);
	SkillComponent->GetProcedure("MakeShell")->OnProcedureDelegate.AddUObject(this, &AProjectUFOCharacter::OnShelled);

	SkillComponent->GetProcedure("Dizzy")->ProcedureDelegate.AddUObject(this, &AProjectUFOCharacter::OnStartDizzy);
	SkillComponent->GetProcedure("Dizzy")->OnProcedureDelegate.AddUObject(this, &AProjectUFOCharacter::OnDizzying);
}

void AProjectUFOCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		OnActorHit.AddDynamic(this, &AProjectUFOCharacter::OnActorBump);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectUFOCharacter::OnRep_IsPauseByOther_Implementation()
{
	
}

void AProjectUFOCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AProjectUFOCharacter::OnTriggerSkill);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AProjectUFOCharacter::OnReleaseSkill);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectUFOCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectUFOCharacter::MoveRight);
}

void AProjectUFOCharacter::ShowGuidUIMulti_Implementation(int32 Index)
{
	AUFOBase* CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
	if(IsValid(CurrentUFO))
	{
		CurrentUFO->SetUFOPlayerDisable();
	}
	ShowGuidUIBluePrint(Index);
}

void AProjectUFOCharacter::ShowGuidUIBluePrint_Implementation(int32 Index)
{
	
}

void AProjectUFOCharacter::EnableUFO_Implementation()
{
	AUFOBase* CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
	if(IsValid(CurrentUFO))
	{
		CurrentUFO->PlayerEnableUFO(PlayerTag);
	}
}

void AProjectUFOCharacter::ReStartGame_Implementation()
{
	OnGameContinue();
	AUFOBase* CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));

	if(IsValid(CurrentUFO))
	{
		CurrentUFO->SetUFOEnable(false);
	}
	if (AProjectUFOGameMode* CurrentGameMode = Cast<AProjectUFOGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		CurrentGameMode->RestartGame();
	}
}

void AProjectUFOCharacter::SetViewTarget()
{
	AUFOBase* CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
	APlayerController* PC = Cast<APlayerController>(this->GetController());

	if(IsValid(PC) && IsValid(CurrentUFO))
	{
		PC->SetViewTargetWithBlend(CurrentUFO);
	}
}

void AProjectUFOCharacter::SetViewTargetClient_Implementation()
{
	AUFOBase* CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
	APlayerController* PC = Cast<APlayerController>(this->GetController());

	if(IsValid(PC) && IsValid(CurrentUFO))
	{
		PC->SetViewTargetWithBlend(CurrentUFO);
	}
}

void AProjectUFOCharacter::SetControlRotation_Implementation(FRotator Rotator)
{
	if (Controller)
	{
		Controller->SetControlRotation(Rotator);
	}
}

bool AProjectUFOCharacter::IsSkillTriggered()
{
	return SkillComponent->GetProcedure("MakeShell")->GetState() == EProceduralState::Trigger;
}

void AProjectUFOCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectUFOCharacter, PlayerTag);
	DOREPLIFETIME_CONDITION(AProjectUFOCharacter, IsPauseByOther, COND_AutonomousOnly);
	DOREPLIFETIME(AProjectUFOCharacter, SkillEnergy);
}

void AProjectUFOCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProjectUFOCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProjectUFOCharacter::OnRep_PlayerTag_Implementation()
{
	
}

void AProjectUFOCharacter::MoveForward(float Value)
{
	if (!EnableToMove) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
	}
}

void AProjectUFOCharacter::MoveRight(float Value)
{
	if (!EnableToMove) return;
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Value);
	}
}

// Procedual Setups
// 1 - start : shell skill
void AProjectUFOCharacter::OnMakeShell(EProceduralState State)
{
	switch (State)
	{
		case EProceduralState::Trigger:
			ProcessMakeShell();
			break;

		case EProceduralState::Process:
			bSkillTriggered = false;
			break;

		case EProceduralState::Suspend:
			break;

		case EProceduralState::Idle:
			bSkillTriggered = false;
			break;
	}
}

void AProjectUFOCharacter::OnShelled(EProceduralState State, float RemainTime)
{
	switch (State)
	{
	case EProceduralState::Trigger:
		break;

	case EProceduralState::Process:
		SkillEnergy = RemainTime / ShellTime;
		break;

	case EProceduralState::Suspend:
		SkillEnergy = 1 - RemainTime / ShellCD;
		break;
	}
}
// 1 - end : shell skill

// 2 - start : Bump
void AProjectUFOCharacter::OnStartDizzy(EProceduralState State)
{
	switch (State)
	{
	case EProceduralState::Trigger:
		UE_LOG(LOGUFORuntime, Log, TEXT("OnTrigger_StartDizzy"));
		ProcessDizzy();
		break;

	case EProceduralState::Process:
		UE_LOG(LOGUFORuntime, Log, TEXT("OnProcess_StartDizzy"));
		LaunchCharacter(LaunchPower, true, false);
		break;

	case EProceduralState::Suspend:
		UE_LOG(LOGUFORuntime, Log, TEXT("OnSuspend_StartDizzy"));
		break;

	case EProceduralState::Idle:
		UE_LOG(LOGUFORuntime, Log, TEXT("OnFinish_StartDizzy"));
		break;
	}
}

void AProjectUFOCharacter::OnDizzying(EProceduralState State, float RemainTime)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	switch (State)
	{
	case EProceduralState::Process:
		EnableToMove = false;
		UE_LOG(LOGUFORuntime, Log, TEXT("OnProcessing_ProcessBump - time remaining [%s]"), *FString::SanitizeFloat(RemainTime));
		break;

	case EProceduralState::Suspend:
		EnableToMove = true;
		UE_LOG(LOGUFORuntime, Log, TEXT("OnSuspending_ProcessBump - time remaining [%s]"), *FString::SanitizeFloat(RemainTime));
		break;
	}
}
// 2 - end : Bump

void AProjectUFOCharacter::TriggerMakeShell()
{
	// Update Skill Settings
	SkillComponent->GetProcedure("MakeShell")->SetTriggerTime(ShellTriggerTime);
	SkillComponent->GetProcedure("MakeShell")->SetProceduralTime(ShellTime);
	SkillComponent->GetProcedure("MakeShell")->SetProceduralCD(ShellCD);
	SkillComponent->GetProcedure("MakeShell")->SetReleaseState(ReleaseAllowed);

	SkillComponent->GetProcedure("MakeShell")->Trigger();
}

void AProjectUFOCharacter::ReleaseMakeShell()
{
	// Update Skill Settings
	SkillComponent->GetProcedure("MakeShell")->ReleaseTrigger();
}

void AProjectUFOCharacter::ProcessMakeShell()
{
	if (GameTeamMate && GameTeamMate->IsSkillTriggered())
	{
		SkillComponent->GetProcedure("MakeShell")->Process();
		GameTeamMate->GetSkillComponent()->GetProcedure("MakeShell")->Process();
	}
}

void AProjectUFOCharacter::OnActorBump(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	AProjectUFOCharacter* SelfCharacter = Cast<AProjectUFOCharacter>(SelfActor);
	AProjectUFOCharacter* OtherCharacter = Cast<AProjectUFOCharacter>(OtherActor);

	if (SelfCharacter && OtherCharacter)
	{
		const FVector& V_s = SelfCharacter->GetVelocity();
		const FVector& V_o = OtherCharacter->GetVelocity();

		const FVector& P_s = SelfCharacter->GetActorLocation();
		const FVector& P_o = OtherCharacter->GetActorLocation();
		const FVector P_os = (P_s - P_o);
		const FVector P_os_n = P_os.GetSafeNormal();

		const float V_os = FVector::DotProduct((V_s - V_o), P_os_n);

		if (FMath::Abs(V_os) > CollideMinVelocity)
		{
			LaunchPower = P_os_n * LaunchStrength;

			SkillComponent->GetProcedure("Dizzy")->SetProceduralTime(DizzyTime);
			SkillComponent->GetProcedure("Dizzy")->SetProceduralCD(FreezeTime);
			SkillComponent->GetProcedure("Dizzy")->Trigger();

			UE_LOG(LOGUFORuntime, Log, TEXT("AProjectUFOCharacter::OnActorBump - [%s]"), *LaunchPower.ToString());
		}
	}
}

void AProjectUFOCharacter::ProcessDizzy()
{
	SkillComponent->GetProcedure("Dizzy")->Process();
}

void AProjectUFOCharacter::OnTriggerSkill_Implementation()
{
	TriggerMakeShell();
}

void AProjectUFOCharacter::OnReleaseSkill_Implementation()
{
	ReleaseMakeShell();
}

void AProjectUFOCharacter::OnGamePause_Implementation()
{
	if (GameTeamMate)
	{
		GameTeamMate->PauseTeamMate();
	}
}

void AProjectUFOCharacter::OnGameContinue_Implementation()
{
	if (GameTeamMate)
	{
		UGameplayStatics::GetGameMode(this)->ClearPause();
		GameTeamMate->ContinueTeamMate();
	}
}

void AProjectUFOCharacter::PauseTeamMate()
{
	IsPauseByOther = true;
}

void AProjectUFOCharacter::ContinueTeamMate()
{
	IsPauseByOther = false;
}

void AProjectUFOCharacter::ShowLoadingUIBluePrint_Implementation(bool State)
{
}



void AProjectUFOCharacter::ShowLoadingUI_Implementation(bool State)
{
	ShowLoadingUIBluePrint(State);
}

void AProjectUFOCharacter::ChangeSkeletalMeshClient_Implementation()
{
	ChangeSkeletalMesh();
}

void AProjectUFOCharacter::ChangeSkeletalMesh_Implementation()
{
	
}

void AProjectUFOCharacter::SetWidgetVisibility_Implementation(const TArray<bool>& Visibility)
{
	SetWidgetVisibilityBlueprint(Visibility);
}

void AProjectUFOCharacter::SetWidgetVisibilityBlueprint_Implementation(const TArray<bool>& Visibility)
{

}

void AProjectUFOCharacter::GamePause_Implementation()
{
	UGameplayStatics::GetGameMode(this)->SetPause(UGameplayStatics::GetPlayerController(GWorld, 0));
	UGameplayStatics::GetGameMode(this)->SetPause(UGameplayStatics::GetPlayerController(GWorld, 1));
}

void AProjectUFOCharacter::GameContinue_Implementation()
{
	UGameplayStatics::GetGameMode(this)->ClearPause();
}

void AProjectUFOCharacter::OnRep_SkillEnergy_Implementation()
{

}