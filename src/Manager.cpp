#include "Manager.h"
#include "BSResource.h"
#include <algorithm>
#include <random>

namespace logger = SKSE::log;

RE::BSResource::ErrorCode Manager::HideLockpickModelVR(std::string target, bool hide)
{
    RE::NiPointer<RE::NiNode> nPointer;
    constexpr RE::BSModelDB::DBTraits::ArgsType args{};

    const auto demandError = RE::BSModelDB::Demand(target.c_str(), nPointer, args);

    if (demandError == RE::BSResource::ErrorCode::kNone)
	{
        nPointer->AsNode()->CullGeometry(hide);
    }

	return demandError;
}

void Manager::HideLockpickModel(bool hide)
{
	//auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

	if (RE::UI::GetSingleton()->IsMenuOpen(RE::LockpickingMenu::MENU_NAME))
	{
		const auto pickModelHandle = static_cast<RE::BSResource::ModelID*>(RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME)->GetRuntimeData().lockpick);

		if (pickModelHandle && pickModelHandle->data)
		{
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

bool Manager::ActivateFavoriteLockpick()
{
	if (PreferFavoriteIndex())
	{
        RE::TESObjectMISC *currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[favoriteLockpickIndex].formid);

		if (currentLockpickObject)
		{
			if (((*currentLockpickSingleton)->formID != currentLockpickObject->formID))
			{
                *currentLockpickSingleton = currentLockpickObject;
                shouldUpdateModel = true;
			}

			return true;
		}
	}
	return false;
}

/// <summary>
/// Recounts the total number of lockpicks in player inventory.
/// Prepares for lockpick model swapping when the lockpicking menu is opened, a lockpick is broken, or the SkyUI MCM 'menu' is opened.
/// </summary>
/// <returns> the total number of lockpicks in player inventory</returns>
int Manager::RecountAndUpdate()
{
    // 0. Uses strongest by quality lockpick
    // 1. Uses weakest by quality lockpick
    // 2. Uses cheapest by gold value lockpick
    // 3. Uses most expensive by gold value lockpick
    // 4. Acquires a random lockpick on lockpicking menu open and again when player runs out of current lockpick
    // 5. Acquires a random lockpick on lockpicking menu open and again each time the player breaks the current lockpick
	switch (currentLockpickProtocol)
	{
		case 1: // 1. Weakest
			return AcquireWeakestLockpick();
		case 2: // 2. Cheapest
			return AcquireCheapestLockpick();
        case 3: // 3. Most expensive
			return AcquireExpensiveLockpick();
		case 4: // 4. Random Once
            return AcquireRandomOnceLockpick();
        case 5: // 5. Random All
            return AcquireRandomAllLockpick();
		default: // 0. Strongest and any exceptions
            return AcquireStrongestLockpick();
	}
}

int Manager::AcquireStrongestLockpick()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const int vectorSize = eudaLockpickVector.size();
    bool searchingForFirstLockpick = true;
	bool allowModelUpdate;
    int currentLockpickCounter = 0;
    RE::TESObjectMISC *currentLockpickObject;

    uniqueLockpickTotal = 0;

	allowModelUpdate = !ActivateFavoriteLockpick();

    for (int i = 0; i < vectorSize; ++i)
	{
        currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(
            eudaLockpickVector[i].formid);
        currentLockpickCounter = player->GetItemCount(currentLockpickObject);

        if ((currentLockpickCounter >= 1) && searchingForFirstLockpick)
		{
            if ((bestLockpickIndex != i) || ((*currentLockpickSingleton)->formID != currentLockpickObject->formID))
			{
                bestLockpickIndex = i;
                
				if (allowModelUpdate)
				{
                    *currentLockpickSingleton = currentLockpickObject;
                    shouldUpdateModel = true;
				}
            }

            searchingForFirstLockpick = false;
        }

        uniqueLockpickTotal += currentLockpickCounter;
    }

    if (searchingForFirstLockpick) // should account for favorite also
	{
        RevertDefaultLockpick();
    }

	return uniqueLockpickTotal;
}

