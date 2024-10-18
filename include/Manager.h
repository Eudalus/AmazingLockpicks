#pragma once

#include "LockData.h"
#include "SimpleIni.h"
#include "BSResource.h"

class Manager
{
public:
	[[nodiscard]] static Manager* GetSingleton()
	{
		static Manager singleton;
		return std::addressof(singleton);
	}
	bool LoadLocks();
	void SortLockpicksByQuality();
	int UpdateBestLockpickFromIndex(int startIndex);
	bool PrepareLockpickSingleton();
	int UpdateLockpickSingleton(RE::TESObjectMISC *lockpickValue, int bestIndex);
	int UpdateUniqueLockpickTotal(int value);
	int RecountUniqueLockpickTotal();
	int RecountAndUpdate();
    int AcquireStrongestLockpick();
    int AcquireWeakestLockpick();
    int AcquireCheapestLockpick();
    int AcquireExpensiveLockpick();
    int AcquireRandomOnceLockpick();
    int AcquireRandomAllLockpick();
    int RevertDefaultLockpick();
	RE::BSResource::ErrorCode ReloadLockpickModel();
	void HideLockpickModel(bool hide = true);
    RE::BSResource::ErrorCode HideLockpickModelVR(std::string target, bool hide = true);
    bool TranslateLockLevel(RE::LOCK_LEVEL value, float& unmodifiedBreakSeconds, float& modifiedBreakSeconds);
    void TranslateLockLevelFallBack(RE::LOCK_LEVEL value, float& unmodifiedBreakSeconds, float& modifiedBreakSeconds);
    float CalculatePickBreak(RE::LOCK_LEVEL lockLevel);
    float CalculateQualityModifier();
    void PrepareGoldValueVector();
    void SortGoldValueVector();
    void PrepareRandomVector();
    void ShuffleRandomVector();
    void PrepareSecondaryVectors(); // does goldvalue and random in a single loop
    bool IsUsingFavorite();
    bool UseFavorite(bool usingFavorite);
    int GetFavoriteIndex();
    int SetFavoriteIndex(int favoriteIndex);
    int GetLockpickProtocol();
    int SetLockpickProtocol(int protocol);
    
	std::string GetLockpickModel(const char* a_fallbackPath);

	struct detail
	{
		static void get_value(const CSimpleIniA& a_ini, std::string& a_value, const char* a_section, const char* a_key,
			const std::string& a_default = std::string())
		{
			a_value = a_ini.GetValue(a_section, a_key, a_default.empty() ? a_value.c_str() : a_default.c_str());
		}

		static bool icontains(std::string_view a_str1, std::string_view a_str2)
		{
			if (a_str2.length() > a_str1.length())
				return false;

			auto found = std::ranges::search(a_str1, a_str2, [](char ch1, char ch2) {
				return std::toupper(static_cast<unsigned char>(ch1)) == std::toupper(static_cast<unsigned char>(ch2));
			});

			return !found.empty();
		}

		static bool has_snow(RE::TESModel* a_model)
		{
			const auto modelSwap = a_model->GetAsModelTextureSwap();

			if (modelSwap && modelSwap->alternateTextures) {
				std::span span(modelSwap->alternateTextures, modelSwap->numAlternateTextures);
				for (const auto& texture : span) {
					if (const auto txst = texture.textureSet;
						txst && icontains(txst->textures[RE::BSTextureSet::Texture::kDiffuse].textureName, "snow"sv)) {
						return true;
					}
				}
			}

			return false;
		}

		static bool is_underwater()
		{
			const auto waterSystem = RE::TESWaterSystem::GetSingleton();
			return waterSystem && waterSystem->playerUnderwater;
		}
	};

	// data stored in eudaLockpickVector
	class EudaLockpickData
	{
    public:
		std::string editor;
		std::string path;
		int         quality;
		RE::FormID  formid = 0;
		float       weight = 0;
		int			goldValue;
		std::string name;
        
		bool operator<(const EudaLockpickData other)
		{
			return quality < other.quality;
		}

