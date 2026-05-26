#pragma once
#define SHADOW_RES_1K	1024
#define SHADOW_RES_2K	2048
#define SHADOW_RES_4K	4096
#define SHADOW_RES_8K	8192

#include <string>

namespace Loopie
{
	// 
	// WARNING: Persisted to project.config as an int.
	// DO NOT REORDER existing entries.
	// Add new ones at the end if need be.
	enum class ShadowQuality
	{
		Low = 0,
		Medium,
		High,
		Ultra
	};

	enum class ShadowFilter
	{
		Hard = 0,
		Soft,
		Softer,
		Softest
	};

	struct ShadowQualityPreset
	{
		int dynamicRes;
		int staticRes;
	};

	ShadowQuality GetPresetQualityFromString(std::string string = "Medium");
	ShadowFilter GetShadowFilterFromString(std::string string = "Soft");

	ShadowQualityPreset LookupQualityPreset(ShadowQuality quality = ShadowQuality::Medium);
	int LookupStaticRes(ShadowQuality quality = ShadowQuality::Medium);
	int LookupDynamicRes(ShadowQuality quality = ShadowQuality::Medium);
	int LookupShadowFilterRadius(ShadowFilter filter = ShadowFilter::Soft);

	class ShadowSettings
	{
	public:
		ShadowSettings(ShadowQuality quality = ShadowQuality::Medium, ShadowFilter filter = ShadowFilter::Soft);
		~ShadowSettings() = default;

		ShadowQuality GetShadowQuality() const;
		void SetShadowQuality(ShadowQuality sqp = ShadowQuality::Medium);
		ShadowQualityPreset GetShadowQualityPreset() const;
		int  GetDynamicRes() const;
		int  GetStaticRes() const;

		ShadowFilter GetFilter() const;
		void SetFilter(ShadowFilter filter);
		int  GetFilterRadius() const;

	private:
		void SetQualityPreset(ShadowQuality quality);
		void SetDynamicRes(ShadowQuality dynamicQuality); // Exists for possible debugging
		void SetStaticRes(ShadowQuality staticQuality); // Exists for possible debugging
		void SetFilterRadius(ShadowFilter filter);

	private:
		ShadowQuality m_shadowQuality = ShadowQuality::Medium;
		ShadowFilter  m_shadowFilter = ShadowFilter::Soft;

		int m_dynamicRes = SHADOW_RES_2K;
		int m_staticRes = SHADOW_RES_4K;
		int m_filterRadius = 1;
	};
}