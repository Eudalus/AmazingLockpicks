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

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	const auto currentManager = Manager::GetSingleton();

	if (a_message->type == SKSE::MessagingInterface::kPostLoad)
	{
		currentManager->isPostLoadComplete = currentManager->LoadLocks();
	}
	else if (a_message->type == SKSE::MessagingInterface::kDataLoaded && currentManager->isPostLoadComplete)
	{
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

		if ((lockpickObject->formFlags & RE::TESObjectMISC::RecordFlags::kNonPlayable))
		{
			lockpickObject->formFlags = lockpickObject->formFlags - RE::TESObjectMISC::RecordFlags::kNonPlayable;
		}

		logger::info("{} --- LOCKPICK IS PLAYABLE: {}", indexor, lockpickObject->GetPlayable());
		++indexor;
		logger::info("{} --- LOCKPICK FORM FLAGS: {}", indexor, lockpickObject->formFlags);
		++indexor;

		logger::info("Checking for invalid editor ids");

		std::vector<Manager::EudaLockpickData>::iterator eudaIterator = currentManager->eudaLockpickVector.begin();

		while (eudaIterator != currentManager->eudaLockpickVector.end()) {
			if (!(RE::TESForm::LookupByEditorID<RE::TESObjectMISC>(eudaIterator->editor)))
			{
				logger::warn("Removing editor id: {}", eudaIterator->editor);
				eudaIterator = currentManager->eudaLockpickVector.erase(eudaIterator);
			} else
			{
				++eudaIterator;
			}
		}

		logger::info("Grabbing {} lockpick formids for lookup", currentManager->eudaLockpickVector.size());

		int ijk;

		for (ijk = 0; ijk < currentManager->eudaLockpickVector.size(); ++ijk)
		{
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

			if (currentManager->eudaLockpickVector.at(ijk).name.size() > 0)
			{
				logger::info("{} --- Changing lockpick name from {} to {}", (ijk + 1),
					currentLockpickForm->GetFullName(), currentManager->eudaLockpickVector.at(ijk).name);

				currentLockpickForm->fullName = currentManager->eudaLockpickVector.at(ijk).name;
			}
		}

		logger::info("Lockpick formids grabbed: {} --- should match total from .ini files (not enforced, check formids in esp files and editor in .ini files if discrepancy occurs)", ijk);

		currentManager->eudaFormList =
			RE::TESForm::LookupByEditorID<RE::BGSListForm>(currentManager->EudaFormListString);

		if (!currentManager->eudaFormList)
		{
			logger::warn("Could not find formlist with editor: {}", currentManager->EudaFormListString);
		}

		const int vectorSize = currentManager->eudaLockpickVector.size();
		bool      stillSearching = true;

		for (int i = 0; i < vectorSize && stillSearching; ++i)
		{
			if (currentManager->eudaLockpickVector.at(i).formid == 0xA)
			{
				stillSearching = false;
			}
		}

		if (stillSearching)
		{
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

	}
	else if (a_message->type == SKSE::MessagingInterface::kPostPostLoad && currentManager->isPostLoadComplete)
	{
        if (currentManager->isPostLoadComplete)
		{
            Model::Lockpick::RequestModel::Install();

            // EudaMessageUpdate::LockpickingMenuMessageHook::Hook();
            // EudaMessageUpdate::EudaIMenuMessageHook::Hook();
            // EudaMessageUpdate::LockpickingMenuMovieHook::Hook();
            // EudaMessageUpdate::PlayerCharacterRemoveItem::Hook();
            // EudaMessageUpdate::UpdatePickHealthHook::Hook();
            EudaMessageUpdate::CanOpenLockpickingMenuHook::Hook();
            EudaMessageUpdate::TryBeginLockPickingHook::Hook();
            EudaMessageUpdate::UnknownSetupHook::Hook();
            EudaMessageUpdate::EnterLockIntroHook::Hook();

			// VR doesn't have survival mode
#ifdef SKYRIM_AE || SKYRIM_SE
            EudaMessageUpdate::GetWeightHook::Hook();
#endif

#ifdef SKYRIM_AE || Skyrim_VR
            EudaMessageUpdate::EnterSoundEffectHookAE::Hook();
#endif

#ifdef SKYRIM_SE
            EudaMessageUpdate::EnterSoundEffectHookSE::Hook();
#endif

			// prevents reloading lock and shiv model each time LockpickingMenu's member variable init3DElements is set
            // to false to dynamically load a different lockpick model
            Model::Lock::RequestModel::Install();
        }
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SetupLog();

    SKSE::Init(skse);

    //SKSE::GetPapyrusInterface()->Register(EudaBindPapyrusFunctions);

    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    return true;
}