int Manager::AcquireWeakestLockpick()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const int vectorSize = eudaLockpickVector.size();
    bool searchingForFirstLockpick = true;
    bool allowModelUpdate;
    int currentLockpickCounter = 0;
    RE::TESObjectMISC *currentLockpickObject;

    uniqueLockpickTotal = 0;

    allowModelUpdate = !ActivateFavoriteLockpick();

    for (int i = (vectorSize - 1); i >= 0; --i)
	{
        currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(
            eudaLockpickVector[i].formid);
        currentLockpickCounter = player->GetItemCount(currentLockpickObject);

        if (currentLockpickCounter >= 1 && searchingForFirstLockpick)
		{
            if ((bestLockpickIndex != i) || ((*currentLockpickSingleton)->formID !=
                                              currentLockpickObject->formID))
			{
                bestLockpickIndex = i;

				if (allowModelUpdate)
				{
                    *currentLockpickSingleton = currentLockpickObject;
                    shouldUpdateModel = true;
				}
            }

            searchingForFirstLockpick = false;
        }

        uniqueLockpickTotal += currentLockpickCounter;
    }

    if (searchingForFirstLockpick)
	{
        RevertDefaultLockpick();
    }

    return uniqueLockpickTotal;
}

int Manager::AcquireCheapestLockpick()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const int vectorSize = std::min(eudaLockpickVector.size(), eudaLockpickGoldValueVector.size());
    bool searchingForFirstLockpick = true;
    bool allowModelUpdate;
    int currentLockpickCounter = 0;
    RE::TESObjectMISC *currentLockpickObject;

    uniqueLockpickTotal = 0;

    allowModelUpdate = !ActivateFavoriteLockpick();

	for (int i = 0; i < vectorSize; ++i)
	{
        currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[eudaLockpickGoldValueVector[i].index].formid);
        currentLockpickCounter = player->GetItemCount(currentLockpickObject);

        if (currentLockpickCounter >= 1 && searchingForFirstLockpick)
		{
            if ((bestLockpickIndex != eudaLockpickGoldValueVector[i].index) || ((*currentLockpickSingleton)->formID != currentLockpickObject->formID))
			{
                bestLockpickIndex = eudaLockpickGoldValueVector[i].index;

				if (allowModelUpdate)
				{
                    *currentLockpickSingleton = currentLockpickObject;
                    shouldUpdateModel = true;
				}
            }

            searchingForFirstLockpick = false;
        }

        uniqueLockpickTotal += currentLockpickCounter;
	}

	if (searchingForFirstLockpick)
	{
        RevertDefaultLockpick();
    }

    return uniqueLockpickTotal;
}

int Manager::AcquireExpensiveLockpick()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const int vectorSize = std::min(eudaLockpickVector.size(), eudaLockpickGoldValueVector.size());
    bool searchingForFirstLockpick = true;
    bool allowModelUpdate;
    int currentLockpickCounter = 0;
    RE::TESObjectMISC *currentLockpickObject;

    uniqueLockpickTotal = 0;

    allowModelUpdate = !ActivateFavoriteLockpick();

	for (int i = (vectorSize - 1); i >= 0; --i)
	{
        currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[eudaLockpickGoldValueVector[i].index].formid);
        currentLockpickCounter = player->GetItemCount(currentLockpickObject);

        if (currentLockpickCounter >= 1 && searchingForFirstLockpick)
		{
            if ((bestLockpickIndex != eudaLockpickGoldValueVector[i].index) || ((*currentLockpickSingleton)->formID != currentLockpickObject->formID))
			{
                bestLockpickIndex = eudaLockpickGoldValueVector[i].index;

				if (allowModelUpdate)
				{
                    *currentLockpickSingleton = currentLockpickObject;
                    shouldUpdateModel = true;
				}
            }

            searchingForFirstLockpick = false;
        }

        uniqueLockpickTotal += currentLockpickCounter;
	}

	if (searchingForFirstLockpick)
	{
        RevertDefaultLockpick();
    }

    return uniqueLockpickTotal;
}