		bool operator>(const EudaLockpickData other)
		{
            return quality > other.quality;
        }
	};

	// used to index into eudaLockpickVector based on gold value
	class EudaGoldValueData
	{
    public:
		int index;
		int goldValue;

		bool operator<(const EudaGoldValueData& other)
		{
			return goldValue < other.goldValue;
		}

		bool operator>(const EudaGoldValueData &other)
		{
            return goldValue > other.goldValue;
        }
	};

	std::vector<EudaLockpickData>       eudaLockpickVector; // default vector, should be sorted by lockpick quality
	std::unordered_map<RE::FormID, int> eudaLockpickMap;  // formid key, int value indexes into eudaLockpickVector

	// contains index values to map into eudaLockpickVector for cheapest and most expensive lockpicks by gold value
	// if sorted by cheapest to most expensive, index 0 should index into eudaLockpickVector for the cheapest lockpick by gold value
	// if sorted by most expensive to cheapest, index 0 should index into eudaLockpickVector for the most expensive lockpick by gold value
    std::vector<EudaGoldValueData> eudaLockpickGoldValueVector;

	// contains index values to map into eudaLockpickVector
	// allows shuffling or randomizing elements so that a single array
	// can be iterated over to randomly choose lockpicks without requiring inserts or deletes
	// each time the player doesn't have the randomly chosen lockpick in inventory
	std::vector<int> eudaLockpickRandomVector;

	RE::TESObjectMISC** currentLockpickSingleton;
	bool isLockpickHealthUpdating = false;
	int  bestLockpickIndex = -1;
	int  uniqueLockpickTotal = 0;
	bool isPostLoadComplete = false;
	bool shouldUpdateModel = false;
	bool allowEnterAudio = true;
	bool allowLockSwap = true;
	bool allowLockIntro = true;
	RE::NiPoint3 originalLockRotationCenter;
	bool bypassSurvivalModeWeight = true;
	bool useFavoriteLockpick = false;
	int currentLockpickProtocol = 0;
	bool needsDataLoad = true;
	bool needsPostLoad = true;
	bool needsPostPostLoad = true;

	const int INVALID_LOCKPICK_INDEX = 0;
    int favoriteLockpickIndex = INVALID_LOCKPICK_INDEX; // will index into the default strongest / weakest vector

	const int LOCKPICKING_MENU_STATE_OPENING = 0;
    const int LOCKPICKING_MENU_STATE_UPDATING = 1;
	const int LOCKPICKING_MENU_STATE_WILD = 2;

	int lockpickingMenuState = LOCKPICKING_MENU_STATE_OPENING; // used to determine if Random Once protocol should find a new lockpick

	const int   DEFAULT_LOCKPICK_QUALITY = 1000;
	const float DEFAULT_LOCKPICK_WEIGHT = 0.0f;
	const float FALLBACK_LOCKPICK_BREAK_NOVICE = 2.00f;
    const float FALLBACK_LOCKPICK_BREAK_APPRENTICE = 1.00f;
    const float FALLBACK_LOCKPICK_BREAK_ADEPT = 0.75f;
    const float FALLBACK_LOCKPICK_BREAK_EXPERT = 0.50f;
    const float FALLBACK_LOCKPICK_BREAK_MASTER = 0.25f;
    const float FALLBACK_LOCKPICK_BREAK_SKILL_MULT = 0.005f;
	const float FALLBACK_LOCKPICK_QUALITY_MODIFIER = 1.0f;
	const int DEFAULT_LOCKPICK_PROTOCOL = 0; // strongest


	// 0. Uses strongest by quality lockpick
	// 1. Uses weakest by quality lockpick
	// 2. Uses cheapest by gold value lockpick
	// 3. Uses most expensive by gold value lockpick
	// 4. Acquires a random lockpick on lockpicking menu open and again when player runs out of current lockpick
	// 5. Acquires a random lockpick on lockpicking menu open and again each time the player breaks the current lockpick
    const std::vector<std::string> lockpickUsageProtocol{"Strongest", "Weakest", "Cheapest", "Expensive", "Random Once", "Random All"};
};
