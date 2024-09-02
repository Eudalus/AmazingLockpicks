#include "Hooks.h"
#include "Manager.h"

#include <ranges>
#include <iostream>
#include <fstream>
#include "BSResource.h"
#include <cctype>    // std::tolower
#include <algorithm> // std::equal

// Include spdlog support for a basic file logger
// See below for more details
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

// Allows us to check if a debugger is attached (optional, see below)
#include <Windows.h>

namespace logger = SKSE::log;

void SetupLog()
{
    // Get the path to the SKSE logs folder
    // This will generally be your Documents\My Games\Skyrim Special Edition\SKSE
    //                          or Documents\My Games\Skyrim Special Edition GOG\SKSE
    auto logsFolder = SKSE::log::log_directory();

    // I really don't understand why the log_directory() might not be provided sometimes,
    // but... just incase... ?
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    // Get the name of this SKSE plugin. We will use it to name the log file.
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    // Generate a path to our log file
    // e.g. Documents\My Games\Skyrim Special Edition\SKSE\OurPlugin.log
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);

    // Now, use whatever you want, but spdlog comes with CommonLibSSE
    // and is the SKSE logger of choice. So you might as well use it!

    // Let's use a spdlog "basic file sink"
    // So like... just a file logger...
    // But the spdlog interface really wants a Shared Pointer to the "basic file sink"
    // So we'll make one of those!
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);

    // // Ok, but set_default_logger() specifically wants a Shared Pointer to a spdlog::logger
    // // So we'll make one of those!
    std::shared_ptr<spdlog::logger> loggerPtr;

    // Now, this is pretty cool.
    // If you want the logs to show up *inside your IDE* when you are debugging, use this code.
    // Whenever a debugger is attached, the logs are setup with an *additional* "sink" to goto
    // your IDE's debug output window.
    if (IsDebuggerPresent()) {
        auto debugLoggerPtr = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        spdlog::sinks_init_list loggers{std::move(fileLoggerPtr), std::move(debugLoggerPtr)};
        loggerPtr = std::make_shared<spdlog::logger>("log", loggers);
    } else {
        // If no debugger is attached, only log to the file.
        loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    }

    // We'll give it the logger we made above. Yeah, I know, kinda redundant right? Welcome to C++
    spdlog::set_default_logger(std::move(loggerPtr));

    // Yay, let's setup spdlog now!
    // By default, let's print out *everything* including trace messages
    // You might want to do something like #ifdef NDEBUG then use trace, else use info or higher severity.
    spdlog::set_level(spdlog::level::trace);

    // This bit is important. When does spdlog WRITE to the file?
    // Make sure it does it everytime you log a message, otherwise it won't write to the file until the game exits.
    spdlog::flush_on(spdlog::level::trace);
}