int Manager::AcquireRandomOnceLockpick()
{
	// check to see if menu is opening, single break, or ran out and need to use a different lockpick
	if (lockpickingMenuState == LOCKPICKING_MENU_STATE_UPDATING) // 1
	{
        if (PreferFavoriteIndex() || (RE::PlayerCharacter::GetSingleton()->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[bestLockpickIndex].formid)) >= 1))
		{
			// updating state and player still has current lockpicks left, just recount
            return RecountUniqueLockpickTotal();
		}
		else
		{
			// updating state but player has no current lockpicks left, get new random lockpick
            return AcquireRandomAllLockpick();
		}
	}
	else // 0, LOCKPICKING_MENU_STATE_OPENING and any exceptions
	{
		// opening state, get new random lockpick
        return AcquireRandomAllLockpick();
	}
}

int Manager::AcquireRandomAllLockpick()
{
    const auto player = RE::PlayerCharacter::GetSingleton();
    const int vectorSize = std::min(eudaLockpickVector.size(), eudaLockpickRandomVector.size());
    bool searchingForFirstLockpick = true;
    bool allowModelUpdate;
    int currentLockpickCounter = 0;
    RE::TESObjectMISC *currentLockpickObject;

    uniqueLockpickTotal = 0;

    allowModelUpdate = !ActivateFavoriteLockpick();

	ShuffleRandomVector();

	for (int i = 0; i < vectorSize; ++i)
	{
        currentLockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[eudaLockpickRandomVector[i]].formid);
        currentLockpickCounter = player->GetItemCount(currentLockpickObject);

		if (currentLockpickCounter >= 1 && searchingForFirstLockpick)
		{
            if ((bestLockpickIndex != eudaLockpickRandomVector[i]) || ((*currentLockpickSingleton)->formID != currentLockpickObject->formID))
			{
                bestLockpickIndex = eudaLockpickRandomVector[i];

				if (allowModelUpdate)
				{
                    *currentLockpickSingleton = currentLockpickObject;
                    shouldUpdateModel = true;
				}
            }

            searchingForFirstLockpick = false;
        }

        uniqueLockpickTotal += currentLockpickCounter;
	}

	if (searchingForFirstLockpick)
	{
        RevertDefaultLockpick();
    }

    return uniqueLockpickTotal;
}

int Manager::RevertDefaultLockpick()
{
    // set to default
    *currentLockpickSingleton = RE::TESForm::LookupByID<RE::TESObjectMISC>(Data::DEFAULT_LOCKPICK_FORM_ID);
    bestLockpickIndex = eudaLockpickMap.at((*currentLockpickSingleton)->formID);
    //shouldUpdateModel = true; // debatable
}

// prefer PrepareSecondaryVectors function
// Needs data loaded first, should only be called after eudaLockpickVector has been sorted
void Manager::PrepareRandomVector()
{
	const int size = eudaLockpickVector.size();

    eudaLockpickRandomVector.resize(size);

    for (int i = 0; i < size; ++i)
	{
        eudaLockpickRandomVector[i] = i;
    }
}

void Manager::ShuffleRandomVector()
{
    std::shuffle(eudaLockpickRandomVector.begin(),
                 eudaLockpickRandomVector.end(),
        std::default_random_engine(
            std::chrono::system_clock::now().time_since_epoch().count()));
}

// prepares secondary vectors such as goldvalue or random
// Needs data loaded first, should only be called after eudaLockpickVector has been sorted
void Manager::PrepareSecondaryVectors()
{
    const int size = eudaLockpickVector.size();

    eudaLockpickGoldValueVector.resize(size);
	eudaLockpickRandomVector.resize(size);

    for (int i = 0; i < size; ++i)
	{
        eudaLockpickGoldValueVector[i].goldValue = eudaLockpickVector[i].goldValue;
        eudaLockpickGoldValueVector[i].index = i;

		eudaLockpickRandomVector[i] = i;
    }

    SortGoldValueVector();
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

	for (int i = 0; i < vectorSize; ++i)
	{
		uniqueLockpickTotal += player->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[i].formid));
	}

	return uniqueLockpickTotal;
}

