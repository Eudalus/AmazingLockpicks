#include "Hooks.h"
#include "Manager.h"

namespace logger = SKSE::log;

namespace Model
{
	namespace Lockpick
	{
		RE::BSResource::ErrorCode RequestModel::thunk(const char* a_modelPath, std::uintptr_t a_modelHandle, const RE::BSModelDB::DBTraits::ArgsType& a_traits)
		{
			auto path = Manager::GetSingleton()->GetLockpickModel(a_modelPath);

			if (path != a_modelPath) {
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
/*
namespace Sound
{
	struct CylinderSqueak
	{
		static void thunk(const char* a_editorID)
		{
			std::string editorID = a_editorID;

			if (const auto& soundData = Manager::GetSingleton()->GetSounds()) {
				if (editorID == "UILockpickingCylinderSqueakA") {
					editorID = soundData->UILockpickingCylinderSqueakA;
				} else {
					editorID = soundData->UILockpickingCylinderSqueakB;				
				}
			}

			return func(editorID.c_str());
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct CylinderStop
	{
		static void thunk(const char* a_editorID)
		{
			const auto&       soundData = Manager::GetSingleton()->GetSounds();
			const std::string editorID = soundData ?
			                                 soundData->UILockpickingCylinderStop :
			                                 a_editorID;

			return func(editorID.c_str());
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct CylinderTurn
	{
		static void thunk(const char* a_editorID)
		{
			const auto&       soundData = Manager::GetSingleton()->GetSounds();
			const std::string editorID = soundData ?
			                                 soundData->UILockpickingCylinderTurn :
			                                 a_editorID;

			return func(editorID.c_str());
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct PickMovement
	{
		static void thunk(const char* a_editorID)
		{
			const auto&       soundData = Manager::GetSingleton()->GetSounds();
			const std::string editorID = soundData ?
			                                 soundData->UILockpickingPickMovement :
			                                 a_editorID;

			return func(editorID.c_str());
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct LockpickingUnlock
	{
		static void thunk(const char* a_editorID)
		{
			const auto&       soundData = Manager::GetSingleton()->GetSounds();
			const std::string editorID = soundData ?
			                                 soundData->UILockpickingUnlock :
			                                 a_editorID;

			return func(editorID.c_str());
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install()
	{
		REL::Relocation<std::uintptr_t> update_lock_angle{ RELOCATION_ID(51096, 51978) };

		stl::write_thunk_call<CylinderSqueak>(update_lock_angle.address() + OFFSET_3(0x159, 0x157, 0x19E));
		stl::write_thunk_call<CylinderStop>(update_lock_angle.address() + OFFSET_3(0xD9, 0xD7, 0x11E));
		stl::write_thunk_call<LockpickingUnlock>(update_lock_angle.address() + OFFSET(0xA1, 0x9F));

		REL::Relocation<std::uintptr_t> rotate_lock{ RELOCATION_ID(51098, 51980) };
		stl::write_thunk_call<CylinderTurn>(rotate_lock.address() + OFFSET(0x33, 0xBC));

		REL::Relocation<std::uintptr_t> update_pick_angle{ RELOCATION_ID(51094, 51976) };
		stl::write_thunk_call<PickMovement>(update_pick_angle.address() + OFFSET_3(0x13F, 0x145, 0x15E));
	}
}
*/

namespace EudaMessageUpdate 
{
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
            Manager::GetSingleton()->allowEnterAudio = true;
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
        // return Manager::GetSingleton()->uniqueLockpickTotal;
        const auto currentManager = Manager::GetSingleton();
        currentManager->allowEnterAudio = true;
        currentManager->shouldUpdateModel = false;
        const int value = currentManager->RecountAndUpdate();
        currentManager->HideLockpickModel(false);
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
        // return Manager::GetSingleton()->uniqueLockpickTotal;
        const auto currentManager = Manager::GetSingleton();
        const int value = currentManager->RecountAndUpdate();
        /*
        if (Manager::GetSingleton()->unknownArg02 && Manager::GetSingleton()->unknownArg03)
        {
            Model::Lockpick::RequestModel::thunk(Manager::GetSingleton()->eudaLockpickVector.at(Manager::GetSingleton()->bestLockpickIndex).path.c_str(),
        Manager::GetSingleton()->unknownArg02, Manager::GetSingleton()->unknownArg03);
        }
        */

        if (currentManager->shouldUpdateModel) {
            // currentManager->HideLockpickModel(true);

            // Model::Lockpick::RequestModel::thunk(Manager::GetSingleton()->eudaLockpickVector.at(Manager::GetSingleton()->bestLockpickIndex).path.c_str(),
            // Manager::GetSingleton()->unknownArg02, Manager::GetSingleton()->unknownArg03);

            currentManager->ReloadLockpickModel();
            currentManager->shouldUpdateModel = false;

            /*
            auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

            if (menuNow)
            {
                logger::info("INSIDE CAN OPEN LOCKPICKING MENU HOOK ---- PRE DELETES");

                auto &dataNow = menuNow->GetRuntimeData();


                //if (dataNow.pickController)
                //{
                //    auto sequenceIterator = dataNow.pickController->sequenceArray.begin();

                //    while (sequenceIterator != dataNow.pickController->sequenceArray.end()) {
                //        sequenceIterator->reset();

                //        ++sequenceIterator;
                //    }

                //    auto activeIterator = dataNow.pickController->activeSequences.begin();

                //    while (activeIterator != dataNow.pickController->activeSequences.end()) {
                //        delete (*activeIterator);

                //        ++activeIterator;
                //    }

                //    auto stringIterator = dataNow.pickController->stringMap.begin();

                //    while (stringIterator != dataNow.pickController->stringMap.end()) {
                //        delete stringIterator->second;

                //        ++stringIterator;
                //    }

                //    auto tempIterator = dataNow.pickController->tempBlendSeqs.begin();

                //    while (tempIterator != dataNow.pickController->tempBlendSeqs.end()) {
                //        tempIterator->reset();

                //        ++tempIterator;
                //    }

                //    delete dataNow.pickController;
                //}


                dataNow.currentPickSequence = nullptr;
                dataNow.pickBreak = nullptr;
                dataNow.pickDamage = nullptr;
                dataNow.pickIntro = nullptr;

                //delete dataNow.lockpick;

                //if (dataNow.lockpick)
                //{
                //    auto pickModelHandle = static_cast<RE::BSResource::ModelHandle *>(dataNow.lockpick);

                //    if (pickModelHandle)
                //    {
                //        pickModelHandle->data.reset();
                //    }

                //    delete dataNow.lockpick;
                //}


                auto pickModelHandle = static_cast<RE::BSResource::ModelHandle *>(dataNow.lockpick);

                pickModelHandle->data.reset();

                //delete pickModelHandle;
            }

            logger::info("INSIDE CAN OPEN LOCKPICKING MENU HOOK --- POST DELETES");

            logger::info("INSIDE CAN OPEN LOCKPICKING MENU HOOK ---- PRE REQUEST");

            REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51081, 51960)};

            auto addressFunction = (std::int32_t(*)(RE::LockpickingMenu *))target.address();
            addressFunction(menuNow.get());

            logger::info("INSIDE CAN OPEN LOCKPICKING MENU HOOK --- POST REQUEST");

            currentManager->shouldUpdateModel = false;
            */
        }

        // menu going to close
        // doesn't catch if swap occurred and lockpicking continued
        // could hook destructor
        if (value == 0) {
            currentManager->HideLockpickModel(false);
        }
        /*
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51081, 51960)};

        auto addressFunction = (std::int32_t(*)(RE::LockpickingMenu *))target.address();

        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow) {
            logger::info("MENU NOW GOOD");
            addressFunction(menuNow.get());
        }
        */

        return value;
    }

    void CanOpenLockpickingMenuHook::Hook()
    {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51087, 51967)};  // CanOpenLockpickingMenu

        stl::write_thunk_call<EudaMessageUpdate::CanOpenLockpickingMenuHook>(
            target.address() + OFFSET(0x31, 0x28));  // call GetItemCount offset
    }