/*
void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			if (Manager::GetSingleton()->LoadLocks()) {
				Model::Install();
				Sound::Install();
			}
		}
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		{
			logger::info("{:*^30}", "MERGES");
			MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
			if (g_mergeMapperInterface) {                        // Use Interface
				const auto version = g_mergeMapperInterface->GetBuildNumber();
				logger::info("\tGot MergeMapper interface buildnumber {}", version);
			} else {
				logger::info("INFO - MergeMapper not detected");
			}
		}
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		Manager::GetSingleton()->InitLockForms();
		break;
	default:
		break;
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("Amazing Lockpicks");
	v.AuthorName("Eudalus");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Amazing Lockpicks";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver
#	ifndef SKYRIMVR
		< SKSE::RUNTIME_1_5_39
#	else
		> SKSE::RUNTIME_VR_1_4_15_1
#	endif
	) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
*/

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	const auto currentManager = Manager::GetSingleton();

	if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
		logger::info("AmazingLockpicks.dll - inside function MessageHandler, if message->type == kPostLoad");

		currentManager->isPostLoadComplete = currentManager->LoadLocks();

		if (currentManager->isPostLoadComplete) {
			logger::info("AmazingLockpicks.dll - inside function MessageHandler, if message->type == kPostLoad - Manager::GetSingleton()->LoadLocks() returned true");

			Model::Install();

			logger::info("AmazingLockpicks.dll - inside function MessageHandler, if message->type == kPostLoad - Model::Install() returned successfully");

			//EudaMessageUpdate::LockpickingMenuMessageHook::Hook();
			//EudaMessageUpdate::EudaIMenuMessageHook::Hook();
			//EudaMessageUpdate::LockpickingMenuMovieHook::Hook();
			//EudaMessageUpdate::PlayerCharacterRemoveItem::Hook();
			//EudaMessageUpdate::UpdatePickHealthHook::Hook();
			EudaMessageUpdate::CanOpenLockpickingMenuHook::Hook();
			EudaMessageUpdate::TryBeginLockPickingHook::Hook();
			EudaMessageUpdate::UnknownSetupHook::Hook();

			logger::info(
				"AmazingLockpicks.dll - inside function MessageHandler, if message->type == kPostLoad - "
				"LockpickingMenuMessageHook::Install() returned successfully");

			//Sound::Install();
		}
	} else if (a_message->type == SKSE::MessagingInterface::kDataLoaded && currentManager->isPostLoadComplete) {
		logger::info("AmazingLockpicks.dll - inside function MessageHandler, else if message->type == kDataLoaded");

		currentManager->PrepareLockpickSingleton();

		RE::FormID        originalLockpickFormID = 0xA;
		RE::FormID        newLockpickFormID = 0xA;
		RE::BSFixedString newString;

		RE::TESObjectMISC* lockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);
		int                indexor = 0;

		logger::info("{} --- LOCKPICK IS PLAYABLE: {}", indexor, lockpickObject->GetPlayable());
		++indexor;
		logger::info("{} --- LOCKPICK FORM FLAGS: {}", indexor, lockpickObject->formFlags);
		++indexor;

		if ((lockpickObject->formFlags & RE::TESObjectMISC::RecordFlags::kNonPlayable)) {
			lockpickObject->formFlags = lockpickObject->formFlags - RE::TESObjectMISC::RecordFlags::kNonPlayable;
		}

		logger::info("{} --- LOCKPICK IS PLAYABLE: {}", indexor, lockpickObject->GetPlayable());
		++indexor;
		logger::info("{} --- LOCKPICK FORM FLAGS: {}", indexor, lockpickObject->formFlags);
		++indexor;

		logger::info("Checking for invalid editor ids");

		std::vector<Manager::EudaLockpickData>::iterator eudaIterator = currentManager->eudaLockpickVector.begin();

		while (eudaIterator != currentManager->eudaLockpickVector.end()) {
			if (!(RE::TESForm::LookupByEditorID<RE::TESObjectMISC>(eudaIterator->editor))) {
				logger::warn("Removing editor id: {}", eudaIterator->editor);
				eudaIterator = currentManager->eudaLockpickVector.erase(eudaIterator);
			} else {
				++eudaIterator;
			}
		}

		logger::info("Grabbing {} lockpick formids for lookup", currentManager->eudaLockpickVector.size());

		int ijk;

		for (ijk = 0; ijk < currentManager->eudaLockpickVector.size(); ++ijk) {
			RE::TESObjectMISC* currentLockpickForm =
				RE::TESForm::LookupByEditorID<RE::TESObjectMISC>(currentManager->eudaLockpickVector.at(ijk).editor);

			currentManager->eudaLockpickVector.at(ijk).formid =
				currentLockpickForm->GetFormID();

			logger::info("{} --- Lockpick form id: {} matched with editor id: {}", (ijk + 1),
				std::format("{:x}", currentManager->eudaLockpickVector.at(ijk).formid),
				currentManager->eudaLockpickVector.at(ijk).editor);

			// acquire
			newString = currentManager->eudaLockpickVector.at(ijk).editor;
			newLockpickFormID = currentLockpickForm->GetFormID();

			logger::info("{} --- Changing lockpick weight from {} to {}", (ijk + 1),
				currentLockpickForm->GetWeight(),
				std::format("{}", currentManager->eudaLockpickVector.at(ijk).weight));
			currentLockpickForm->weight = currentManager->eudaLockpickVector.at(ijk).weight;

			if (currentManager->eudaLockpickVector.at(ijk).name.size() > 0) {
				logger::info("{} --- Changing lockpick name from {} to {}", (ijk + 1),
					currentLockpickForm->GetFullName(), currentManager->eudaLockpickVector.at(ijk).name);

				currentLockpickForm->fullName = currentManager->eudaLockpickVector.at(ijk).name;
			}
		}

		logger::info("Lockpick formids grabbed: {} --- should match total from .ini files (not enforced, check formids in esp files and editor in .ini files if discrepancy occurs)", ijk);

		currentManager->eudaFormList =
			RE::TESForm::LookupByEditorID<RE::BGSListForm>(currentManager->EudaFormListString);

		if (!currentManager->eudaFormList) {
			logger::warn("Could not find formlist with editor: {}", currentManager->EudaFormListString);
		}

		const int vectorSize = currentManager->eudaLockpickVector.size();
		bool      stillSearching = true;

		for (int i = 0; i < vectorSize && stillSearching; ++i) {
			if (currentManager->eudaLockpickVector.at(i).formid == 0xA) {
				stillSearching = false;
			}
		}

		if (stillSearching) {
			logger::warn("Did not find original (0xA) lockpick from file. Inserting with default values.");

			Manager::EudaLockpickData tempData;
			tempData.editor = "Lockpick";
			tempData.formid = 0xA;
			tempData.name = "Iron Lockpick";
			tempData.path = Data::defaultLockPick;
			tempData.quality = currentManager->DEFAULT_LOCKPICK_QUALITY;
			tempData.weight = currentManager->DEFAULT_LOCKPICK_WEIGHT;

			currentManager->eudaLockpickVector.emplace_back(tempData);
		}

		currentManager->SortLockpicksByQuality();

		for (int i = 0; i < currentManager->eudaLockpickVector.size(); ++i) {
			if (currentManager->eudaLockpickMap.count(currentManager->eudaLockpickVector.at(i).formid) == 0) {
				currentManager->eudaLockpickMap.emplace(currentManager->eudaLockpickVector.at(i).formid, i);
			}
		}

		currentManager->UpdateLockpickSingleton(lockpickObject, currentManager->eudaLockpickMap.at(0xA));

		currentManager->RecountUniqueLockpickTotal();
		currentManager->UpdateBestLockpickFromIndex(0);

	} else if (a_message->type == SKSE::MessagingInterface::kNewGame && currentManager->isPostLoadComplete) {
		logger::info("AmazingLockpicks.dll - inside function MessageHandler, else if message->type == kNewGame");

		RE::TESObjectMISC* lockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);

		currentManager->UpdateLockpickSingleton(lockpickObject, currentManager->eudaLockpickMap.at(0xA));
		currentManager->RecountUniqueLockpickTotal();
		currentManager->UpdateBestLockpickFromIndex(0);

	} else if (a_message->type == SKSE::MessagingInterface::kPostLoadGame && currentManager->isPostLoadComplete) {
		logger::info("AmazingLockpicks.dll - inside function MessageHandler, else if message->type == kPostLoadGame");

		RE::TESObjectMISC* lockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);

		currentManager->UpdateLockpickSingleton(lockpickObject, currentManager->eudaLockpickMap.at(0xA));
		currentManager->RecountUniqueLockpickTotal();
		currentManager->UpdateBestLockpickFromIndex(0);
	}
}

