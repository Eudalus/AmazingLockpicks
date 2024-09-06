#include "Hooks.h"
#include "Manager.h"

namespace logger = SKSE::log;

namespace Model
{
    namespace Lock
    {
        RE::BSResource::ErrorCode RequestModel::thunk(const char* a_modelPath, std::uintptr_t a_modelHandle,
                                        const RE::BSModelDB::DBTraits::ArgsType& a_traits)
        {
            if (Manager::GetSingleton()->allowLockSwap)
            {
                return func(a_modelPath, a_modelHandle, a_traits);
            }

            Manager::GetSingleton()->allowLockSwap = true;

            return RE::BSResource::ErrorCode::kNone;
        }

        void RequestModel::Install()
        {
            REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51081, 51960)};

            stl::write_thunk_call<Lock::RequestModel>(target.address() + OFFSET_3(0xC6, 0xBC, 0xBB));
        }
    }

	namespace Lockpick
	{
		RE::BSResource::ErrorCode RequestModel::thunk(const char* a_modelPath, std::uintptr_t a_modelHandle, const RE::BSModelDB::DBTraits::ArgsType& a_traits)
		{
			const auto path = Manager::GetSingleton()->GetLockpickModel(a_modelPath);

			if (path != a_modelPath)
            {
				logger::info("	Lockpick : {} -> {}", a_modelPath, path);
			}

			return func(path.c_str(), a_modelHandle, a_traits);
		}
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(51081, 51960) };

		stl::write_thunk_call<Lockpick::RequestModel>(target.address() + OFFSET_3(0xA1, 0x97, 0x96));
	}
}

namespace EudaMessageUpdate 
{
    // EnterLockIntroHook
    std::uintptr_t EnterLockIntroHook::thunk(RE::LockpickingMenu* menu, RE::NiControllerManager* niManager,
        RE::NiControllerSequence* niSequence)
    {
        const auto currentManager = Manager::GetSingleton();
        
        if (currentManager->allowLockIntro && menu)
        {
            auto& runtimeData = menu->GetRuntimeData();

            currentManager->originalLockRotationCenter.x = runtimeData.lockRotCenter.x;
            currentManager->originalLockRotationCenter.y = runtimeData.lockRotCenter.y;
            currentManager->originalLockRotationCenter.z = runtimeData.lockRotCenter.z;

            return func(menu, niManager, niSequence);
        }
        else if (menu)
        {
            auto& runtimeData = menu->GetRuntimeData();

            runtimeData.lockRotate->Activate(0, 1, 1.0, -1.0, 0, true);
            runtimeData.lockRotCenter.x = currentManager->originalLockRotationCenter.x;
            runtimeData.lockRotCenter.y = currentManager->originalLockRotationCenter.y;
            runtimeData.lockRotCenter.z = currentManager->originalLockRotationCenter.z;
            //logger::info("UNMODIFIED --- PICK BREAK SECONDS: {}", runtimeData.pickBreakSeconds);
            //runtimeData.pickBreakSeconds = currentManager->CalculatePickBreak(menu->GetTargetReference()->GetLockLevel()) * currentManager->CalculateQualityModifier();
            //logger::info("MODIFIED ----- PICK BREAK SECONDS: {}", runtimeData.pickBreakSeconds);
        }

        Manager::GetSingleton()->allowLockIntro = true;

        return NULL;
    }