    // PlayerCharacterRemoveItem
    RE::ObjectRefHandle PlayerCharacterRemoveItem::RemoveItem(
        RE::PlayerCharacter* playerCharacter, RE::TESBoundObject* a_item,
                                          std::int32_t a_count, RE::ITEM_REMOVE_REASON a_reason,
                                          RE::ExtraDataList* a_extraList, RE::TESObjectREFR* a_moveToRef,
                                          const RE::NiPoint3* a_dropLoc, const RE::NiPoint3* a_rotate) {
        if (a_item) {
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
        // logger::info("INSIDE UPDATEPICKHEALTH HOOK FUNCTION BEFORE ORIGINAL --- PLAYER LOCKPICK COUNT: {}",
        // RE::PlayerCharacter::GetSingleton()->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA)));
        auto originalValue = func(a1, a2, a3, a4);
        // logger::info("INSIDE UPDATEPICKHEALTH HOOK FUNCTION AFTER ORIGINAL  --- PLAYER LOCKPICK COUNT: {}",
        // RE::PlayerCharacter::GetSingleton()->GetItemCount(RE::TESForm::LookupByID<RE::TESObjectMISC>(0xA)));
        Manager::GetSingleton()->isLockpickHealthUpdating = false;

        return originalValue;
    }

    void UpdatePickHealthHook::Hook()
    {
        //_UpdatePickHealth = REL::Relocation<uintptr_t>(RE::VTABLE_LockpickingMenu[0]).write_vfunc(0x04,
        //UpdatePickHealth); REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51093, 51975)}; // UpdatePickHealth
        // direct
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(51096, 51978)};  // ProcessLockpicking
        // REL::safe_write(target.address(), UpdatePickHealth);

        stl::write_thunk_call<EudaMessageUpdate::UpdatePickHealthHook>(
            target.address() + OFFSET(0xED, 0xEB));  // call UpdatePickHealth offset
        // stl::write_thunk_call<EudaMessageUpdate::UpdatePickHealthHook>(target.address());
        //_UpdatePickHealth = REL::Relocation<uintptr_t>(REL::Relocation_ID.safe_write(0x04, UpdatePickHealth);
    }

