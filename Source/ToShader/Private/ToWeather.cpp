#include "ToWeather.h"

#include "ToShader.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "LevelSequencePlayer.h"
#include "Components/ExponentialHeightFogComponent.h"


UToWeather::UToWeather()
{
	//Enable tick in editor
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetTickGroup(TG_PostPhysics);
}

void UToWeather::BeginPlay()
{
	Super::BeginPlay();

	if (TODAsset && TODAsset->TODSequence)
	{
		FMovieSceneSequencePlaybackSettings PlaybackSettings;
		PlaybackSettings.bAutoPlay = true;
		PlaybackSettings.LoopCount.Value = -1;
		PlaybackSettings.StartTime = TODTimeWhenBeginPlay * 10 / 30;
		tolog("TODTime:", TODTimeWhenBeginPlay);
		PlaybackSettings.PlayRate = 1 / TODAnimScale;

		LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), TODAsset->TODSequence, PlaybackSettings, SequenceActor);
		if (LevelSequencePlayer)
		{
			LevelSequencePlayer->Play();
		}
		else
		{
			tolog("TODSequence Create Fail");
		}
	}
	else
	{
		tolog("No Valid TOD Sequence");
	}
}

void UToWeather::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Init();
	RunTOD();
}

void UToWeather::Init()
{
	if (!GetOwner()) return;
	if (bInitSuccess) return;
	TArray<AActor*> ChildActors;
	GetOwner()->GetAttachedActors(ChildActors,true,true);
	if (ChildActors.IsEmpty()) return;
	for (const auto ChildActor : ChildActors)
	{
		if (!ChildActor) continue;
		if (!Sun || !Moon)
		{
			if (auto l = ChildActor->FindComponentByClass<UDirectionalLightComponent>())
			{
				if (!Sun && l->ComponentHasTag("Sun")) Sun = l;
				if (!Moon && l->ComponentHasTag("Moon")) Moon = l;
			}
		}

		if (!SkyLight) SkyLight = ChildActor->FindComponentByClass<USkyLightComponent>();
		if (!SkyAtmosphere) SkyAtmosphere = ChildActor->FindComponentByClass<USkyAtmosphereComponent>();
		if (!HeightFog) HeightFog = ChildActor->FindComponentByClass<UExponentialHeightFogComponent>();

		//渲染Mesh
		if (!SunMesh)
		{
			if (auto Comp = ChildActor->FindComponentByClass<UStaticMeshComponent>(); Comp != nullptr && Comp->ComponentHasTag("SunMesh"))
				SunMesh = Comp;
		}
		if (!MoonMesh)
		{
			if (auto Comp = ChildActor->FindComponentByClass<UStaticMeshComponent>(); Comp != nullptr && Comp->ComponentHasTag("MoonMesh"))
				MoonMesh = Comp;
		}
		if (!SkySphere)
		{
			if (auto Comp = ChildActor->FindComponentByClass<UStaticMeshComponent>(); Comp != nullptr && Comp->ComponentHasTag("SkySphere"))
				SkySphere = Comp;
		}
	}
	if (!SunMesh) tolog("can't find sun mesh");
	if (Sun && Moon && SkyLight && SkyAtmosphere && HeightFog)
	{
		bInitSuccess = true;
		tolog("ToWeatherInitSuccess");

		Sun->SetUseTemperature(false);
		Sun->bAffectsWorld = true;
		Sun->BloomThreshold = 0;
		Moon->SetUseTemperature(false);
		Moon->bAffectsWorld = true;
		Moon->BloomThreshold = 0;

		if (!bEnableTOD)
		{
			Moon->bAffectsWorld = false;
		}
	}
}

