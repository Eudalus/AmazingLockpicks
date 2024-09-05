#pragma once

#include "LockData.h"
#include "SimpleIni.h"
#include "BSResource.h"

/*
class Manager : public ISingleton<Manager>
{
public:
	bool LoadLocks();
	void InitLockForms();

	std::string GetLockModel(const char* a_fallbackPath);
	std::string GetLockpickModel(const char* a_fallbackPath);

	const std::optional<Lock::Sound>& GetSounds();

private:
	void Sanitize(const std::string& a_path);
	
	// members
	std::set<Lock::Variant, std::less<>> lockVariants{};
	std::optional<Lock::Sound>           currentSound{};
};
*/

class Manager
{
public:
	[[nodiscard]] static Manager* GetSingleton()
	{
		static Manager singleton;
		return std::addressof(singleton);
	}
	void AttachNodes();
	bool LoadLocks();
	void SortLockpicksByQuality();
	int UpdateBestLockpickFromIndex(int startIndex);
	bool PrepareLockpickSingleton();
	int UpdateLockpickSingleton(RE::TESObjectMISC *lockpickValue, int bestIndex);
	int UpdateUniqueLockpickTotal(int value);
	int RecountUniqueLockpickTotal();
	int RecountAndUpdate();
	RE::BSResource::ErrorCode ReloadLockpickModel();
	void HideLockpickModel(bool hide = true);
	void RemoveNodes(RE::NiObjectPtr& node);

	//std::string GetLockModel(const char* a_fallbackPath);
	std::string GetLockpickModel(const char* a_fallbackPath);

	//std::optional<Data::Sound> GetSoundData();

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

	//std::map<Data::LockType, Data::LockSet> lockDataMap;
	//std::map<Data::LockType, Data::Sound>   soundDataMap;

	//std::optional<Data::LockType> currentLockType;

	struct EudaLockpickData
	{
		std::string editor;
		std::string path;
		int         quality;
		RE::FormID  formid = 0;
		float       weight = 0;
		std::string name;
	};

	std::vector<EudaLockpickData>       eudaLockpickVector;
	std::unordered_map<RE::FormID, int> eudaLockpickMap;  // formid key, int value indexes into eudaLockpickVector

	RE::TESObjectMISC** currentLockpickSingleton;
    //RE::BSResource::ModelID* lastLockpickHandle;
	bool isLockpickHealthUpdating = false;
	int  bestLockpickIndex = -1;
	int  uniqueLockpickTotal = 0;
	bool isPostLoadComplete = false;
	bool shouldUpdateModel = false;
	bool allowEnterAudio = true;
	bool allowLockSwap = true;
	bool allowLockIntro = true;
	RE::NiPoint3 originalLockRotationCenter;

	const int   DEFAULT_LOCKPICK_QUALITY = 1000;
	const float DEFAULT_LOCKPICK_WEIGHT = 0.0f;

	RE::NiPointer<RE::NiObject> pickBodyNode = nullptr;
	RE::NiPointer<RE::NiObject> pickRodNode = nullptr;

	RE::NiPointer<RE::NiObject> pickBodyTrishape = nullptr;
	RE::NiPointer<RE::NiObject> pickRodTrishape = nullptr;

	RE::NiPointer<RE::NiObject> originalBodyNode = nullptr;
	RE::NiPointer<RE::NiObject> originalRodNode = nullptr;

	RE::TESObjectMISC** lockpickSingletonDoublePointer = nullptr;
	RE::TESObjectMISC*  lockpickSingletonSinglePointer = nullptr;

	bool nodesReady = false;

	RE::BGSListForm*  eudaFormList;
	const std::string EudaFormListString = "EudaLockpickFormList";
};
