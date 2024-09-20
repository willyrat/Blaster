// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"
//#include "ProjectileMovementComponent.h"

//lesson 136 ...this is going to be used to stop the player shooting the rocket not causing the rocket to explode on them sometimes
//need to fully qualify this with URocketMovementComponent:: because EHandleBlockingHitResult is defined inside of UProjectileMovementComponent
URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	return EHandleBlockingHitResult::AdvanceNextSubstep;	//only want it to return this
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//rockets should not stop...only explode when their collision box detects a hit
}
