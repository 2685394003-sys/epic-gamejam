#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSwitch.generated.h"

class ULightComponent;
class UMeshComponent;
class UMaterialInstanceDynamic;

/** 单盏灯的记忆：组件 + 设计师设的目标强度 */
struct FLightEntry
{
	TWeakObjectPtr<ULightComponent> Light;
	float TargetIntensity = 0.0f;
};

/** 单个灯具的记忆：网格 + 动态材质实例（驱动自发光） */
struct FFixtureEntry
{
	TWeakObjectPtr<UMeshComponent> Mesh;
	TWeakObjectPtr<UMaterialInstanceDynamic> MID;
};

/**
 * 供电总开关。
 *
 * 用法：把本 Actor 放进关卡，Play 后按 E 切换（测试用）。
 * 正式版：把发电机/交互对象的回调直接连到 PowerRestored() / PowerLost() / TogglePower()，
 *         不用改本类任何逻辑。
 *
 * 断电态：PowerLight 全部熄灭，EmergencyLight 全亮，PowerFixture 自发光为 0。
 * 通电态：PowerLight 从 0 渐亮到各自原强度，EmergencyLight 渐灭，PowerFixture 自发光 0→1。
 */
UCLASS()
class DEMO_API APowerSwitch : public AActor
{
	GENERATED_BODY()

public:
	APowerSwitch();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// ---------------- 设计师可调参数（Details 面板） ----------------

	/** 通电后渐亮的灯 Tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags")
	FName PowerLightTag = FName("PowerLight");

	/** 断电时常亮的应急灯 Tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags")
	FName EmergencyLightTag = FName("EmergencyLight");

	/** 通电后渐亮自发光的灯具 Tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags")
	FName FixtureTag = FName("PowerFixture");

	/** 断电时自发光常亮的灯具 Tag（应急灯具，通电后反过来熄灭） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags")
	FName EmergencyFixtureTag = FName("EmergencyFixture");

	/**
	 * 灯具上哪个材质槽是「发光面」。
	 * LED 平板灯实测：0 = Door_002（外壳），1 = LED（发光面）⇒ 默认 1。
	 * 换别的灯具就改这个数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags", meta = (ClampMin = "0"))
	int32 EmissiveMaterialIndex = 1;

	/**
	 * 自发光参数名。
	 * 默认用 UE 内置的 "EmissiveBoost"（乘算自发光，0=不发光）⇒ 不用改材质。
	 * 若想自己控制曲线，可在材质里加一个同名标量参数（如 Glow）连到 Emissive，再把这里改成那个名字。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Tags")
	FName EmissiveParamName = FName("EmissiveBoost");

	/** 渐亮 / 渐灭总时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Fade", meta = (ClampMin = "0.1", UIMax = "10.0"))
	float FadeDuration = 1.5f;

	/** 初始是否断电（true = 主灯全灭 + 应急灯亮） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power|Fade")
	bool bStartUnpowered = true;

	// ---------------- 对外接口：发电机将来直接调这三个 ----------------

	UFUNCTION(BlueprintCallable, Category = "Power")
	void PowerRestored();

	UFUNCTION(BlueprintCallable, Category = "Power")
	void PowerLost();

	UFUNCTION(BlueprintCallable, Category = "Power")
	void TogglePower();

	UFUNCTION(BlueprintCallable, Category = "Power")
	bool IsPowered() const { return bPowered; }

private:
	bool bPowered = false;
	bool bInputBound = false;

	/** 0 = 全断，1 = 全通；Tick 里向 FadeTarget 插值 */
	float FadeT = 0.0f;
	float FadeTarget = 0.0f;

	TArray<FLightEntry> PowerLights;
	TArray<FLightEntry> EmergencyLights;
	TArray<FFixtureEntry> Fixtures;
	TArray<FFixtureEntry> EmergencyFixtures;

	void GatherLights(FName Tag, TArray<FLightEntry>& OutLights);
	void GatherFixtures(FName Tag, TArray<FFixtureEntry>& OutFixtures);
	bool TryBindInput();
	void ApplyFade(float T) const;
};