void TranslateLockLevel(RE::LOCK_LEVEL value, float& unmodifiedBreakSeconds, float& modifiedBreakSeconds)
{
	// values equivalent to UNMODIFIED game settings.
	//fLockpickBreakBase:		0.05 --- potentially unused
	//fLockpickBreakNovice:		2.00
	//fLockpickBreakApprentice:	1.00
	//fLockpickBreakAdept:		0.75
	//fLockpickBreakExpert:		0.50
	//fLockpickBreakMaster:		0.25

	switch (value) {
	case RE::LOCK_LEVEL::kEasy:  //kEasy = 1 --- apprentice
		unmodifiedBreakSeconds = 1.00;
		modifiedBreakSeconds =
			RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakApprentice")->data.f;
		break;
	case RE::LOCK_LEVEL::kAverage:  //kAverage = 2 --- adept
		unmodifiedBreakSeconds = 1.00;
		modifiedBreakSeconds =
			RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakAdept")->data.f;
		break;
	case RE::LOCK_LEVEL::kHard:  //kHard = 3 --- expert
		unmodifiedBreakSeconds = 1.00;
		modifiedBreakSeconds =
			RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakExpert")->data.f;
		break;
	case RE::LOCK_LEVEL::kVeryHard:  //kVeryHard = 4 --- master
		unmodifiedBreakSeconds = 1.00;
		modifiedBreakSeconds =
			RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakMaster")->data.f;
		break;
	default:  // kVeryEasy = 0 --- novice also exceptions for kUnlocked = -1 and kRequiresKey = 5
		unmodifiedBreakSeconds = 1.00;
		modifiedBreakSeconds =
			RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakNovice")->data.f;
	}
}

float CalculatePickBreak(RE::LOCK_LEVEL lockLevel)
{
	float unmodifiedBreakSeconds, modifiedBreakSeconds;

	TranslateLockLevel(lockLevel, unmodifiedBreakSeconds, modifiedBreakSeconds);

	return ((RE::PlayerCharacter::GetSingleton()
					->GetInfoRuntimeData()
					.skills->data->skills[RE::PlayerCharacter::PlayerSkills::Data::Skills::kLockpicking]
					.level *
				RE::GameSettingCollection::GetSingleton()->GetSetting("fLockpickBreakSkillMult")->data.f *
				unmodifiedBreakSeconds) +
			   unmodifiedBreakSeconds) *
	       modifiedBreakSeconds;

	//RE::Actor *player = RE::TESForm::LookupByID<RE::Actor>(0x14);

	//player->GetActorRuntimeData().avStorage.baseValues.
}

/*
void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path)
	{
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}

*/





SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SetupLog();

    // auto *plugin = SKSE::PluginDeclaration::GetSingleton();
    // auto version = plugin->GetVersion();

    // version.compare(SKSE::PluginDeclaration::GetSingleton()->GetVersion());

    // logger::info("{} {} is loading...", plugin->GetName(), version);

    SKSE::Init(skse);

    //SKSE::GetPapyrusInterface()->Register(EudaBindPapyrusFunctions);

    // spdlog::info("UniqueLockpicks - printing from function SKSEPluginLoad using spdlog::info");
    // SKSE::log::info("UniqueLockpicks - printing from function SKSEPluginLoad using SKSE::log::info");

    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    return true;
}