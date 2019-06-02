// Copyright Chris Parello 2019

#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Brush.h"
#include "ConstructorHelpers.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Grabber is reporting for duty"));

	FindPhysicsHandleComponent();
	
	FindGrabberInputComponent();
}

void UGrabber::FindGrabberInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent) {
		//phys handle found
		//UE_LOG(LogTemp, Warning, TEXT("InputComponent found "));
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Release", IE_Released, this, &UGrabber::Release);
		InputComponent->BindAction("Throw", IE_Pressed, this, &UGrabber::Throw);
		InputComponent->BindAction("Spawn", IE_Pressed, this, &UGrabber::Spawn);
		InputComponent->BindAction("TractorBeam", IE_Pressed, this, &UGrabber::TractorBeam);

	}
	else {
		UE_LOG(LogTemp, Error, TEXT(" %s is missing InputComponent "), *GetOwner()->GetName());
	}
}

void UGrabber::FindPhysicsHandleComponent()
{
	/// Look for attached Physics Handle
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr) {
		//phys handle not found
		UE_LOG(LogTemp, Error, TEXT(" %s is missing physics handle component "), *GetOwner()->GetName());
	}
}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!PhysicsHandle){ return; }
	//if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent) {
		// move the object attached
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}


	if (InputPressed.Equals(TEXT("Tractor")))
	{
// 		if(ActorBeingTractored)
// 		{
// 			// Interp to the current location of the Player:
// 			FVector CurrentLocation = GetOwner()->GetActorLocation();
// 			float WorldDeltaTime = GetWorld()->DeltaTimeSeconds;
// 			FVector MovementStep = FMath::VInterpTo(ActorToMoveLocation, CurrentLocation, WorldDeltaTime,
// 				DefaultMovementTime);
// 			FRotator RotationStep = UKismetMathLibrary::FindLookAtRotation(ActorToMoveLocation, CurrentLocation);
// 			ActorBeingTractored->SetActorLocationAndRotation(MovementStep, RotationStep);
// 		}
	}
}

FVector UGrabber::GetReachLineStart()
{
	FVector PlayerViewPoint;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPoint, OUT PlayerViewPointRotation);
	//TODO log out test
	//UE_LOG(LogTemp, Warning, TEXT("Viewpoint %s Rotation %s"), *PlayerViewPoint.ToString(), *PlayerViewPointRotation.ToString());
	return PlayerViewPoint;
}

FVector UGrabber::GetReachLineEnd()
{
	float LineEnd;
	if(InputPressed.Equals(TEXT("Tractor")))
	{
		LineEnd = TractorReach;
	}else
	{
		LineEnd = Reach;
	}
	
	FVector PlayerViewPoint;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPoint, OUT PlayerViewPointRotation);
	//TODO log out test
	//UE_LOG(LogTemp, Warning, TEXT("Viewpoint %s Rotation %s"), *PlayerViewPoint.ToString(), *PlayerViewPointRotation.ToString());
	return PlayerViewPoint + PlayerViewPointRotation.Vector() * LineEnd;
}

void UGrabber::Grab()
{
	InputPressed = TEXT("Grab");
	UE_LOG(LogTemp, Warning, TEXT("Grab pressed"));

	// line trace try to reach out and grab any actors with physics body collision channel set
	HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();

	if (!PhysicsHandle) { return; }
	//if we hit attach
	if (ActorHit) {
		//TODO attach physics handle
		PhysicsHandle->GrabComponentAtLocation(ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation()); //allow rotation
		bIsObjectGrabbed = true;
	}
	
}

void UGrabber::TractorBeam()
{
	InputPressed = TEXT("Tractor");

	UE_LOG(LogTemp, Warning, TEXT("TractorBeam pressed"));

	// line trace try to reach out and grab any actors with physics body collision channel set
	HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToMove = HitResult.GetComponent();
	ActorBeingTractored = HitResult.GetActor();

	if(ComponentToMove)
	{
		ActorToMoveLocation = ComponentToMove->GetOwner()->GetActorLocation();
		if (ActorBeingTractored)
		{
			// Interp to the current location of the Player:
			FVector CurrentLocation = GetOwner()->GetActorLocation();
			float WorldDeltaTime = GetWorld()->DeltaTimeSeconds;
			FVector MovementStep = FMath::VInterpTo(ActorToMoveLocation, CurrentLocation, WorldDeltaTime,
				DefaultMovementTime);
			FRotator RotationStep = UKismetMathLibrary::FindLookAtRotation(ActorToMoveLocation, CurrentLocation);
			ActorBeingTractored->SetActorLocationAndRotation(MovementStep, RotationStep);
		}
	}
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Warning, TEXT("Release pressed"));
	if (!PhysicsHandle) { return; }
	//TODO release physics handle
	PhysicsHandle->ReleaseComponent();
}

