#include "ToWeather.h"

#include "ToShader.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"


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
	GetOwner()->GetAttachedActors(ChildActors);
	if (ChildActors.IsEmpty()) return;
	for (const auto ChildActor : ChildActors)
	{
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
	}
	if (Sun && Moon && SkyLight && SkyAtmosphere)
	{
		bInitSuccess = true;
		tolog("ToWeatherInitSuccess");

		Sun->SetUseTemperature(false);
		Moon->SetUseTemperature(false);

		if (!bEnableTOD)
		{
			Moon->bAffectsWorld = false;
		}
	}
}

void UToWeather::RunTOD()
{
	if (!bInitSuccess || FMath::IsNearlyEqual(TODTime, LastTODTime)) return;
	FRotator SunRot, MoonRot;
	float SunScale, MoonScale;
	CalculateSolarMoonWithIntensity(TODTime, HangzhouLatitude, HangzhouLongitude, 5, SunRot, MoonRot, SunScale, MoonScale);

	Sun->SetWorldRotation(SunRot);
	Sun->SetLightColor(SunColor);
	Sun->SetIntensity(SunIntensity * SunScale);
	Sun->CastShadows = SunScale > 0.2f ? 1 : 0;
	Sun->SetShadowAmount(FMath::Lerp(0, SunShadowAmount, SunScale));

	Moon->SetWorldRotation(MoonRot);
	Moon->SetLightColor(MoonColor);
	Moon->SetIntensity(MoonIntensity * MoonScale);
	Moon->CastShadows = MoonScale > 0.5f ? 1 : 0;
	Moon->SetShadowAmount(FMath::Lerp(0, MoonShadowAmount, MoonScale));

	LastTODTime = TODTime;
}

#pragma region 昼夜计算
// 常量定义
constexpr float SolarDeclinationFixed = 23.44f; // 固定黄赤交角（简化模型）
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 180.0f / PI;
constexpr float MoonOffsetHours = 0.83f; // 月亮比太阳滞后约50分钟（0.83小时）

void UToWeather::CalculateFixedSunPosition(float TimeOfDay, float Latitude, float Longitude, float& OutAltitude, float& OutAzimuth)
{
	// 简化模型：固定太阳轨迹为平滑弧线
	// 上午(6-12点)从0°升至60°，下午(12-18)从60°降至0°
	// 夜晚(18-6)保持在地平线下
    
	if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) {
		// 白天时间
		float progress = (TimeOfDay - 6.0f) / 12.0f; // 0-1
		OutAltitude = 60.0f * FMath::Sin(progress * PI); // 平滑弧线运动
		OutAzimuth = 180.0f + (progress * 180.0f); // 从东(180)到西(360)
	} else {
		// 夜晚时间
		OutAltitude = -20.0f; // 保持在地平线下
		OutAzimuth = 180.0f;
	}
}

void UToWeather::CalculateFixedMoonPosition(float TimeOfDay, float Latitude, float Longitude, float& OutAltitude, float& OutAzimuth)
{
	// 月亮位置 = 太阳位置 + 6小时偏移(简化模型)
	float MoonTime = FMath::Fmod(TimeOfDay + MoonOffsetHours, 24.0f);
	CalculateFixedSunPosition(MoonTime, Latitude, Longitude, OutAltitude, OutAzimuth);
    
	// 月亮高度略低于太阳
	OutAltitude *= 0.8f;
}

FRotator UToWeather::AltAzToRotator(float Altitude, float Azimuth)
{
	return FRotator(-Altitude, Azimuth, 0);
}

