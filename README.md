# Amazing Lockpicks

[Skyrim SKSE plugin mod](https://www.nexusmods.com/skyrimspecialedition/mods/108832) that enables unique lockpick models

---

# CommonLibSSE NG

Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR.

[CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) is a fork of the popular [powerof3 fork](https://github.com/powerof3/CommonLibSSE) of the _original_ `CommonLibSSE` library created by [Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) in [2018](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/commit/224773c424bdb8e36c761810cdff0fcfefda5f4a).

# Credits

powerofthree / powerof3 for [Security Overhaul SKSE - Lock Variations](https://www.nexusmods.com/skyrimspecialedition/mods/58224) / [Lock Variations](https://github.com/powerof3/LockVariations/)

CharmedBaryon for [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG)

Parapets / Exit-9B for Ammo Enchanting - [Craft Magical Arrows and Bolts](https://www.nexusmods.com/skyrimspecialedition/mods/79764) / [AmmoEnchanting](https://github.com/Exit-9B/AmmoEnchanting)

brofield for [simpleini](https://github.com/brofield/simpleini)

# User Requirements
* [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
	* Needed for SE/AE

# Building Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`

<img src="https://raw.githubusercontent.com/SkyrimDev/Images/main/images/screenshots/Setting%20Environment%20Variables/VCPKG_ROOT.png" height="150">

# Opening the project

Once you have Visual Studio 2022 installed, you can open this folder in basically any C++ editor, e.g. [VS Code](https://code.visualstudio.com/) or [CLion](https://www.jetbrains.com/clion/) or [Visual Studio](https://visualstudio.microsoft.com/)
- > _for VS Code, if you are not automatically prompted to install the [C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extensions, please install those and then close VS Code and then open this project as a folder in VS Code_

You may need to click `OK` on a few windows, but the project should automatically run CMake!

It will _automatically_ download [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) and everything you need to get started making your new plugin!

To build for AE, SE, or VR, you must go into PCH.h and allow the specific version's define while the other versions are commented out.

Example for building AE:

//#define SKYRIM_VR = TRUE
//#define SKYRIM_SE = TRUE
#define SKYRIM_AE = TRUE

This is necessary because CMake is witchcraft.

> 📜 other templates available at https://github.com/SkyrimScripting/SKSE_Templates

# License
[MIT](LICENSE)