    void EnterLockIntroHook::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51071, 51950)};  // LockpickingMenu::ProcessMouseMove4

        stl::write_thunk_call<EudaMessageUpdate::EnterLockIntroHook>(target.address() + OFFSET(0x103, 0x103));  // call SetupControllers
    }

    // EnterSoundEffectHook - SE version
    void EnterSoundEffectHookSE::thunk(char* soundPath)
    {
        if (Manager::GetSingleton()->allowEnterAudio)
        {
            func(soundPath);
        }

        Manager::GetSingleton()->allowEnterAudio = true;
    }

    void EnterSoundEffectHookSE::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51087, 51967)};  // CanOpenLockpickingMenu

        stl::write_thunk_branch<EudaMessageUpdate::EnterSoundEffectHookSE>(target.address() + OFFSET(0xB5, 0xAC));  // PlaySoundEffect, should never call AE address
    }

    //EnterSoundEffectHook - AE version
    std::uintptr_t EnterSoundEffectHookAE::thunk(char* soundPath)
    {
        if (Manager::GetSingleton()->allowEnterAudio)
        {
            return func(soundPath);
        }

        Manager::GetSingleton()->allowEnterAudio = true;

        return NULL;
    }

    void EnterSoundEffectHookAE::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51087, 51967)};  // CanOpenLockpickingMenu

        stl::write_thunk_branch<EudaMessageUpdate::EnterSoundEffectHookAE>(target.address() + OFFSET(0xB5, 0xAC));  // PlaySoundEffect, should never call SE address
    }

    // UnknownSetupHook
    std::int32_t UnknownSetupHook::thunk(RE::Character* character, RE::TESBoundObject* lockpick)
	{
        return Manager::GetSingleton()->uniqueLockpickTotal;
    }

	void UnknownSetupHook::Hook()
	{
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51084, 51963)};  // UnknownSetupHook

        stl::write_thunk_call<EudaMessageUpdate::UnknownSetupHook>(target.address() + OFFSET(0x5B6, 0x5C1));  // call GetItemCount offset
    }

	// TryBeginLockPickingHook
    std::int32_t TryBeginLockPickingHook::thunk(RE::Character* character, RE::TESBoundObject* lockpick)
	{
        const auto currentManager = Manager::GetSingleton();
        currentManager->allowEnterAudio = true;
        currentManager->allowLockSwap = true;
        currentManager->allowLockIntro = true;
        currentManager->shouldUpdateModel = false;
        const int value = currentManager->RecountAndUpdate();
        currentManager->shouldUpdateModel = false;

        return value;
    }

	void TryBeginLockPickingHook::Hook()
	{
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51080, 51959)};  // TryBeginLockpicking

        stl::write_thunk_call<EudaMessageUpdate::TryBeginLockPickingHook>(
            target.address() + OFFSET(0xEE, 0xE3));  // call GetItemCount offset
    }

	// CanOpenLockpickingMenuHook
	std::int32_t CanOpenLockpickingMenuHook::thunk(RE::Character* character, RE::TESBoundObject* lockpick)
    {
        const auto currentManager = Manager::GetSingleton();
        const int value = currentManager->RecountAndUpdate();

        if (currentManager->shouldUpdateModel)
        {
            currentManager->ReloadLockpickModel();
            currentManager->shouldUpdateModel = false;
        }

        const auto uiNow = RE::UI::GetSingleton();
        const auto menuNow = uiNow ? uiNow->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME) : nullptr;
        RE::TESObjectREFR* objectRef;

        if (menuNow && (objectRef = RE::LockpickingMenu::GetTargetReference()))
        {
            auto& runtimeData = menuNow->GetRuntimeData();

            runtimeData.pickBreakSeconds = currentManager->CalculatePickBreak(objectRef->GetLockLevel()) *
                                                         currentManager->CalculateQualityModifier();
        }

        return value;
    }

    void CanOpenLockpickingMenuHook::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51087, 51967)};  // CanOpenLockpickingMenu

        stl::write_thunk_call<EudaMessageUpdate::CanOpenLockpickingMenuHook>(
            target.address() + OFFSET(0x31, 0x28));  // call GetItemCount offset
    }