void UToWeather::RunTOD()
{

	if (!bInitSuccess) return;
	
	FRotator SunRot, MoonRot;
	float SunScale, MoonScale;

	CalculateSolarMoonWithIntensity(TODTime, TODAsset == nullptr ? HangzhouLatitude : TODAsset->Latitude, HangzhouLongitude,
	                                5, SunRot, MoonRot, SunScale, MoonScale);

	MoonScale *= MoonScale * MoonScale;//月亮变化更快
	
	Sun->SetWorldRotation(SunRot);
	Sun->SetLightColor(SunColor);
	Sun->SetIntensity(SunIntensity * SunScale);
	Sun->CastShadows = SunScale > 0.2f ? 1 : 0;
	Sun->SetShadowAmount(FMath::Lerp(0, SunShadowAmount, SunScale));
	Sun->bAffectsWorld = !FMath::IsNearlyEqual(SunScale, 0);
	Sun->bEnableLightShaftBloom = !FMath::IsNearlyEqual(SunScale*SunScale*SunScale, 0);
	Sun->BloomScale = SunBloomScale;
	
	Moon->SetWorldRotation(MoonRot);
	Moon->SetLightColor(MoonColor);
	Moon->SetIntensity(MoonIntensity * MoonScale);
	Moon->CastShadows = MoonScale > 0.5f ? 1 : 0;
	Moon->SetShadowAmount(FMath::Lerp(0, MoonShadowAmount, MoonScale));
	Moon->bAffectsWorld = !FMath::IsNearlyEqual(MoonScale, 0);
	Moon->bEnableLightShaftBloom = !FMath::IsNearlyEqual(MoonScale*MoonScale, 0);
	Moon->BloomScale = MoonBloomScale;

	if (Sun->bEnableLightShaftBloom)
	{
		Sun->BloomTint = SunBloomColor;
		Moon->BloomTint = SunBloomColor;
	}else
	{
		Sun->BloomTint = MoonBloomColor;
		Moon->BloomTint = MoonBloomColor;
	}

	if (SkySphere)
	{
		SkySphere->SetCustomPrimitiveDataVector3(SkySphereBaseColorIndex,FVector(SkyBaseColor));
	}
	if (SunMesh)
	{
		SunMesh->SetCustomPrimitiveDataVector3(SunColorIndex,FVector(SunMeshColor));
		SunMesh->SetCustomPrimitiveDataFloat(SunAlphaIndex,SunScale);
	}
	if (MoonMesh)
	{
		MoonMesh->SetCustomPrimitiveDataVector3(SunColorIndex,FVector(MoonMeshColor));
		MoonMesh->SetCustomPrimitiveDataFloat(SunAlphaIndex,MoonScale);
	}

	HeightFog->SetFogInscatteringColor(FogColor);

	LastTODTime = TODTime;
}

#pragma region 昼夜计算
// 常量定义
constexpr float SolarDeclinationFixed = 23.44f; // 固定黄赤交角（简化模型）
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 180.0f / PI;

