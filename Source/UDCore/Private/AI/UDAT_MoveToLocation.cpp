// © 2024 Unreal Directive. All rights reserved.


#include "AI/UDAT_MoveToLocation.h"

#include "UDCoreLogChannels.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


UUDAT_MoveToLocation* UUDAT_MoveToLocation::MoveToLocation(
	UObject* WorldContextObject,
	AController* Controller,
	const FVector Destination,
	const float AcceptanceRadius,
	const bool bDebugLineTrace)
{
	UUDAT_MoveToLocation* Action = NewObject<UUDAT_MoveToLocation>();
	Action->Controller = Controller;
	Action->Destination = Destination;
	Action->AcceptanceRadius = AcceptanceRadius;
	Action->bDebugLineTrace = bDebugLineTrace;

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
		// Controller or pawn has been destroyed
		ExecuteCompleted(false);
		UE_LOG(LogUDCore, Warning, TEXT("Controller or pawn has been destroyed while moving to location. Aborting."));
		return;
	}

	// Cache the start location, so we can check if the controller has moved
	StartLocation = Controller->GetPawn()->GetActorLocation();

	// Start a timer to check if the controller has moved to the destination
	Controller->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUDAT_MoveToLocation::CheckMoveToLocation, 0.1f, true);

	// Start a timer to check if the controller is stuck
	Controller->GetWorld()->GetTimerManager().SetTimer(StuckTimerHandle, this, &UUDAT_MoveToLocation::CheckStuckMovement, 3.0f, true);
	
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
	UE_LOG(LogUDCore, Verbose, TEXT("Moving controller to location."));

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
		// Controller or pawn has been destroyed
		ExecuteCompleted(false);
		UE_LOG(LogUDCore, Warning, TEXT("Controller or pawn has been destroyed while moving to location. Aborting."));
		return;
	}
	
	// Get controllers pawns world location
	CurrentLocation = Controller->GetPawn()->GetActorLocation();
	UE_LOG(LogUDCore, Log, TEXT("Controller is moving to location (%s)."), *CurrentLocation.ToString());
	UE_LOG(LogUDCore, Log, TEXT("Distance to destination: %f."), FVector::Dist(CurrentLocation, Destination));
	if (FVector::Dist(CurrentLocation, Destination) < AcceptanceRadius)
	{
		// Movement has completed
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

	if (FVector::Dist(CurrentLocation, StartLocation) < 1.0f)
	{
		// Controller is stuck
		UE_LOG(LogUDCore, Warning, TEXT("Controller is stuck while moving to location. Aborting"));
		ExecuteCompleted(false);
	}
}

void UUDAT_MoveToLocation::ExecuteCompleted(const bool bSuccess)
{

	UE_LOG(LogUDCore, Log, TEXT("Movement to location completed. Success: %s."), bSuccess ? TEXT("true") : TEXT("false"));
	
	// Clear the timers
	Controller->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Controller->GetWorld()->GetTimerManager().ClearTimer(StuckTimerHandle);

	// Broadcast the completion event
	Completed.Broadcast(bSuccess);

	// Reset the controller and destination
	Controller = nullptr;
	Destination = FVector::ZeroVector;

	// Set the action to be ready to destroy
	SetReadyToDestroy();
}