bool Manager::PrepareLockpickSingleton()
{
	//currentLockpickSingleton = (RE::TESObjectMISC**)RELOCATION_ID(514921, 401059).address();  // Lockpick singleton ID -> address
    currentLockpickSingleton = (RE::TESObjectMISC**)REL::VariantID(514921, 401059, 0x2FC4748).address();

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
	if (lockpickValue)
	{
		*currentLockpickSingleton = lockpickValue;
		bestLockpickIndex = bestIndex;
	}
	else
	{
		*currentLockpickSingleton = RE::TESForm::LookupByID<RE::TESObjectMISC>(Data::DEFAULT_LOCKPICK_FORM_ID);
		bestLockpickIndex = eudaLockpickMap.at((*currentLockpickSingleton)->formID);
	}

	return bestLockpickIndex;
}

int Manager::UpdateBestLockpickFromIndex(int indexValue = 0)
{
	const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
	const auto vectorSize = eudaLockpickVector.size();

	while (indexValue >= 0 && indexValue < vectorSize)
	{
		const auto currentItem = RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[indexValue].formid);

		if (currentItem && playerCharacter->GetItemCount(currentItem) >= 1)
		{
			return UpdateLockpickSingleton(currentItem, indexValue);
		}

		++indexValue;
	}

	return UpdateLockpickSingleton(RE::TESForm::LookupByID<RE::TESObjectMISC>(Data::DEFAULT_LOCKPICK_FORM_ID), eudaLockpickMap.at(Data::DEFAULT_LOCKPICK_FORM_ID));
}


/// <summary>
/// Sorts the member variable eudaLockpickVector in descending order based on quality variable.
/// With index 0 being the highest quality and index (eudaLockpickVector.size - 1) being the weakest quality
/// </summary>
void Manager::SortLockpicksByQuality()
{
    std::sort(eudaLockpickVector.begin(), eudaLockpickVector.end(), std::greater<EudaLockpickData>()); // descending, strongest at index 0
}

bool Manager::TranslateLockLevel(RE::LOCK_LEVEL value, float& unmodifiedBreakSeconds, float& modifiedBreakSeconds)
{
    const auto currentGameSettingCollection = RE::GameSettingCollection::GetSingleton();
	RE::Setting* currentSetting;
	bool successful = false;

    // equivalent to UNMODIFIED game settings.
    // fLockpickBreakBase:			0.05 --- potentially unused
    // fLockpickBreakNovice:		2.00
    // fLockpickBreakApprentice:	1.00
    // fLockpickBreakAdept:			0.75
    // fLockpickBreakExpert:		0.50
    // fLockpickBreakMaster:		0.25
	if (currentGameSettingCollection)
	{
		if ((value == RE::LOCK_LEVEL::kEasy) && (currentSetting = currentGameSettingCollection->GetSetting("fLockpickBreakApprentice")) && (currentSetting->GetType() == RE::Setting::Type::kFloat)) 
		{
            // kEasy = 1 --- apprentice
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = currentSetting->data.f;
			successful = true;
		}
		else if ((value == RE::LOCK_LEVEL::kAverage) && (currentSetting = currentGameSettingCollection->GetSetting("fLockpickBreakAdept")) && (currentSetting->GetType() == RE::Setting::Type::kFloat))
		{
            // kAverage = 2 --- adept
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = currentSetting->data.f;
            successful = true;
		}
		else if ((value == RE::LOCK_LEVEL::kHard) && (currentSetting = currentGameSettingCollection->GetSetting("fLockpickBreakExpert")) && (currentSetting->GetType() == RE::Setting::Type::kFloat))
		{
            // kHard = 3 --- expert
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = currentSetting->data.f;
            successful = true;
		}
		else if ((value == RE::LOCK_LEVEL::kVeryHard) && (currentSetting = currentGameSettingCollection->GetSetting("fLockpickBreakMaster")) && (currentSetting->GetType() == RE::Setting::Type::kFloat))
		{
            // kVeryHard = 4 --- master
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = currentSetting->data.f;
            successful = true;
		}
		else if ((currentSetting = currentGameSettingCollection->GetSetting("fLockpickBreakNovice")) && (currentSetting->GetType() == RE::Setting::Type::kFloat))
		{
            // kVeryEasy = 0 --- novice also exceptions for kUnlocked = -1 and kRequiresKey = 5
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = currentSetting->data.f;
            successful = true;
		}
	}

	return successful;
}

