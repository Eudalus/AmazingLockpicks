#include "Manager.h"
#include "BSResource.h"

namespace logger = SKSE::log;

void Manager::HideLockpickModel(bool hide)
{
	//auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

	if (RE::UI::GetSingleton()->IsMenuOpen(RE::LockpickingMenu::MENU_NAME)) {
		const auto pickModelHandle = static_cast<RE::BSResource::ModelID*>(RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME)->GetRuntimeData().lockpick);

		if (pickModelHandle && pickModelHandle->data) {
			//pickModelHandle->data->SetAppCulled(hide);
			pickModelHandle->data->CullGeometry(hide);
			//pickModelHandle->data->CullNode(hide);
		}
	}
}

RE::BSResource::ErrorCode Manager::ReloadLockpickModel()
{
	auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

	if (menuNow)
	{
        RE::LockpickingMenu::RUNTIME_DATA& dataNow = menuNow->GetRuntimeData();
        auto lockpickHandle = static_cast<RE::BSResource::ModelID*>(dataNow.lockpick);

		if (lockpickHandle)
		{
            const auto scene = RE::UI3DSceneManager::GetSingleton();
            scene->DetachChild(lockpickHandle->data.get());
            lockpickHandle = nullptr;
		}
        allowLockIntro = false;
		allowLockSwap = false;
		allowEnterAudio = false;
		dataNow.init3DElements = false;
	}

	return RE::BSResource::ErrorCode::kNone;
}

int Manager::RecountAndUpdate()
{
	const auto         player = RE::PlayerCharacter::GetSingleton();
	const int          vectorSize = eudaLockpickVector.size();
	bool               stillSearching = true;
	int                currentLockpickCounter = 0;
	RE::TESObjectMISC* currentLockpickObject;

	uniqueLockpickTotal = 0;

	for (int i = 0; i < vectorSize; ++i) {
		currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector.at(i).formid);
		currentLockpickCounter = player->GetItemCount(currentLockpickObject);

		if (currentLockpickCounter >= 1 && stillSearching) {
			if (bestLockpickIndex != i || (*currentLockpickSingleton)->formID != currentLockpickObject->formID) {
				*currentLockpickSingleton = currentLockpickObject;
				bestLockpickIndex = i;
				shouldUpdateModel = true;
			}

			stillSearching = false;
		}

		uniqueLockpickTotal += currentLockpickCounter;
	}

	if (stillSearching) {
		// set to default
		*currentLockpickSingleton = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);
		bestLockpickIndex = eudaLockpickMap.at((*currentLockpickSingleton)->formID);
		//shouldUpdateModel = true; // debatable
	}

	return uniqueLockpickTotal;
}

int Manager::UpdateUniqueLockpickTotal(int value)
{
	uniqueLockpickTotal += value;
	return uniqueLockpickTotal;
}

int Manager::RecountUniqueLockpickTotal()
{
	const std::size_t vectorSize = eudaLockpickVector.size();
	auto const        player = RE::PlayerCharacter::GetSingleton();

	uniqueLockpickTotal = 0;

	for (int i = 0; i < vectorSize; ++i) {
		uniqueLockpickTotal += player->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector.at(i).formid));
	}

	return uniqueLockpickTotal;
}

bool Manager::PrepareLockpickSingleton()
{
	currentLockpickSingleton = (RE::TESObjectMISC**)RELOCATION_ID(514921, 401059).address();  // Lockpick singleton ID -> address

	return currentLockpickSingleton;
}

/// <summary>
/// Updates the game's lockpick singleton
/// Assumes that the member variable currentLockpickSingleton has been prepared via the PrepareLockpickSingleton() function
/// </summary>
/// <param name="lockpickValue">A pointer that the singleton will be updated to point to</param>
/// <param name="bestIndex>The index of the new lockpick in the member variable eudaLockpickVector</param>
/// <returns>The int member variable bestLockpickIndex</returns>
int Manager::UpdateLockpickSingleton(RE::TESObjectMISC* lockpickValue, int bestIndex)
{
	if (lockpickValue) {
		*currentLockpickSingleton = lockpickValue;
		bestLockpickIndex = bestIndex;
	} else {
		*currentLockpickSingleton = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);
		bestLockpickIndex = eudaLockpickMap.at((*currentLockpickSingleton)->formID);
	}

	return bestLockpickIndex;
}

int Manager::UpdateBestLockpickFromIndex(int indexValue = 0)
{
	const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
	const auto vectorSize = eudaLockpickVector.size();

	while (indexValue >= 0 && indexValue < vectorSize) {
		const auto currentItem = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector.at(indexValue).formid);

		if (currentItem && playerCharacter->GetItemCount(currentItem) >= 1) {
			return UpdateLockpickSingleton(currentItem, indexValue);
		}

		++indexValue;
	}

	return UpdateLockpickSingleton(RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA), eudaLockpickMap.at(0xA));
}


