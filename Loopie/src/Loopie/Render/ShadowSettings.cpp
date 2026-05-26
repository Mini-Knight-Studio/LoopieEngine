#include "ShadowSettings.h"
#include "ShadowSettings.h"
#include "ShadowSettings.h"


namespace Loopie
{
	ShadowQuality GetPresetQualityFromString(std::string string)
	{
		if (string == "Low")
			return ShadowQuality::Low;
		else if (string == "High")
			return ShadowQuality::High;
		else if (string == "Ultra")
			return ShadowQuality::Ultra;

		return ShadowQuality::Medium; // default option
	}

	const char* GetPresetQualityString(ShadowQuality quality)
	{
		switch (quality)
		{
			case Loopie::ShadowQuality::Low:
				return "Low";
			case Loopie::ShadowQuality::Medium:
				return "Medium";
			case Loopie::ShadowQuality::High:
				return "High";
			case Loopie::ShadowQuality::Ultra:
				return "Ultra";
			default:
				return "Unknown";
		}
	}

	ShadowFilter GetShadowFilterFromString(std::string string)
	{
		if (string == "Hard")
			return ShadowFilter::Hard;
		else if (string == "Softer")
			return ShadowFilter::Softer;
		else if (string == "Softest")
			return ShadowFilter::Softest;

		return ShadowFilter::Soft; // default option
	}

	const char* GetShadowFilterString(ShadowFilter filter)
	{
		switch (filter)
		{
		case Loopie::ShadowFilter::Hard:
			return "Hard";
		case Loopie::ShadowFilter::Soft:
			return "Soft";
		case Loopie::ShadowFilter::Softer:
			return "Softer";
		case Loopie::ShadowFilter::Softest:
			return "Softest";
		default:
			return "Unknown";
		}
	}

	ShadowQualityPreset LookupQualityPreset(ShadowQuality quality)
	{
		ShadowQualityPreset sqp;
		sqp.dynamicRes = LookupDynamicRes(quality);
		sqp.staticRes = LookupStaticRes(quality);
		return sqp;
	}

	int LookupStaticRes(ShadowQuality quality)
	{
		switch (quality)
		{
		case ShadowQuality::Low:
			return SHADOW_RES_2K;
		case ShadowQuality::High:
			return SHADOW_RES_4K;
		case ShadowQuality::Ultra:
			return SHADOW_RES_8K;
		default:
		case ShadowQuality::Medium:
			break;
		}
		return SHADOW_RES_4K;
	}

	int LookupDynamicRes(ShadowQuality quality)
	{
		switch (quality)
		{
		case ShadowQuality::Low:
			return SHADOW_RES_1K;
		case ShadowQuality::High:
			return SHADOW_RES_4K;
		case ShadowQuality::Ultra:
			return SHADOW_RES_4K;
		default:
		case ShadowQuality::Medium:
			break;
		}
		return SHADOW_RES_2K;
	}

	int LookupShadowFilterRadius(ShadowFilter filter)
	{
		return static_cast<int>(filter);
	}

	ShadowSettings::ShadowSettings(ShadowQuality quality, ShadowFilter filter)
	{
		SetShadowQuality(quality);
		SetFilter(filter);
	}

	ShadowQuality ShadowSettings::GetShadowQuality() const
	{
		return m_shadowQuality;
	}

	void ShadowSettings::SetShadowQuality(ShadowQuality sqp)
	{
		m_shadowQuality = sqp;
		SetQualityPreset(sqp);
	}

	ShadowQualityPreset ShadowSettings::GetShadowQualityPreset() const
	{
		ShadowQualityPreset sqp;
		sqp.dynamicRes = m_dynamicRes;
		sqp.staticRes = m_staticRes;
		return sqp;
	}

	int ShadowSettings::GetDynamicRes() const
	{
		return m_dynamicRes;
	}

	void ShadowSettings::SetQualityPreset(ShadowQuality quality)
	{
		ShadowQualityPreset sqp = LookupQualityPreset(quality);
		m_dynamicRes = sqp.dynamicRes;
		m_staticRes = sqp.staticRes;
	}

	void ShadowSettings::SetDynamicRes(ShadowQuality dynamicQuality)
	{
		m_dynamicRes = LookupDynamicRes(dynamicQuality);
	}

	int ShadowSettings::GetStaticRes() const
	{
		return m_staticRes;
	}

	void ShadowSettings::SetStaticRes(ShadowQuality staticQuality)
	{
		m_staticRes = LookupStaticRes(staticQuality);
	}

	ShadowFilter ShadowSettings::GetFilter() const
	{
		return m_shadowFilter;
	}

	void ShadowSettings::SetFilter(ShadowFilter filter)
	{
		m_shadowFilter = filter;
		SetFilterRadius(filter);
	}

	int ShadowSettings::GetFilterRadius() const
	{
		return m_filterRadius;
	}

	void ShadowSettings::SetFilterRadius(ShadowFilter filter)
	{
		m_filterRadius = LookupShadowFilterRadius(filter);
	}
}