void UToWeather::CalculateFixedDaySolarMoonRotation(float TimeOfDay, float Latitude, float Longitude, FRotator& OutSunRotation, FRotator& OutMoonRotation)
{
	// 1. 计算固定太阳位置
	float SunAltitude, SunAzimuth;
	CalculateFixedSunPosition(TimeOfDay, Latitude, Longitude, SunAltitude, SunAzimuth);
	OutSunRotation = AltAzToRotator(SunAltitude, SunAzimuth);

	// 2. 计算固定月亮位置（太阳位置 + 滞后）
	float MoonAltitude, MoonAzimuth;
	CalculateFixedMoonPosition(TimeOfDay, Latitude, Longitude, MoonAltitude, MoonAzimuth);
	OutMoonRotation = AltAzToRotator(MoonAltitude, MoonAzimuth);
}

EMoonPhase UToWeather::CalculateMoonPhase(const FDateTime& Date)
{
	// 月相周期约29.53天，从新月开始
	const float MoonCycleDays = 29.53f;
	int32 DayOfYear = Date.GetDayOfYear();
	float PhaseRatio = FMath::Fmod(DayOfYear, MoonCycleDays) / MoonCycleDays;

	// 根据周期比例返回月相
	if (PhaseRatio < 0.03f) return EMoonPhase::NewMoon;
	else if (PhaseRatio < 0.22f) return EMoonPhase::WaxingCrescent;
	else if (PhaseRatio < 0.28f) return EMoonPhase::FirstQuarter;
	else if (PhaseRatio < 0.47f) return EMoonPhase::WaxingGibbous;
	else if (PhaseRatio < 0.53f) return EMoonPhase::FullMoon;
	else if (PhaseRatio < 0.72f) return EMoonPhase::WaningGibbous;
	else if (PhaseRatio < 0.78f) return EMoonPhase::LastQuarter;
	else return EMoonPhase::WaningCrescent;
}

void UToWeather::CalculateSolarMoonWithIntensity(float TimeOfDay, float Latitude, float Longitude, float TransitionAngle, FRotator& OutSunRotation, FRotator& OutMoonRotation, float& OutSunIntensity, float& OutMoonIntensity)
{
	// 1. 计算固定太阳/月亮位置（保持原有逻辑）
	CalculateFixedDaySolarMoonRotation(
		TimeOfDay, Latitude, Longitude,
		OutSunRotation, OutMoonRotation
	);

	// 2. 简化线性过渡参数
	const float SunriseStart = 5.5f;  // 日出开始时间
	const float SunriseEnd = 6.5f;    // 日出结束时间
	const float SunsetStart = 17.5f;  // 日落开始时间
	const float SunsetEnd = 18.5f;    // 日落结束时间

	// 3. 纯线性过渡处理
	if (TimeOfDay >= SunriseStart && TimeOfDay <= SunriseEnd) {
		// 日出线性过渡
		float t = (TimeOfDay - SunriseStart) / (SunriseEnd - SunriseStart);
		OutSunIntensity = t;
		OutMoonIntensity = 1.0f - t;
	}
	else if (TimeOfDay >= SunsetStart && TimeOfDay <= SunsetEnd) {
		// 日落线性过渡（确保从1.0平滑降到0.0）
		float t = (TimeOfDay - SunsetStart) / (SunsetEnd - SunsetStart);
		OutSunIntensity = 1.0f - t;
		OutMoonIntensity = t;
	}
	else if (TimeOfDay > SunriseEnd && TimeOfDay < SunsetStart) {
		// 白天（全太阳光）
		OutSunIntensity = 1.0f;
		OutMoonIntensity = 0.0f;
	}
	else {
		// 夜晚（全月光）
		OutSunIntensity = 0.0f;
		OutMoonIntensity = 1.0f;
	}

	// 4. 确保边界值准确
	if (TimeOfDay <= SunriseStart || TimeOfDay >= SunsetEnd) {
		OutSunIntensity = 0.0f;
		OutMoonIntensity = 1.0f;
	}
	else if (TimeOfDay >= SunriseEnd && TimeOfDay <= SunsetStart) {
		OutSunIntensity = 1.0f;
		OutMoonIntensity = 0.0f;
	}
}
#pragma endregion