/// <summary>
/// Sorts the member variable eudaLockpickVector in descending order based on quality variable.
/// With index 0 being the highest quality and index (eudaLockpickVector.size - 1) being the weakest quality
/// </summary>
void Manager::SortLockpicksByQuality()
{
	EudaLockpickData tempData;
	const int        vectorSize = eudaLockpickVector.size();

	for (int k = 0; k < vectorSize; ++k) {
		for (int i = 0; (i + 1) < vectorSize; ++i) {
			tempData.quality = eudaLockpickVector.at(i + 1).quality;

			if (tempData.quality > eudaLockpickVector.at(i).quality) {
				tempData.editor = eudaLockpickVector.at(i + 1).editor;
				tempData.path = eudaLockpickVector.at(i + 1).path;
				tempData.weight = eudaLockpickVector.at(i + 1).weight;
				tempData.name = eudaLockpickVector.at(i + 1).name;
				tempData.formid = eudaLockpickVector.at(i + 1).formid;

				eudaLockpickVector.at(i + 1).quality = eudaLockpickVector.at(i).quality;
				eudaLockpickVector.at(i + 1).editor = eudaLockpickVector.at(i).editor;
				eudaLockpickVector.at(i + 1).path = eudaLockpickVector.at(i).path;
				eudaLockpickVector.at(i + 1).weight = eudaLockpickVector.at(i).weight;
				eudaLockpickVector.at(i + 1).name = eudaLockpickVector.at(i).name;
				eudaLockpickVector.at(i + 1).formid = eudaLockpickVector.at(i).formid;

				eudaLockpickVector.at(i).quality = tempData.quality;
				eudaLockpickVector.at(i).editor = tempData.editor;
				eudaLockpickVector.at(i).path = tempData.path;
				eudaLockpickVector.at(i).weight = tempData.weight;
				eudaLockpickVector.at(i).name = tempData.name;
				eudaLockpickVector.at(i).formid = tempData.formid;
			}
		}
	}
}

bool Manager::LoadLocks()
{
	logger::info("{:*^30}", "INI");

	std::vector<std::string>              configs;
	std::unordered_map<std::string, bool> mapChecker;

	const int   defaultQuality = DEFAULT_LOCKPICK_QUALITY;
	const float defaultWeight = DEFAULT_LOCKPICK_WEIGHT;

	const std::string defaultQualityString = "1000";
	const std::string defaultWeightString = "0.0";

	constexpr auto suffix = "_EAL"sv;

	auto constexpr folder = R"(Data\)";
	for (const auto& entry : std::filesystem::directory_iterator(folder)) {
		if (entry.exists() && !entry.path().empty() && entry.path().extension() == ".ini"sv) {
			if (const auto path = entry.path().string(); path.rfind(suffix) != std::string::npos) {
				configs.push_back(path);
			}
		}
	}

	if (configs.empty()) {
		logger::warn("	No .ini files with {} suffix were found within the Data folder, aborting...", suffix);
		return false;
	}

	logger::info("	{} matching inis found", configs.size());

	std::ranges::sort(configs);

	for (auto& path : configs) {
		logger::info("		INI : {}", path);

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetMultiKey();

		if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
			logger::error("	couldn't read INI");
			continue;
		}

		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);
		sections.sort(CSimpleIniA::Entry::LoadOrder());

		for (auto& [section, comment, order] : sections) {
			EudaLockpickData eudaData;
			std::string      qualityString;
			std::string      weightString;

			logger::info("READ SECTION: {}", section);

			detail::get_value(ini, eudaData.editor, section, "Editor");
			logger::info("Editor: {}", eudaData.editor);

			detail::get_value(ini, eudaData.path, section, "Lockpick");
			logger::info("Lockpick: {}", eudaData.path);

			detail::get_value(ini, eudaData.name, section, "Name");
			logger::info("Name: {}", eudaData.name);

			detail::get_value(ini, qualityString, section, "Quality", defaultQualityString);
			logger::info("Quality: {}", qualityString);

			detail::get_value(ini, weightString, section, "Weight", defaultWeightString);

			try {
				logger::info("CONVERTING QUALITY STRING TO INTEGER");
				eudaData.quality = std::stoi(qualityString);

				if (0 >= eudaData.quality) {
					logger::warn("Bad quality {} from editor {} from section {} from file {} --- Changing to default: {}", eudaData.quality, eudaData.editor, section, path, defaultQuality);
					eudaData.quality = defaultQuality;
				}
			} catch (const std::invalid_argument& ia) {
				logger::warn("Invalid quality: {} --- setting to default: {}", ia.what(), defaultQuality);
				eudaData.quality = defaultQuality;
			}

			try {
				logger::info("CONVERTING WEIGHT STRING TO FLOAT");
				eudaData.weight = std::stof(weightString);

				if (0 > eudaData.weight) {
					logger::warn("Bad weight {} from editor {} from section {} from file {} --- Changing to 0.00", eudaData.weight, eudaData.editor, section, path);
					eudaData.weight = defaultWeight;
				}
			} catch (const std::invalid_argument& ia) {
				logger::warn("Invalid weight: {} --- setting to default: {}", ia.what(), defaultWeight);
				eudaData.weight = defaultWeight;
			}

			if (mapChecker.count(eudaData.editor) == 0) {
				eudaLockpickVector.emplace_back(eudaData);
				mapChecker.emplace(eudaData.editor, true);
			} else {
				logger::info("Ignoring duplicate editor {} from section {} from file {}", eudaData.editor, section, path);
			}

			logger::info("READ COMPLETE");
		}
	}

	logger::info("{:*^30}", "RESULTS");

	logger::info("{} lockpick types found", eudaLockpickVector.size());

	logger::info("{:*^30}", "INFO");

	return !eudaLockpickVector.empty();
}

std::string Manager::GetLockpickModel(const char* a_fallbackPath)
{

    std::string path(a_fallbackPath);

    if (path == Data::skeletonKey) {
        return path;
    }

    // return EudaRealUpdateLock();
    return eudaLockpickVector.at(bestLockpickIndex).path;
}