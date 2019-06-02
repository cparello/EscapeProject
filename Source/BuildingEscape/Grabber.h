// Copyright Chris Parello 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/InputComponent.h"
#include "Grabber.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUILDINGESCAPE_API UGrabber : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabber();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void FindGrabberInputComponent();

	void FindPhysicsHandleComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//returns the start of reach line
	FVector GetReachLineStart();

	//returns current end of reach line
	FVector GetReachLineEnd();

private:
	FString InputPressed;


	//how far ahead of the player we can reach
	float Reach = 100.0f;

	UPhysicsHandleComponent* PhysicsHandle = nullptr;

	float TractorReach = 1000.0f;

	AActor* ActorBeingTractored;
	FVector ActorToMoveLocation;
	float DefaultMovementTime = 10.0f;

	//UPhysicsHandleComponent* PhysicsHandle = nullptr;

	UInputComponent* InputComponent = nullptr;

	//ray cast and grab whats in reach
	void Grab();

	void TractorBeam();

	//release whats grabbed
	void Release();
	void Throw();

	void Spawn();

	UPROPERTY()
	AActor* SpawnedActor;

	const FHitResult GetFirstPhysicsBodyInReach();

	FHitResult HitResult;

	bool bIsObjectGrabbed = false;
};
