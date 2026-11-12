#include "PowerSwitch.h"

#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

APowerSwitch::APowerSwitch()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void APowerSwitch::BeginPlay()
{
	Super::BeginPlay();

	// 1) 收集三组对象（灯 / 应急灯 / 灯具），并记住各自的目标强度
	GatherLights(PowerLightTag, PowerLights);
	GatherLights(EmergencyLightTag, EmergencyLights);
	GatherFixtures(FixtureTag, Fixtures);
	GatherFixtures(EmergencyFixtureTag, EmergencyFixtures);

	// 2) 按初始状态落位
	bPowered = !bStartUnpowered;
	FadeT = bPowered ? 1.0f : 0.0f;
	FadeTarget = FadeT;
	ApplyFade(FadeT);

	// 3) 绑测试按键
	TryBindInput();

	UE_LOG(LogTemp, Log, TEXT("[PowerSwitch] init powered=%d | powerLights=%d emergencyLights=%d fixtures=%d emergencyFixtures=%d"),
		bPowered ? 1 : 0, PowerLights.Num(), EmergencyLights.Num(), Fixtures.Num(), EmergencyFixtures.Num());
}

void APowerSwitch::GatherLights(FName Tag, TArray<FLightEntry>& OutLights)
{
	OutLights.Reset();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(this, Tag, Actors);

	for (AActor* LightActor : Actors)
	{
		if (LightActor == nullptr)
		{
			continue;
		}

		// 注意：用 ULightComponent（不是 ULightComponentBase）—— SetIntensity() 只在这个子类上
		ULightComponent* LightComp = Cast<ULightComponent>(
			LightActor->GetComponentByClass(ULightComponent::StaticClass()));

		if (LightComp == nullptr)
		{
			continue;
		}

		FLightEntry Entry;
		Entry.Light = LightComp;
		// 记住设计师在关卡里设的目标强度，渐亮时就渐到它
		Entry.TargetIntensity = LightComp->Intensity;
		OutLights.Add(Entry);
	}
}

void APowerSwitch::GatherFixtures(FName Tag, TArray<FFixtureEntry>& OutFixtures)
{
	OutFixtures.Reset();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(this, Tag, Actors);

	for (AActor* FixtureActor : Actors)
	{
		if (FixtureActor == nullptr)
		{
			continue;
		}

		UMeshComponent* Mesh = Cast<UMeshComponent>(
			FixtureActor->GetComponentByClass(UMeshComponent::StaticClass()));

		if (Mesh == nullptr)
		{
			continue;
		}

		FFixtureEntry Entry;
		Entry.Mesh = Mesh;

		// 动态材质实例：用来驱动自发光参数（材质里要有 EmissiveParamName 同名标量）
		// 注意：只处理「发光面」那一个材质槽（LED 平板灯是槽 1），外壳槽不动
		if (UMaterialInterface* SourceMat = Mesh->GetMaterial(EmissiveMaterialIndex))
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(SourceMat, this))
			{
				Mesh->SetMaterial(EmissiveMaterialIndex, MID);
				MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				Entry.MID = MID;
			}
		}

		OutFixtures.Add(Entry);
	}
}

bool APowerSwitch::TryBindInput()
{
	if (bInputBound)
	{
		return true;
	}

	// 玩家 Pawn 可能比本 Actor 稍晚才 Possess，所以 Tick 里会重试
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UInputComponent* InputComp = PlayerPawn->InputComponent)
		{
			InputComp->BindKey(EKeys::E, EInputEvent::IE_Pressed, this, &APowerSwitch::TogglePower);
			bInputBound = true;
			UE_LOG(LogTemp, Log, TEXT("[PowerSwitch] E key bound to player pawn"));
			return true;
		}
	}
	return false;
}

void APowerSwitch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInputBound)
	{
		TryBindInput();
	}

	if (FMath::IsNearlyEqual(FadeT, FadeTarget, KINDA_SMALL_NUMBER))
	{
		return;
	}

	// 线性插值到目标（1.5s 走完 0→1）
	const float Step = (FadeDuration > KINDA_SMALL_NUMBER) ? (DeltaSeconds / FadeDuration) : 1.0f;
	FadeT = (FadeTarget > FadeT) ? FMath::Min(FadeTarget, FadeT + Step)
	                             : FMath::Max(FadeTarget, FadeT - Step);

	ApplyFade(FadeT);
}

void APowerSwitch::PowerRestored()
{
	if (bPowered && FadeTarget >= 1.0f)
	{
		return;
	}

	bPowered = true;
	FadeTarget = 1.0f;

	UE_LOG(LogTemp, Log, TEXT("[PowerSwitch] PowerRestored -> fading in over %.2fs"), FadeDuration);
}

void APowerSwitch::PowerLost()
{
	if (!bPowered && FadeTarget <= 0.0f)
	{
		return;
	}

	bPowered = false;
	FadeTarget = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("[PowerSwitch] PowerLost -> fading out over %.2fs"), FadeDuration);
}

void APowerSwitch::TogglePower()
{
	if (bPowered)
	{
		PowerLost();
	}
	else
	{
		PowerRestored();
	}
}

void APowerSwitch::ApplyFade(float T) const
{
	// 主灯：0 → 目标强度
	for (const FLightEntry& Entry : PowerLights)
	{
		if (Entry.Light.IsValid())
		{
			Entry.Light->SetIntensity(Entry.TargetIntensity * T);
		}
	}

	// 应急灯：目标强度 → 0（与主灯反向）
	for (const FLightEntry& Entry : EmergencyLights)
	{
		if (Entry.Light.IsValid())
		{
			Entry.Light->SetIntensity(Entry.TargetIntensity * (1.0f - T));
		}
	}

	// 主照明灯具自发光：0 → 1
	for (const FFixtureEntry& Entry : Fixtures)
	{
		if (Entry.MID.IsValid())
		{
			Entry.MID->SetScalarParameterValue(EmissiveParamName, T);
		}
	}

	// 应急灯具自发光：1 → 0（与主灯反相 —— 断电时亮、通电后灭）
	for (const FFixtureEntry& Entry : EmergencyFixtures)
	{
		if (Entry.MID.IsValid())
		{
			Entry.MID->SetScalarParameterValue(EmissiveParamName, 1.0f - T);
		}
	}
}
