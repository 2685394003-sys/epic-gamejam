#include "SlidingDoor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ASlidingDoor::ASlidingDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	Panel->SetupAttachment(RootComponent);
	Panel->SetCollisionProfileName(TEXT("BlockAll"));
}

void ASlidingDoor::BeginPlay()
{
	Super::BeginPlay();

	bOpen = bStartOpen;
	Alpha = bOpen ? 1.0f : 0.0f;
	AlphaTarget = Alpha;

	// 记住门扇的初始（关闭）位置，之后所有滑动都相对它算
	ClosedLocation = Panel->GetRelativeLocation();
	ApplyAlpha(Alpha);

	UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s init open=%d locked=%d offset=%s dur=%.2f"),
		*GetName(), bOpen ? 1 : 0, bLocked ? 1 : 0, *OpenOffset.ToString(), SlideDuration);
}

void ASlidingDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FMath::IsNearlyEqual(Alpha, AlphaTarget, 0.001f))
	{
		return;
	}

	const float Step = (SlideDuration > KINDA_SMALL_NUMBER) ? (DeltaSeconds / SlideDuration) : 1.0f;
	Alpha = (AlphaTarget > Alpha)
		? FMath::Min(AlphaTarget, Alpha + Step)
		: FMath::Max(AlphaTarget, Alpha - Step);

	ApplyAlpha(Alpha);
}

void ASlidingDoor::ApplyAlpha(float InAlpha)
{
	// 缓入缓出，避免突然启停
	const float Eased = FMath::InterpEaseInOut(0.0f, 1.0f, InAlpha, 2.0f);
	Panel->SetRelativeLocation(ClosedLocation + OpenOffset * Eased);
}

void ASlidingDoor::Open()
{
	if (bLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s is LOCKED, open refused"), *GetName());
		return;
	}

	if (bOpen)
	{
		return;
	}

	bOpen = true;
	AlphaTarget = 1.0f;
	UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s opening over %.2fs"), *GetName(), SlideDuration);

	// 联动门一起开（电梯双扇）
	for (ASlidingDoor* Other : LinkedDoors)
	{
		if (Other && Other != this)
		{
			Other->bLocked = false;
			Other->Open();
		}
	}
}

void ASlidingDoor::Close()
{
	if (!bOpen)
	{
		return;
	}

	bOpen = false;
	AlphaTarget = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s closing"), *GetName());

	for (ASlidingDoor* Other : LinkedDoors)
	{
		if (Other && Other != this)
		{
			Other->Close();
		}
	}
}

void ASlidingDoor::Interact()
{
	if (bLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s is LOCKED (needs key / power)"), *GetName());
		return;
	}

	if (bOpen)
	{
		Close();
	}
	else
	{
		Open();
	}
}

void ASlidingDoor::Unlock()
{
	bLocked = false;
	UE_LOG(LogTemp, Log, TEXT("[SlidingDoor] %s unlocked"), *GetName());
}