///// ----- deprecated, debug, and testing functions -----
    /*
    // PlayerCharacterRemoveItem
    RE::ObjectRefHandle PlayerCharacterRemoveItem::RemoveItem(
        RE::PlayerCharacter* playerCharacter, RE::TESBoundObject* a_item,
                                          std::int32_t a_count, RE::ITEM_REMOVE_REASON a_reason,
                                          RE::ExtraDataList* a_extraList, RE::TESObjectREFR* a_moveToRef,
                                          const RE::NiPoint3* a_dropLoc, const RE::NiPoint3* a_rotate) {
        if (a_item)
        {
            logger::info("INSIDE PLAYER CHARACTER REMOVE ITEM HOOK --- ITEM: {} --- COUNT: {}",
                         std::format("{:x}", a_item->formID), a_count);
        }

        return _RemoveItem(playerCharacter, a_item, a_count, a_reason, a_extraList, a_moveToRef, a_dropLoc, a_rotate);
    }

    void PlayerCharacterRemoveItem::Hook()
    {
        // 0x56 address of RemoveItem
        _RemoveItem = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0x56, RemoveItem);
    }

    // UpdatePickHealthHook
    std::int32_t UpdatePickHealthHook::thunk(RE::LockpickingMenu* a1, std::int64_t a2, std::int64_t a3, std::int64_t a4)
    {
        Manager::GetSingleton()->isLockpickHealthUpdating = true;
        auto originalValue = func(a1, a2, a3, a4);
        Manager::GetSingleton()->isLockpickHealthUpdating = false;

        return originalValue;
    }

    void UpdatePickHealthHook::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51096, 51978)};  // ProcessLockpicking

        stl::write_thunk_call<EudaMessageUpdate::UpdatePickHealthHook>(
            target.address() + OFFSET(0xED, 0xEB));  // call UpdatePickHealth offset
    }

    // LockpickingMenuMessageHook
    RE::UI_MESSAGE_RESULTS LockpickingMenuMessageHook::ProcessMessage(RE::IMenu* menu, RE::UIMessage& a_message)
    {
        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow)
        {
            // if (a_message.type == RE::UI_MESSAGE_TYPE::kUpdate)
            //{
            //     logger::info("{} ----- kUpdate data: {}",a_message.menu.c_str(), a_message.data ?
            //     a_message.data->unk08 : 0);
            // }
            if (a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
                logger::info("{} ----- kShow data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kReshow) {
                logger::info("{} ----- kReshow data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide) {
                logger::info("{} ----- kHide data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
                logger::info("{} ----- kForceHide data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent) {
                RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

                RE::TESObjectMISC* lockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);

                logger::info("CURRENT LOCKPICK (0xA) QUANITY: {} ", playerCharacter->GetItemCount(lockpickObject));

                logger::info("{} ----- kScaleformEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
                // return RE::UI_MESSAGE_RESULTS::kIgnore;
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUserEvent) {
                logger::info("{} ----- kUserEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
                logger::info("{} ----- kInventoryUpdate data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUserProfileChange) {
                logger::info("{} ----- kUserProfileChange data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kMUStatusChange) {
                logger::info("{} ----- kMUStatusChange data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kResumeCaching) {
                logger::info("{} ----- kResumeCaching data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUpdateController) {
                logger::info("{} ----- kUpdateController data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kChatterEvent) {
                logger::info("{} ----- kChatterEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            }
            // else
            //{
            //     logger::info("UNKNOWN UI_MESSAGE_TYPE!");
            // }
        }

        return _ProcessMessage(menu, a_message);
    }

    void LockpickingMenuMessageHook::Hook()
    {
        // 0x04 address of ProcessMessage function
        _ProcessMessage = REL::Relocation<uintptr_t>(RE::VTABLE_LockpickingMenu[0]).write_vfunc(0x04, ProcessMessage);
    }

    // EudaIMenuMessageHook
    RE::UI_MESSAGE_RESULTS EudaIMenuMessageHook::ProcessMessage(RE::IMenu* menu, RE::UIMessage& a_message)
    {
        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow)
        {
            // logger::info("INSIDE LOCKPICK PROCESS MESSAGE HOOK: MENU NAME IS: {}", a_message.menu.c_str());

            if (a_message.type == RE::UI_MESSAGE_TYPE::kUpdate) {
                logger::info("{} ----- kUpdate data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
                logger::info("{} ----- kShow data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kReshow) {
                logger::info("{} ----- kReshow data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide) {
                logger::info("{} ----- kHide data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
                logger::info("{} ----- kForceHide data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent) {
                RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

                RE::TESObjectMISC* lockpickObject = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA);

                logger::info("CURRENT LOCKPICK (0xA) QUANITY: {} ", playerCharacter->GetItemCount(lockpickObject));

                logger::info("{} ----- kScaleformEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
                // return RE::UI_MESSAGE_RESULTS::kIgnore;
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUserEvent) {
                logger::info("{} ----- kUserEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
                logger::info("{} ----- kInventoryUpdate data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUserProfileChange) {
                logger::info("{} ----- kUserProfileChange data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kMUStatusChange) {
                logger::info("{} ----- kMUStatusChange data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kResumeCaching) {
                logger::info("{} ----- kResumeCaching data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kUpdateController) {
                logger::info("{} ----- kUpdateController data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            } else if (a_message.type == RE::UI_MESSAGE_TYPE::kChatterEvent) {
                logger::info("{} ----- kChatterEvent data: {}", a_message.menu.c_str(),
                             a_message.data ? a_message.data->unk08 : 0);
            }
            // else
            //{
            //     logger::info("UNKNOWN UI_MESSAGE_TYPE!");
            // }
        }

        return _ProcessMessage(menu, a_message);
    }

    void EudaIMenuMessageHook::Hook()
    {
        // 0x04 address of ProcessMessage function
        _ProcessMessage = REL::Relocation<uintptr_t>(RE::VTABLE_IMenu[0]).write_vfunc(0x04, ProcessMessage);
    }

    // LockpickingMenuMovieHook
    RE::UI_MESSAGE_RESULTS LockpickingMenuMovieHook::AdvanceMovie(RE::IMenu* menu, float a_interval,
                                                                std::uint32_t a_currentTime)
    {

        RE::UI_MESSAGE_RESULTS originalValue = _AdvanceMovie(menu, a_interval, a_currentTime);

        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow)
        {
            RE::LockpickingMenu::RUNTIME_DATA& dataNow = menuNow->GetRuntimeData();

            const auto pickModelHandle = static_cast<RE::BSResource::ModelID*>(dataNow.lockpick);
        }

        return originalValue;
    }

    void LockpickingMenuMovieHook::Hook()
    {
        // 0x05 address of AdvanceMovie function
        _AdvanceMovie = REL::Relocation<uintptr_t>(RE::VTABLE_LockpickingMenu[0]).write_vfunc(0x05, AdvanceMovie);
    }
    */
}