void Manager::TranslateLockLevelFallBack(RE::LOCK_LEVEL value, float& unmodifiedBreakSeconds, float& modifiedBreakSeconds)
{
    // equivalent to UNMODIFIED game settings.
    // fLockpickBreakBase:			0.05 --- potentially unused
    // fLockpickBreakNovice:		2.00
    // fLockpickBreakApprentice:	1.00
    // fLockpickBreakAdept:			0.75
    // fLockpickBreakExpert:		0.50
    // fLockpickBreakMaster:		0.25
    switch (value)
	{
        case RE::LOCK_LEVEL::kEasy:  // kEasy = 1 --- apprentice
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = FALLBACK_LOCKPICK_BREAK_APPRENTICE;
            break;
        case RE::LOCK_LEVEL::kAverage:  // kAverage = 2 --- adept
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = FALLBACK_LOCKPICK_BREAK_ADEPT;
            break;
        case RE::LOCK_LEVEL::kHard:  // kHard = 3 --- expert
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = FALLBACK_LOCKPICK_BREAK_EXPERT;
            break;
        case RE::LOCK_LEVEL::kVeryHard:  // kVeryHard = 4 --- master
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = FALLBACK_LOCKPICK_BREAK_MASTER;
            break;
        default:  // kVeryEasy = 0 --- novice also exceptions for kUnlocked = -1 and kRequiresKey = 5
            unmodifiedBreakSeconds = 1.00;
            modifiedBreakSeconds = FALLBACK_LOCKPICK_BREAK_NOVICE;
    }
}

float Manager::CalculatePickBreak(RE::LOCK_LEVEL lockLevel)
{
    const auto currentGameSettingCollections = RE::GameSettingCollection::GetSingleton();
    const auto currentPlayer = RE::PlayerCharacter::GetSingleton();

	RE::Setting* currentSetting;
    float unmodifiedBreakSeconds, modifiedBreakSeconds;
    
	const auto actorValueOwner = currentPlayer ? currentPlayer->AsActorValueOwner() : nullptr;

	// float arguments passed by reference
    bool successful = TranslateLockLevel(lockLevel, unmodifiedBreakSeconds, modifiedBreakSeconds);

	if (successful && actorValueOwner && currentGameSettingCollections &&
        (currentSetting = currentGameSettingCollections->GetSetting("fLockpickBreakSkillMult")) && currentSetting &&
        (currentSetting->GetType() == RE::Setting::Type::kFloat))
	{
        // running smoothly
        return ((actorValueOwner->GetActorValue(RE::ActorValue::kLockpicking) * currentSetting->data.f) + unmodifiedBreakSeconds) * modifiedBreakSeconds;
	}
	else if (actorValueOwner && currentGameSettingCollections &&
               (currentSetting = currentGameSettingCollections->GetSetting("fLockpickBreakSkillMult")) &&
               currentSetting && (currentSetting->GetType() == RE::Setting::Type::kFloat))
	{
        // Game setting from TranslateLockLevel doesn't exist, had its name, or data type modified
        // float arguments passed by reference
		TranslateLockLevelFallBack(lockLevel, unmodifiedBreakSeconds, modifiedBreakSeconds);

		return ((actorValueOwner->GetActorValue(RE::ActorValue::kLockpicking) * currentSetting->data.f) +
                unmodifiedBreakSeconds) *
               modifiedBreakSeconds;
	}
	else if (actorValueOwner)
	{
		// Game setting fLockpickBreakSkillMult doesn't exist, player skill exists at least
        // float arguments passed by reference
        TranslateLockLevelFallBack(lockLevel, unmodifiedBreakSeconds, modifiedBreakSeconds);

		return ((actorValueOwner->GetActorValue(RE::ActorValue::kLockpicking) *
            FALLBACK_LOCKPICK_BREAK_SKILL_MULT) +
        unmodifiedBreakSeconds) *
        modifiedBreakSeconds;
	}
	else
	{
        // everything is busted, assume vanilla values. Can't even scale lockpicking off player.
        // float arguments passed by reference
        TranslateLockLevelFallBack(lockLevel, unmodifiedBreakSeconds, modifiedBreakSeconds);

		return modifiedBreakSeconds;
	}
}

