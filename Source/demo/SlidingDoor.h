#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlidingDoor.generated.h"

class UStaticMeshComponent;

/**
 * 滑动门（仓库门 / 电梯门 通用）。
 *
 * 行为：调用 Open() 后门扇沿 OpenOffset 平滑滑开，【不会自动关上】，直到有人调 Close()。
 * - 仓库门：OpenOffset = (120, 0, 0)  ⇒ 向右滑 120cm
 * - 电梯双扇：两扇各放一个本 Actor，互相拖进 LinkedDoors；一个 Open() 两扇一起开
 *
 * 上锁：bLocked = true 时 Open() 不动作（留给你后面接"钥匙 / 门禁 / 通电"）
 *       条件满足时调 Unlock() 即可，不需要改本类代码。
 *
 * 按键：E 键目前不自动绑定（等交互框架统一处理），先由按钮/蓝图调 Interact()。
 */
UCLASS()
class DEMO_API ASlidingDoor : public AActor
{
	GENERATED_BODY()

public:
	ASlidingDoor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	/** 门扇：在 Details 里指定网格与材质 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Panel;

	/** 开启时的位移（厘米）。向右 = (+X, 0, 0)；向左 = (-X, 0, 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FVector OpenOffset = FVector(120.0f, 0.0f, 0.0f);

	/** 滑动时长（秒）。内部用缓入缓出，不会突然启停 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (ClampMin = "0.1", UIMax = "5.0"))
	float SlideDuration = 1.5f;

	/** 初始就是开着的 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bStartOpen = false;

	/** 上锁：锁着时 Open() / Interact() 不动作 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bLocked = false;

	/** 联动门（电梯双扇：把另一扇拖进来，一次 Open() 两扇都开） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TArray<TObjectPtr<ASlidingDoor>> LinkedDoors;

	// ---------------- 对外接口 ----------------

	/** 开门（条件满足后由按钮/钥匙/发电机调用） */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Open();

	/** 关门 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Close();

	/** 开/关切换（将来 E 键交互调它） */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Interact();

	/** 解锁（钥匙 / 门禁 / 通电 满足时调） */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Unlock();

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return bOpen; }

private:
	bool bOpen = false;

	/** 0 = 全关，1 = 全开 */
	float Alpha = 0.0f;
	float AlphaTarget = 0.0f;

	FVector ClosedLocation;

	void ApplyAlpha(float InAlpha);
};