void UToWeather::CalculateFixedSunPosition(float TimeOfDay, float Latitude, float Longitude, float& OutAltitude, float& OutAzimuth)
{
	// 设置默认时间值
	float SunriseStartTime = 5.5f; // 日出开始时间
	float SunriseEndTime = 6.5f; // 日出结束时间
	float SunsetStartTime = 17.5f; // 日落开始时间
	float SunsetEndTime = 18.5f; // 日落结束时间

	if (TODAsset)
	{
		SunriseStartTime = TODAsset->SunriseStartTime;
		SunriseEndTime = TODAsset->SunriseEndTime;
		SunsetStartTime = TODAsset->SunsetStartTime;
		SunsetEndTime = TODAsset->SunsetEndTime;
	}

	// 基于纬度计算太阳高度参数
	const float LatFactor = FMath::Clamp(FMath::Abs(Latitude) / 90.0f, 0.0f, 1.0f); // 0(赤道)-1(极地)
	const float MinSunAltitude = 5.0f * (1.0f - LatFactor * 0.4f); // 高纬度最小高度更低
	const float MaxSunAltitude = 75.0f * (1.0f - LatFactor * 0.5f); // 高纬度最大高度更低

	const float SunriseDuration = SunriseEndTime - SunriseStartTime;
	const float DayDuration = SunsetStartTime - SunriseEndTime;
	const float SunsetDuration = SunsetEndTime - SunsetStartTime;

	if (TimeOfDay < SunriseStartTime || TimeOfDay > SunsetEndTime)
	{
		// 非活动时段：太阳不可见
		OutAltitude = -10.0f;
		OutAzimuth = 180.0f;
		return;
	}

	// 计算标准化进度(0-1)
	float progress;

	if (TimeOfDay <= SunriseEndTime)
	{
		// 日出阶段：从0到0.3
		progress = 0.3f * (TimeOfDay - SunriseStartTime) / SunriseDuration;
	}
	else if (TimeOfDay <= SunsetStartTime)
	{
		// 白天阶段：从0.3到0.7
		progress = 0.3f + 0.4f * (TimeOfDay - SunriseEndTime) / DayDuration;
	}
	else
	{
		// 日落阶段：从0.7到1.0
		progress = 0.7f + 0.3f * (TimeOfDay - SunsetStartTime) / SunsetDuration;
	}

	// 加入纬度影响的高度计算
	float baseAltitude = MinSunAltitude + (MaxSunAltitude - MinSunAltitude) *
		FMath::Sin(progress * PI);

	// 高纬度地区太阳轨迹更平缓
	OutAltitude = baseAltitude * (1.0f - LatFactor * 0.3f);

	// 日落特别处理（保持与光强同步）
	if (TimeOfDay > SunsetStartTime)
	{
		float sunsetProgress = (TimeOfDay - SunsetStartTime) / SunsetDuration;
		OutAltitude = FMath::Lerp(OutAltitude, MinSunAltitude * 0.5f, sunsetProgress);
	}

	// 确保最低可见高度
	OutAltitude = FMath::Max(OutAltitude, 0.1f);

	// 方位角计算（保持不变）
	OutAzimuth = 180.0f + (progress * 180.0f);
}

void UToWeather::CalculateFixedMoonPosition(float TimeOfDay, float Latitude, float Longitude, float& OutAltitude, float& OutAzimuth)
{
	// 月亮比太阳晚12小时出现
	float moonTime = FMath::Fmod(TimeOfDay + 12.0f, 24.0f);
	CalculateFixedSunPosition(moonTime, Latitude, Longitude, OutAltitude, OutAzimuth);

	// 月亮高度限制和调整
	OutAltitude = FMath::Clamp(OutAltitude * 0.7f, 10.0f, 60.0f);

	// 使用TODAsset配置的偏移时间（如果存在）
	if (TODAsset && TODAsset->MoonOffsetHours > 0)
	{
		moonTime = FMath::Fmod(TimeOfDay + TODAsset->MoonOffsetHours, 24.0f);
		CalculateFixedSunPosition(moonTime, Latitude, Longitude, OutAltitude, OutAzimuth);
		OutAltitude = FMath::Clamp(OutAltitude * 0.7f, 10.0f, 60.0f);
	}
}

FRotator UToWeather::AltAzToRotator(float Altitude, float Azimuth)
{
	return FRotator(-Altitude, Azimuth, 0);
}

EMoonPhase UToWeather::CalculateMoonPhase(const FDateTime& Date)
{
	// 月相周期约29.53天，从新月开始
	const float MoonCycleDays = 29.53f;
	int32 DayOfYear = Date.GetDayOfYear();
	float PhaseRatio = FMath::Fmod(DayOfYear, MoonCycleDays) / MoonCycleDays;

	// 根据周期比例返回月相
	if (PhaseRatio < 0.03f) return EMoonPhase::NewMoon;
	if (PhaseRatio < 0.22f) return EMoonPhase::WaxingCrescent;
	if (PhaseRatio < 0.28f) return EMoonPhase::FirstQuarter;
	if (PhaseRatio < 0.47f) return EMoonPhase::WaxingGibbous;
	if (PhaseRatio < 0.53f) return EMoonPhase::FullMoon;
	if (PhaseRatio < 0.72f) return EMoonPhase::WaningGibbous;
	if (PhaseRatio < 0.78f) return EMoonPhase::LastQuarter;
	return EMoonPhase::WaningCrescent;
}

