#pragma once

#include "Manager.h"
#include "BSResource.h"

namespace stl
{
    using namespace SKSE::stl;

    template <class T>
    void write_thunk_call(std::uintptr_t a_src)
	{
        auto& trampoline = SKSE::GetTrampoline();
        SKSE::AllocTrampoline(14);
        T::func = trampoline.write_call<5>(a_src, T::thunk);
    }
}


namespace Model
{
	namespace Lockpick
	{
		struct RequestModel
		{
			//static std::uint8_t thunk(const char *a_modelPath, std::uintptr_t a_unk02, std::uintptr_t a_unk03);
			static RE::BSResource::ErrorCode thunk(const char* a_modelPath, std::uintptr_t a_modelHandle, const RE::BSModelDB::DBTraits::ArgsType& a_traits);

			static inline REL::Relocation<decltype(thunk)> func;
		};
	}

	void Install();
}

namespace Sound
{
	void Install();
}

// move definitions into Hooks.cpp, create declarations here
namespace EudaMessageUpdate
{

	/// <summary>
	/// Uses a hook to make the Lockpicking Menu display the member variable uniqueLockpickTotal
	/// </summary>
	struct UnknownSetupHook
	{
		static std::int32_t thunk(RE::Character* character, RE::TESBoundObject* lockpick);

		static inline REL::Relocation<decltype(thunk)> func;

		static void Hook();
	};

	/// <summary>
	/// Uses a hook to make the Lockpicking Menu use the member variable uniqueLockpickTotal
	/// when determining if the player has enough lockpicks to begin lockpicking
	/// </summary>
	struct TryBeginLockPickingHook
	{
		static std::int32_t thunk(RE::Character* character, RE::TESBoundObject* lockpick);

		static inline REL::Relocation<decltype(thunk)> func;

		static void Hook();
	};

	/// <summary>
	/// Uses a hook to make the Lockpicking Menu use the member variable uniqueLockpickTotal
	/// when determining if the player has enough lockpicks to continue lockpicking
	/// Potentially also runs after initially opening the Lockpicking Menu once
	/// </summary>
	struct CanOpenLockpickingMenuHook
	{
		static std::int32_t thunk(RE::Character* character, RE::TESBoundObject* lockpick);

		static inline REL::Relocation<decltype(thunk)> func;

		static void Hook();
	};

	struct PlayerCharacterRemoveItem
	{
		static RE::ObjectRefHandle RemoveItem(RE::PlayerCharacter* playerCharacter, RE::TESBoundObject* a_item, std::int32_t a_count,
			RE::ITEM_REMOVE_REASON a_reason, RE::ExtraDataList* a_extraList,
			RE::TESObjectREFR* a_moveToRef, const RE::NiPoint3* a_dropLoc = 0,
			const RE::NiPoint3* a_rotate = 0);

		static inline REL::Relocation<decltype(RemoveItem)> _RemoveItem;

		static void Hook();
	};

	struct UpdatePickHealthHook
	{
		static std::int32_t thunk(RE::LockpickingMenu* a1, std::int64_t a2, std::int64_t a3, std::int64_t a4);

		static inline REL::Relocation<decltype(thunk)> func;

		static void Hook();
	};

	class LockpickingMenuMessageHook
	{
    public:
		//static inline int counter = 0;

		static RE::UI_MESSAGE_RESULTS ProcessMessage(RE::IMenu* menu, RE::UIMessage& a_message);

		static inline REL::Relocation<decltype(ProcessMessage)> _ProcessMessage;

		static void Hook();
	};

	class EudaIMenuMessageHook
	{
    public:
		// static inline int counter = 0;

		static RE::UI_MESSAGE_RESULTS ProcessMessage(RE::IMenu* menu, RE::UIMessage& a_message);

		static inline REL::Relocation<decltype(ProcessMessage)> _ProcessMessage;

		static void Hook();
	};

	class LockpickingMenuMovieHook
	{
    public:
		//static inline int counter = 0;
		static inline float updateValue = 0;

		static RE::UI_MESSAGE_RESULTS AdvanceMovie(RE::IMenu* menu, float a_interval, std::uint32_t a_currentTime);

		static inline REL::Relocation<decltype(AdvanceMovie)> _AdvanceMovie;

		static void Hook();
	};
}
