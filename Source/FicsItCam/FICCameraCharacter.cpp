#include "FICCameraCharacter.h"

#include "CineCameraComponent.h"
#include "FGGameUserSettings.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "FICUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

void AFICCameraCharacter::OnTickWorldStreamTimer() {
	UWorld* world = GetWorld();
	world->UpdateLevelStreamingState(); 
	if(world->IsLevelStreamingRequestPending(world->GetFirstPlayerController())) return;
	AFGCharacterPlayer* Char = Cast<AFGCharacterPlayer>(OriginalCharacter);
	if (Char) Char->CheatToggleGhostFly(false);
	GetWorld()->GetTimerManager().ClearTimer(WorldStreamTimer);
}

AFICCameraCharacter::AFICCameraCharacter() {
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("CaptureComponent");
	CaptureComponent->SetupAttachment(GetCapsuleComponent());
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->ShowFlags.SetTemporalAA(true);

	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>("RenderTarget");
	CaptureComponent->TextureTarget = RenderTarget;
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	bSimGravityDisabled = true;
	
	SetActorEnableCollision(false);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AFICCameraCharacter::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	
}

void AFICCameraCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AFICCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	PlayerInputComponent->BindAction("FicsItCam.StopAnimation", EInputEvent::IE_Pressed, this, &AFICCameraCharacter::StopAnimation);
}

void AFICCameraCharacter::PossessedBy(AController* NewController) {}
void AFICCameraCharacter::UnPossessed() {}

void AFICCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (Animation) {
		//double Start = FPlatformTime::Seconds();
		float Time;
		if (bDoRender) {
			if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) return;
			
			Time = Progress;
			Progress += 1;

			if (!Animation->bBulletTime) SetTimeDilation(1.0f/Animation->FPS/DeltaSeconds);
		} else {
			Time = Progress * Animation->FPS;
			Progress += DeltaSeconds;
		}
		FVector Pos;
		Pos.X = Animation->PosX.GetValue(Time);
		Pos.Y = Animation->PosY.GetValue(Time);
		Pos.Z = Animation->PosZ.GetValue(Time);
		FRotator Rot;
		Rot.Pitch = Animation->RotPitch.GetValue(Time);
		Rot.Yaw = Animation->RotYaw.GetValue(Time);
		Rot.Roll = Animation->RotRoll.GetValue(Time);
		float FOV = Animation->FOV.GetValue(Time);
		float Aperture = Animation->Aperture.GetValue(Time);
		float FocusDistance = Animation->FocusDistance.GetValue(Time);
		
		SetActorLocation(Pos);
		SetActorRotation(Rot);
		GetController()->SetControlRotation(Rot);
		Camera->SetFieldOfView(FOV);
		UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
		if (CineCamera) {
			CineCamera->CurrentAperture = Aperture;
			CineCamera->FocusSettings.ManualFocusDistance = FocusDistance;
		}

		if (bDoRender) {
			FMinimalViewInfo ViewInfo;
			Camera->GetCameraView(0, ViewInfo);
			CaptureComponent->SetCameraView(ViewInfo);
			CaptureComponent->FOVAngle = Camera->FieldOfView;
			FWeightedBlendables Blendables = CaptureComponent->PostProcessSettings.WeightedBlendables;
			CaptureComponent->PostProcessSettings = ViewInfo.PostProcessSettings;
			//CaptureComponent->PostProcessSettings.WeightedBlendables = Blendables;
			CaptureComponent->PostProcessBlendWeight = Camera->PostProcessBlendWeight;
			
			CaptureComponent->CaptureScene();
			
			FString FSP;
			// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
			if (FSP.IsEmpty()) {
				FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/") TEXT("FicsItCam/"), Animation->GetName());
			}

			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);

			FSP = FPaths::Combine(FSP, FString::FromInt((int)Progress) + TEXT(".jpg"));

			bool bSuccess = FIC_SaveRenderTargetAsJPG(FSP, RenderTarget);
		if (!Animation->bBulletTime)
			if (bSuccess) {
				Cast<APlayerController>(GetController())->SetPause(false);
				if (Animation->GetEndOfAnimation() < Progress / Animation->FPS) {
					StopAnimation();
				}
			}
		} else {
			if (Animation->GetEndOfAnimation() < Progress) {
				StopAnimation();
			}
		}
		//SML::Logging::error((FPlatformTime::Seconds() - Start) * 1000);
	}
}

void AFICCameraCharacter::StartAnimation(AFICAnimation* inAnimation, bool bInDoRender) {
	StopAnimation();
	Animation = inAnimation;
	bDoRender = bInDoRender;
	if (!Animation) return;
	Progress = Animation->GetStartOfAnimation();
	
	AFGPlayerController* NewController = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	OriginalCharacter = NewController->GetCharacter();
	NewController->Possess(this);
	OriginalCharacter->SetActorHiddenInGame(true);
	Cast<AFGHUD>(NewController->GetHUD())->SetPumpiMode(true);
	NewController->PlayerCameraManager->UnlockFOV();
	
	if (bDoRender) {
		FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
		RenderTarget->InitCustomFormat(Resolution.X, Resolution.Y, EPixelFormat::PF_R8G8B8A8, false);
	}
	if (Camera) Camera->DestroyComponent();
	if (Animation->bUseCinematic) {
		UCineCameraComponent* CineCamera = NewObject<UCineCameraComponent>(this);
		CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
		CineCamera->bConstrainAspectRatio = false;
		Camera = CineCamera;
	} else {
		Camera = NewObject<UCameraComponent>(this);
	}
	Camera->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	AController* PController = Controller;
	PController->UnPossess();
	PController->Possess(this);

	if (Animation->bBulletTime) SetTimeDilation(0);
}

void AFICCameraCharacter::StopAnimation() {
	if (Animation) {
		AFGPlayerController* NewController = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
		NewController->Possess(OriginalCharacter);
		OriginalCharacter->SetActorHiddenInGame(false);
		Cast<AFGHUD>(NewController->GetHUD())->SetPumpiMode(false);
		Animation = nullptr;
		Progress = 0.0f;
		Cast<AFGCharacterPlayer>(OriginalCharacter)->CheatToggleGhostFly(true);
		GetWorld()->GetTimerManager().SetTimer(WorldStreamTimer, this, &AFICCameraCharacter::OnTickWorldStreamTimer, 0.1f, true);
		SetTimeDilation(1);
	}
}

void AFICCameraCharacter::SetTimeDilation(float InTimeDilation) {
	if (InTimeDilation < 0.0001) InTimeDilation = 0.0001;
	UGameplayStatics::SetGlobalTimeDilation(this, InTimeDilation);
	CustomTimeDilation = 1.0f/InTimeDilation;
}
