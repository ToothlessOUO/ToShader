// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ToWeather.generated.h"

// 月相枚举（8种主要月相）
UENUM(BlueprintType)
enum class EMoonPhase : uint8
{
	NewMoon,        // 新月
	WaxingCrescent, // 蛾眉月
	FirstQuarter,   // 上弦月
	WaxingGibbous,  // 盈凸月
	FullMoon,       // 满月
	WaningGibbous,  // 亏凸月
	LastQuarter,    // 下弦月
	WaningCrescent  // 残月
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOSHADER_API UToWeather : public UActorComponent
{
	GENERATED_BODY()

public:
	UToWeather();
	
	UPROPERTY(EditAnywhere,meta=(
		ToolTip="若不启用，灯光角度将能够自由控制"))
	bool bEnableTOD = true;
	UPROPERTY(EditAnywhere,meta=(
		EditCondition="bEnableTOD",ClampMin="0.0",ClampMax="24.0"))
	float TODTime = 14.5;
	
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	float SunIntensity = 4.5;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	FLinearColor SunColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	float SunShadowAmount = 0.55;

	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonOffsetHours = 0.83f;// 月亮比太阳滞后约50分钟（0.83小时）
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonIntensity = 2.5;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	FLinearColor MoonColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonShadowAmount = 0.55;


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float LastTODTime;
	bool bInitSuccess = false;
	TObjectPtr<UDirectionalLightComponent> Sun;
	TObjectPtr<UDirectionalLightComponent> Moon;
	TObjectPtr<USkyLightComponent> SkyLight;
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	void Init();
	
	// 杭州的经纬度（30.25°N, 120.16°E）
	static constexpr float HangzhouLatitude = 30.25f;
	static constexpr float HangzhouLongitude = 120.16f;

	void RunTOD();
#pragma region 昼夜计算
	// 固定每日的太阳位置计算（忽略年/月变化）
	void CalculateFixedSunPosition(
		float TimeOfDay,
		float Latitude,
		float Longitude,
		float& OutAltitude,
		float& OutAzimuth
	);

	// 固定每日的月亮位置计算（基于太阳位置 + 固定偏移）
	void CalculateFixedMoonPosition(
		float TimeOfDay,
		float Latitude,
		float Longitude,
		float& OutAltitude,
		float& OutAzimuth
	);

	// 转换高度角/方位角为UE旋转
	static FRotator AltAzToRotator(float Altitude, float Azimuth);
	
	/**
	* 计算太阳和月亮的旋转角度（固定每日角度，仅依赖时间）
	* @param TimeOfDay 当日时间（小时，0~24）
	* @param Latitude 纬度（-90~90）
	* @param Longitude 经度（-180~180）
	* @param OutSunRotation 太阳光源旋转
	* @param OutMoonRotation 月亮光源旋转
	*/
	void CalculateFixedDaySolarMoonRotation(
		float TimeOfDay,
		float Latitude,
		float Longitude,
		FRotator& OutSunRotation,
		FRotator& OutMoonRotation
	);

	/**
	 * 计算当前月相（基于日期，与时间无关）
	 * @param Date 输入日期（年月日）
	 * @return 月相枚举值
	 */
	static EMoonPhase CalculateMoonPhase(const FDateTime& Date);

    /**
     * 基于高度角的昼夜强度过渡
     * @param TimeOfDay 时间（小时）
     * @param Latitude 纬度
     * @param Longitude 经度
     * @param TransitionAngle 过渡角度范围（度，如5°）
     * @param OutSunRotation 太阳旋转
     * @param OutMoonRotation 月亮旋转
     * @param OutSunIntensity 太阳强度系数（0~1）
     * @param OutMoonIntensity 月亮强度系数（0~1）
     */
    UFUNCTION(BlueprintCallable, Category = "Astronomy|Lighting")
    void CalculateSolarMoonWithIntensity(
        float TimeOfDay,
        float Latitude,
        float Longitude,
        float TransitionAngle,
        FRotator& OutSunRotation,
        FRotator& OutMoonRotation,
        float& OutSunIntensity,
        float& OutMoonIntensity
    );
#pragma endregion
};