void UToWeather::CalculateSolarMoonWithIntensity(float TimeOfDay, float Latitude, float Longitude, float TransitionAngle, FRotator& OutSunRotation, FRotator& OutMoonRotation, float& OutSunIntensity, float& OutMoonIntensity)
{
	// 设置默认时间值
	float SunriseStartTime = 5.5f;
	float SunriseEndTime = 6.5f;
	float SunsetStartTime = 17.5f;
	float SunsetEndTime = 18.5f;

	if (TODAsset)
	{
		SunriseStartTime = TODAsset->SunriseStartTime;
		SunriseEndTime = TODAsset->SunriseEndTime;
		SunsetStartTime = TODAsset->SunsetStartTime;
		SunsetEndTime = TODAsset->SunsetEndTime;
	}

	// 计算天体角度
	float sunAltitude, sunAzimuth;
	CalculateFixedSunPosition(TimeOfDay, Latitude, Longitude, sunAltitude, sunAzimuth);
	OutSunRotation = FRotator(-sunAltitude, sunAzimuth, 0);

	float moonAltitude, moonAzimuth;
	CalculateFixedMoonPosition(TimeOfDay, Latitude, Longitude, moonAltitude, moonAzimuth);
	OutMoonRotation = FRotator(-moonAltitude, moonAzimuth, 0);

	// 太阳强度 - 纯线性版
	if (TimeOfDay <= SunriseStartTime || TimeOfDay >= SunsetEndTime)
	{
		// 完全夜晚时段：无日光
		OutSunIntensity = 0.0f;
	}
	else if (TimeOfDay >= SunriseStartTime && TimeOfDay <= SunriseEndTime)
	{
		// 日出阶段：线性增强 (0.0 -> 1.0)
		float t = (TimeOfDay - SunriseStartTime) / (SunriseEndTime - SunriseStartTime);
		OutSunIntensity = t;
	}
	else if (TimeOfDay >= SunsetStartTime && TimeOfDay <= SunsetEndTime)
	{
		// 日落阶段：线性减弱 (1.0 -> 0.0)
		float t = (TimeOfDay - SunsetStartTime) / (SunsetEndTime - SunsetStartTime);
		OutSunIntensity = 1.0f - t;
	}
	else
	{
		// 完全白天时段：全强度
		OutSunIntensity = 1.0f;
	}

	// 月光强度 - 完全对称
	if (TimeOfDay <= SunriseStartTime || TimeOfDay >= SunsetEndTime)
	{
		// 完全夜晚时段：满月强度
		OutMoonIntensity = 1.0f;
	}
	else if (TimeOfDay >= SunriseStartTime && TimeOfDay <= SunriseEndTime)
	{
		// 日出阶段：线性减弱 (1.0 -> 0.0)
		float t = (TimeOfDay - SunriseStartTime) / (SunriseEndTime - SunriseStartTime);
		OutMoonIntensity = 1.0f - t;
	}
	else if (TimeOfDay >= SunsetStartTime && TimeOfDay <= SunsetEndTime)
	{
		// 日落阶段：线性增强 (0.0 -> 1.0)
		float t = (TimeOfDay - SunsetStartTime) / (SunsetEndTime - SunsetStartTime);
		OutMoonIntensity = t;
	}
	else
	{
		// 完全白天时段：无月光
		OutMoonIntensity = 0.0f;
	}
}
#pragma endregion
