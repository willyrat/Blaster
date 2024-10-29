// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//lesson 202 take this check out
	//if (!HasAuthority()) return;	//we check to if we are on the server and only run when on server...so this is safe to call on client, nothing will happen

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleMeshSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();

	if (MuzzleMeshSocket && World)
	{
		FTransform SocketTransform = MuzzleMeshSocket->GetSocketTransform(GetWeaponMesh());
		//from muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();		//gets owner of weapon which sets its owner to Character
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;


		if (bUserServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority())
			{
				if (InstigatorPawn->IsLocallyControlled())	//Server, host -  user replicated projectile
				{
					//spawn projectile
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage

				}
				else //Server, not locally controlled - spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = true; //should not cause damage on server
				}
			}
			else //client, using SSR
 			{
				if (InstigatorPawn->IsLocallyControlled()) //Client - locally controlled - spawn non-replicated projectile, use SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage
				}
				else //client - not locally controlled...spawn non-replicated projectile, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
				}
				
			}
		}
		else //weapon not using SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				//in this case, all server characters are going to spawn replicated projectiles
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUserServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;	//no longer setting damage on the projectile blueprint or in the projectile class... now setting to weapon's that is firing damage

			}


		}

	}

}
 