    // LockpickingMenuMessageHook
    RE::UI_MESSAGE_RESULTS LockpickingMenuMessageHook::ProcessMessage(RE::IMenu* menu, RE::UIMessage& a_message)
    {
        /*
        if (counter % 100 == 0)
        {
            logger::info("{} PROCESS MESSAGE HOOK", counter);
        }
        counter++;
        */

        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow) {
            // logger::info("INSIDE LOCKPICK PROCESS MESSAGE HOOK: MENU NAME IS: {}", a_message.menu.c_str());

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

            // RE::LockpickingMenu::RUNTIME_DATA &dataNow = menuNow->GetRuntimeData();

            // const auto pickModelHandle = static_cast<RE::BSResource::ModelHandle *>(dataNow.lockpick);
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
        /*
        if (counter % 100 == 0)
        {
            logger::info("{} PROCESS MESSAGE HOOK", counter);
        }
        counter++;
        */

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

            // RE::LockpickingMenu::RUNTIME_DATA &dataNow = menuNow->GetRuntimeData();

            // const auto pickModelHandle = static_cast<RE::BSResource::ModelHandle *>(dataNow.lockpick);
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
        /*
        if (counter % 100 == 0)
        {
            logger::info("{} --- ADVANCE MOVIE HOOK --- INTERVAL: {} --- CURRENT TIME: {}", counter, a_interval,
        a_currentTime);
        }
        counter++;
        */

        RE::UI_MESSAGE_RESULTS originalValue = _AdvanceMovie(menu, a_interval, a_currentTime);

        auto menuNow = RE::UI::GetSingleton()->GetMenu<RE::LockpickingMenu>(RE::LockpickingMenu::MENU_NAME);

        if (menuNow) {
            RE::LockpickingMenu::RUNTIME_DATA& dataNow = menuNow->GetRuntimeData();

            const auto pickModelHandle = static_cast<RE::BSResource::ModelID*>(dataNow.lockpick);

            // RE::NiTimeController::StartAnimations(pickModelHandle->data->AsFadeNode());

            // RE::NiTimeController::StartAnimations(dataNow.pickController->sequenceArray[3]->As);

            // updateValue += a_interval * 1000;

            updateValue += a_interval * 100;

            /*
            RE::NiTimeControllerPtr firstController = dataNow.lockController->next->next;

            RE::NiTimeControllerPtr secondController = firstController->next;

            RE::NiPointer<RE::BSShaderProperty> firstShaderPointer =
                static_cast<RE::NiPointer<RE::BSShaderProperty>> (firstController->target);

            //RE::BSShaderProperty *firstShaderProperty =
            //    static_cast<RE::BSShaderProperty *>(firstController->target);

            //RE::BSShaderProperty *secondShaderProperty =
           //     static_cast<RE::BSShaderProperty *>(secondController->target);

            firstShaderProperty->alpha = 0;
            //secondShaderProperty->alpha = 0;
            */

            // RE::NiTimeController::StartAnimations(firstController.get());
            //  RE::NiTimeController::StartAnimations(secondController);

            // pickModelHandle->data->AsFadeNode()->world.scale += updateValue;
            /*
            pickModelHandle->data
                ->GetChildren()[0]  // 21
                ->AsNode()
                ->GetChildren()[0]  // 22
                ->AsNode()
                ->GetChildren()[1]
                ->AsTriShape()
                ->world.scale += updateValue;
            */
            /*
            pickModelHandle->data
                ->GetChildren()[0]  // 21
                ->AsNode()
                ->GetChildren()[2]
                ->AsTriShape()  // 35
                ->GetGeometryRuntimeData()
                .properties[RE::BSGeometry::States::kEffect]  //->Update(updateValue); // 36
                ->GetControllers()[0]
                .Start(updateValue);  // 37

            RE::BSShaderProperty *shadeProp =

                static_cast<RE::BSShaderProperty *>(
            pickModelHandle->data
                ->GetChildren()[0]  // 21
                ->AsNode()
                ->GetChildren()[0]  // 22
                ->AsNode()
                ->GetChildren()[1]
                ->AsTriShape()  // 29
                ->GetGeometryRuntimeData()
                .properties[RE::BSGeometry::States::kEffect]
                .get());  //->Update(updateValue); // 30
                //->GetControllers()[0].Start(static_cast<int>(updateValue) % 8);
            */
        }

        return originalValue;
    }

    void LockpickingMenuMovieHook::Hook()
    {
        // 0x05 address of AdvanceMovie function
        _AdvanceMovie = REL::Relocation<uintptr_t>(RE::VTABLE_LockpickingMenu[0]).write_vfunc(0x05, AdvanceMovie);
    }
}