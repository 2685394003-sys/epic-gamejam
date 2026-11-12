#include "SwingDoor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

ASwingDoor::ASwingDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 铰链轴作为 Root：旋转它就等于门绕铰链转
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	RootComponent = Pivot;

	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	Panel->SetupAttachment(Pivot);
	Panel->SetCollisionProfileName(TEXT("BlockAll"));
}

void ASwingDoor::BeginPlay()
{
	Super::BeginPlay();

	bOpen = bStartOpen;
	Alpha = bOpen ? 1.0f : 0.0f;
	AlphaTarget = Alpha;
	ApplyAlpha(Alpha);

	UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s init open=%d locked=%d angle=%.1f dur=%.2f bindF=%d"),
		*GetName(), bOpen ? 1 : 0, bLocked ? 1 : 0, OpenAngle, SwingDuration, bBindFKey ? 1 : 0);
}

void ASwingDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleFKeyInput();

	if (FMath::IsNearlyEqual(Alpha, AlphaTarget, 0.001f))
	{
		return;
	}

	const float Step = (SwingDuration > KINDA_SMALL_NUMBER) ? (DeltaSeconds / SwingDuration) : 1.0f;
	Alpha = (AlphaTarget > Alpha)
		? FMath::Min(AlphaTarget, Alpha + Step)
		: FMath::Max(AlphaTarget, Alpha - Step);

	ApplyAlpha(Alpha);
}

void ASwingDoor::ApplyAlpha(float InAlpha)
{
	// 缓入缓出，避免突然启停
	const float Eased = FMath::InterpEaseInOut(0.0f, 1.0f, InAlpha, 2.0f);
	const float Sign = bInvertDirection ? -1.0f : 1.0f;
	Pivot->SetRelativeRotation(FRotator(0.0f, OpenAngle * Eased * Sign, 0.0f));
}

void ASwingDoor::HandleFKeyInput()
{
	if (!bBindFKey)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	const bool bFDown = PC->IsInputKeyDown(EKeys::F);
	const bool bJustPressed = bFDown && !bPrevFDown;
	bPrevFDown = bFDown;

	if (!bJustPressed)
	{
		return;
	}

	// 只在玩家靠近时才响应，避免隔着墙也能按
	if (APawn* Pawn = PC->GetPawn())
	{
		const float Dist = FVector::Dist2D(Pawn->GetActorLocation(), GetActorLocation());
		if (Dist > InteractRange)
		{
			return;
		}
	}
	else
	{
		return;
	}

	Interact();
}

void ASwingDoor::Open()
{
	if (bLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s is LOCKED, open refused"), *GetName());
		return;
	}

	if (bOpen)
	{
		return;
	}

	bOpen = true;
	AlphaTarget = 1.0f;
	UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s opening over %.2fs"), *GetName(), SwingDuration);
}

void ASwingDoor::Close()
{
	if (!bOpen)
	{
		return;
	}

	bOpen = false;
	AlphaTarget = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s closing"), *GetName());
}

void ASwingDoor::Interact()
{
	if (bLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s is LOCKED (needs key / power)"), *GetName());
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

void ASwingDoor::Unlock()
{
	bLocked = false;
	UE_LOG(LogTemp, Log, TEXT("[SwingDoor] %s unlocked"), *GetName());
}