void UGrabber::Throw()
{
	auto ComponentGrabbed = HitResult.GetComponent();
	int rate = 1000; /// Rate at which the impulse is applied.

	/// Checking if the object is first grabbed before throwing.
	if (HitResult.GetActor() && bIsObjectGrabbed)
	{
		PhysicsHandle->ReleaseComponent();

		FVector PlayerViewPoint;
		FRotator PlayerViewPointRotation;
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPoint, OUT PlayerViewPointRotation);

		
	//PhysicsHandle->GetOwner()->GetActorForwardVector()
		/// (Using player's forward vector to determine direction of throw).
		ComponentGrabbed->AddImpulse(
			PlayerViewPoint + PlayerViewPointRotation.Vector() * rate,
			NAME_None, /// No bone names for specific  objects.
			true /// Makes sure that mass is immaterial to the force.
		);

		bIsObjectGrabbed = false;
	}
}

void UGrabber::Spawn()
{

	UE_LOG(LogTemp, Warning, TEXT("Spawn pressed"));

	// line trace try to reach out and grab any actors with physics body collision channel set
	HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToSpawn = HitResult.GetComponent();
	auto ActorToSpawn = HitResult.GetActor();

	if(ComponentToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ComponentToSpawn %s"), *ComponentToSpawn->GetName());
	}

	if(ActorToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn pressed %s"), *ActorToSpawn->GetName());

		FVector Location(0.0f, 0.0f, 150.0f);
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		

		SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorToSpawn->GetClass(), Location, Rotation, SpawnInfo);
		UStaticMeshComponent* MyMeshComponent = NewObject<UStaticMeshComponent>(SpawnedActor, UStaticMeshComponent::StaticClass(), TEXT("Mesh"));

		UStaticMesh* MeshAsset = nullptr;
		FString Chair = TEXT("Chair");
		if (ActorToSpawn->GetName().Contains(Chair)) {
			MeshAsset = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TEXT("StaticMesh'/Game/StarterContent/Props/SM_Chair.SM_Chair'")));
		}
		FString Table = TEXT("Table");
		if (ActorToSpawn->GetName().Contains(Table)) {
			MeshAsset = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TEXT("StaticMesh'/Game/StarterContent/Props/SM_TableRound.SM_TableRound'")));
		}
		
		//UStaticMesh* MeshAsset = Cast<UStaticMesh>(ComponentToSpawn);
		//UMaterial* MaterialAsset = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("Material'/Game/Weapons/axes/doubleaxe02c_Mat.doubleaxe02c_Mat'")));

		if(MyMeshComponent && MeshAsset)
		{
			MyMeshComponent->SetStaticMesh(MeshAsset);
			//MyMeshComponent->SetMaterial(0, MaterialAsset);
			MyMeshComponent->SetWorldLocation(Location);
			MyMeshComponent->SetIsReplicated(true);


			MyMeshComponent->RegisterComponent();

			if(SpawnedActor)
			{
				SpawnedActor->AddOwnedComponent(MyMeshComponent);
				SpawnedActor->SetRootComponent(MyMeshComponent);
				MyMeshComponent->SetNotifyRigidBodyCollision(true);
				MyMeshComponent->SetMobility(EComponentMobility::Movable);
				MyMeshComponent->SetSimulatePhysics(true);
				MyMeshComponent->SetMassOverrideInKg(NAME_None, 15.0f, true);
				MyMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MyMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);

			}
		}
	}



	
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	// ray-cast (line trace) out to reach distance
	DrawDebugLine(GetWorld(), GetReachLineStart(), GetReachLineEnd(), FColor(255,0,0), false, 0.0f, 0.0f, 10.0f);

	FHitResult LineTraceHit;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	GetWorld()->LineTraceSingleByObjectType(OUT LineTraceHit, GetReachLineStart(), GetReachLineEnd(), FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), TraceParameters);
	//see what we hit
	AActor* ActorHit = LineTraceHit.GetActor();

	if (ActorHit) {
		UE_LOG(LogTemp, Warning, TEXT("We Hit %s"), (*ActorHit->GetName()));
	}

	return LineTraceHit;
}

