#pragma once

#include "FICRuntimeProcess.h"
#include "FICRuntimeProcessPlayScene.generated.h"

class AFICCaptureCamera;

UCLASS()
class UFICRuntimeProcessPlayScene : public UFICRuntimeProcess {
	GENERATED_BODY()
protected:
	FICFrameFloat Progress = 0.0f;
	
public:
	UPROPERTY()
	AFICScene* Scene = nullptr;

	UPROPERTY()
	bool bBackground = false;
	
	// Begin UFICRuntimeProcess
	virtual bool NeedsRuntimeProcessCharacter() { return true; }
	virtual void Initialize() override;
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Shutdown() override;
	// End UFICRuntimeProcess
	
	FICFrameFloat GetProgress() { return Progress; }
};
