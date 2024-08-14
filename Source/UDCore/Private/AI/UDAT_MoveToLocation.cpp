// © 2024 Unreal Directive. All rights reserved.


#include "AI/UDAT_MoveToLocation.h"
#include "UDCoreLogChannels.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"


UUDAT_MoveToLocation* UUDAT_MoveToLocation::MoveToLocation(
	UObject* WorldContextObject,
	AController* Controller,
	const FVector Destination,
	const float AcceptanceRadius,
	const bool bCheckStuckMovement,
	const float StuckThreshold,
	const bool bDebugLineTrace)
{
	UUDAT_MoveToLocation* Action = NewObject<UUDAT_MoveToLocation>();
	Action->Controller = Controller;
	Action->Destination = Destination;
	Action->AcceptanceRadius = AcceptanceRadius;
	Action->bDebugLineTrace = bDebugLineTrace;
	Action->StuckThreshold = StuckThreshold;
	Action->bCheckStuckMovement = bCheckStuckMovement;

	Action->RegisterWithGameInstance(WorldContextObject);

	return Action;
}


void UUDAT_MoveToLocation::EndTask()
{
	ExecuteCompleted(false);
}

void UUDAT_MoveToLocation::Activate()
{
	if(!Controller || !Controller->GetPawn())
	{
		ExecuteCompleted(false);
		UE_LOG(LogUDCore, Warning, TEXT("Controller or pawn has been destroyed while moving to location. Aborting."));
		return;
	}
	
	StartLocation = Controller->GetPawn()->GetActorLocation();
	LastCheckedLocation = StartLocation;
	
	Controller->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUDAT_MoveToLocation::CheckMoveToLocation, 0.1f, true);

	if (bCheckStuckMovement)
	{
		Controller->GetWorld()->GetTimerManager().SetTimer(StuckTimerHandle, this, &UUDAT_MoveToLocation::CheckStuckMovement, 3.f, true);
	}
	
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
	UE_LOG(LogUDCore, Verbose, TEXT("Moving controller to location (%s)."), *Destination.ToString());

	if (bDebugLineTrace)
	{
		DrawDebugLine(
			Controller->GetWorld(),
			Destination + FVector(0, 0, 100),
			Destination,
			FColor::Green,
			false,
			5.0f,
			0,
			1.0f
		);
	}
}

void UUDAT_MoveToLocation::CheckMoveToLocation()
{

	if(!Controller || !Controller->GetPawn())
	{
		ExecuteCompleted(false);
		UE_LOG(LogUDCore, Warning, TEXT("Controller or pawn has been destroyed while moving to location. Aborting."));
		return;
	}
	
	CurrentLocation = Controller->GetPawn()->GetActorLocation();
	UE_LOG(LogUDCore, Verbose, TEXT("Controller is moving to location (%s). Current distance: %f."), *Destination.ToString(), FVector::Dist(CurrentLocation, Destination));
	
	if (FVector::Dist(CurrentLocation, Destination) < AcceptanceRadius)
	{
		UE_LOG(LogUDCore, Verbose, TEXT("Controller has moved to location."));
		ExecuteCompleted(true);
	}
}

void UUDAT_MoveToLocation::CheckStuckMovement()
{
	if(!Controller || !Controller->GetPawn())
	{
		// Controller or pawn has been destroyed
		ExecuteCompleted(false);
		UE_LOG(LogUDCore, Warning, TEXT("Controller or pawn has been destroyed while moving to location. Aborting."));
		return;
	}

	if (FVector::Dist(CurrentLocation, LastCheckedLocation) < StuckThreshold)
	{
		// Controller is stuck
		UE_LOG(LogUDCore, Warning, TEXT("Controller is stuck while moving to location. Aborting"));
		ExecuteCompleted(false);
	}

	LastCheckedLocation = CurrentLocation;
}

void UUDAT_MoveToLocation::ExecuteCompleted(const bool bSuccess)
{
	UE_LOG(LogUDCore, Log, TEXT("Movement to location completed. Success: %s."), bSuccess ? TEXT("true") : TEXT("false"));
	
	Controller->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Controller->GetWorld()->GetTimerManager().ClearTimer(StuckTimerHandle);
	
	Completed.Broadcast(bSuccess);
	
	Controller = nullptr;
	Destination = FVector::ZeroVector;
	
	SetReadyToDestroy();
}

