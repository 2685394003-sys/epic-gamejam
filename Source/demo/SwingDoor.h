#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwingDoor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * 平开门（绕铰链旋转）。
 *
 * 与 ASlidingDoor 的分工：
 *   - ASlidingDoor：平移滑动（仓库门 / 电梯门）
 *   - ASwingDoor  ：绕铰链转动（诊断室门 / 房间门）—— 本类
 *
 * 结构（重要）：
 *   Root = Pivot（铰链轴，摆 Actor 时把轴心放在门的铰链边缘）
 *   └─ Panel（门板）—— 在蓝图子类里把 RelativeLocation 往门宽方向偏移「半个门宽」
 *
 * 交互：
 *   默认 bBindFKey = true，Tick 里轮询 F 键（按住的第一帧触发一次 Toggle）。
 *   开门后【保持开启】，不会自动关；再按一次 F 关门。
 *   bBindFKey 关掉后只保留 Open()/Close()/Interact() 接口，交给外部交互框架调。
 *
 * 上锁：bLocked = true 时 F 键与 Interact() 都不动作，Unlock() 后恢复。
 */
UCLASS()
class DEMO_API ASwingDoor : public AActor
{
	GENERATED_BODY()

public:
	ASwingDoor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	/** 铰链轴（Root）。旋转发生在这个组件上 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<USceneComponent> Pivot;

	/** 门板：在 Details 里指定网格与材质，并把相对位置偏移半个门宽 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Panel;

	/** 开启角度（度）。往房间里开还是往走廊开，靠正负 + bInvertDirection 决定 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float OpenAngle = 100.0f;

	/** 转动时长（秒）。内部用缓入缓出，不会突然启停 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (ClampMin = "0.1", UIMax = "5.0"))
	float SwingDuration = 1.0f;

	/** 反向开。勾上则往 -Yaw 转 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bInvertDirection = false;

	/** 初始就是开着的 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bStartOpen = false;

	/** 上锁：锁着时 F 键 / Interact() 不动作 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bLocked = false;

	// ---------------- 交互 ----------------

	/** 是否由本类自己绑定 F 键。关掉后交给外部交互框架调 Interact() */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Interact")
	bool bBindFKey = true;

	/** 可交互距离（厘米，平面距离） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Interact", meta = (ClampMin = "0.0"))
	float InteractRange = 220.0f;

	// ---------------- 对外接口 ----------------

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Open();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Close();

	/** 开/关切换（F 键按下时调它） */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Unlock();

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return bOpen; }

private:
	bool bOpen = false;

	/** 0 = 全关，1 = 全开 */
	float Alpha = 0.0f;
	float AlphaTarget = 0.0f;

	/** 上一帧 F 是否按着（边沿检测，避免按住时每帧翻转） */
	bool bPrevFDown = false;

	void ApplyAlpha(float InAlpha);
	void HandleFKeyInput();
};
