// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
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


UCLASS(Blueprintable)
class TOSHADER_API UTODDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunriseStartTime = 5.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunriseEndTime = 6.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunsetStartTime = 17.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunsetEndTime = 18.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoonOffsetHours = 12;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Latitude = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULevelSequence* TODSequence = nullptr;
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
	UPROPERTY(EditAnywhere,Interp,meta=(
		EditCondition="bEnableTOD && !bAnimTOD",ClampMin="0.0",ClampMax="24.0"))
	float TODTime = 14.5;
	UPROPERTY(EditAnywhere,meta=(
		EditCondition="bEnableTOD",ClampMin="0.0",ClampMax="24.0"))
	float TODTimeWhenBeginPlay = 14.5;
	UPROPERTY(EditAnywhere,Category="TODAnim",meta=(
		EditCondition="bEnableTOD"))
	bool bAnimTOD = false;
	UPROPERTY(EditAnywhere,Category="TODAnim",meta=(
		EditCondition="bEnableTOD && bAnimTOD"))
	float TODAnimScale = 1;
	UPROPERTY(EditAnywhere,Category="TODAnim",meta=(
		EditCondition="bEnableTOD && bAnimTOD"))
	UTODDataAsset* TODAsset = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	float SunIntensity = 4.5;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	FLinearColor SunColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	float SunShadowAmount = 0.55;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	FColor SunBloomColor = FColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	float SunBloomScale = 1.f;
	UPROPERTY(EditAnywhere, Category="Lighting|Sun",Interp)
	FColor SunMeshColor = FColor::White;
	
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonIntensity = 2.5;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	FLinearColor MoonColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonShadowAmount = 0.55;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	FColor MoonBloomColor = FColor::White;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	float MoonBloomScale = 1.f;
	UPROPERTY(EditAnywhere, Category="Lighting|Moon",Interp)
	FColor MoonMeshColor = FColor::White;
	
	UPROPERTY(EditAnywhere, Category="SkyAtmosphere",Interp)
	FLinearColor SkyBaseColor = FLinearColor(0.236926,0.74633,1,1);
	UPROPERTY(EditAnywhere, Category="SkyAtmosphere",Interp)
	FLinearColor FogColor = FLinearColor(0.170931,0.224525,0.224066,1);
	


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float LastTODTime;
	bool bInitSuccess = false;
	
	TObjectPtr<UDirectionalLightComponent> Sun;
	TObjectPtr<UStaticMeshComponent> SunMesh;
	TObjectPtr<UDirectionalLightComponent> Moon;
	TObjectPtr<UStaticMeshComponent> MoonMesh;
	TObjectPtr<USkyLightComponent> SkyLight;
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
	TObjectPtr<UStaticMeshComponent> SkySphere;
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;

	static constexpr int SkySphereBaseColorIndex = 0;//float3
	static constexpr int SunColorIndex = 0;//rgb 012
	static constexpr int SunAlphaIndex = 3;//alpha 3

	void Init();

	UPROPERTY()
	ALevelSequenceActor* SequenceActor;
	UPROPERTY()
	ULevelSequencePlayer* LevelSequencePlayer;
	
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
