// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);	//in blueprint this is called Simulation Generates Hit Events... make sure this is checked in bp if set true here
	ShellEjectionImpules = 10.f;

	DelayDestory = 3.f;
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->AddImpulse(GetActorForwardVector()* ShellEjectionImpules);

	if (HasAuthority())
	{
		CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	}
}



void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound && !TimerHandle.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	GetWorldTimerManager().SetTimer(TimerHandle, this, &ACasing::DestroyMe, DelayDestory, false, -1.f);

	//Destroy();
}

void ACasing::DestroyMe()
{
	TimerHandle.Invalidate();
	Destroy();
}