float Manager::CalculateQualityModifier()
{
	if (bestLockpickIndex >= 0 && bestLockpickIndex < eudaLockpickVector.size())
	{
        return (eudaLockpickVector[bestLockpickIndex].quality / ((float)DEFAULT_LOCKPICK_QUALITY));
	}
	else
	{
		return FALLBACK_LOCKPICK_QUALITY_MODIFIER;
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

bool Manager::PreferFavoriteIndex()
{
    return useFavoriteLockpick && ((favoriteLockpickIndex >= 0) && (favoriteLockpickIndex < eudaLockpickVector.size())) && (RE::PlayerCharacter::GetSingleton()->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(eudaLockpickVector[favoriteLockpickIndex].formid)) >= 1);
}

bool Manager::GoodBestIndex()
{
    return ((bestLockpickIndex >= 0) && (bestLockpickIndex < eudaLockpickVector.size()));
}

std::string Manager::GetLockpickModel(const char* a_fallbackPath)
{

    std::string path(a_fallbackPath);

    if (path == Data::skeletonKey)
	{
        return path;
    }

	if (PreferFavoriteIndex())
	{
        return eudaLockpickVector[favoriteLockpickIndex].path;
    }
	else if (GoodBestIndex())
	{
        return eudaLockpickVector[bestLockpickIndex].path;
	}

    return path;
}

// prefer PrepareSecondaryVectors function
// Needs data loaded first, should only be called after eudaLockpickVector has been sorted
void Manager::PrepareGoldValueVector()
{
	const int size = eudaLockpickVector.size();

	eudaLockpickGoldValueVector.resize(size);

	for (int i = 0; i < size; ++i)
	{
        eudaLockpickGoldValueVector[i].goldValue = eudaLockpickVector[i].goldValue;
        eudaLockpickGoldValueVector[i].index = i;
	}

	SortGoldValueVector();
}

void Manager::SortGoldValueVector()
{
	std::sort(eudaLockpickGoldValueVector.begin(), eudaLockpickGoldValueVector.end()); // ascending, cheapest at index 0
}


bool Manager::IsUsingFavorite()
{
	return useFavoriteLockpick;
}

bool Manager::UseFavorite(bool usingFavorite)
{
	useFavoriteLockpick = usingFavorite;

	return useFavoriteLockpick;
}

int Manager::GetFavoriteIndex()
{
	return favoriteLockpickIndex;
}

int Manager::SetFavoriteIndex(int favoriteIndex)
{
	if (favoriteIndex >= 0 && favoriteIndex < eudaLockpickVector.size())
	{
        favoriteLockpickIndex = favoriteIndex;
	}
	else
	{
		useFavoriteLockpick = false;
        favoriteLockpickIndex = INVALID_LOCKPICK_INDEX;
	}

	return favoriteLockpickIndex;
}

int Manager::GetLockpickProtocol()
{
	return currentLockpickProtocol;
}

int Manager::SetLockpickProtocol(int protocol)
{
	if (protocol >= 0 && protocol < lockpickUsageProtocol.size()) 
	{
        currentLockpickProtocol = protocol;
	}
	else
	{
        currentLockpickProtocol = DEFAULT_LOCKPICK_PROTOCOL;
	}

	return currentLockpickProtocol;
}