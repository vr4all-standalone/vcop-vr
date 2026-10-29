# Virtua Cop VR

## Downloads

- [Quest APK](https://github.com/vr4all-standalone/vcop-vr/releases/latest/download/vcop-vr-quest.apk)
- [Windows PCVR](https://github.com/vr4all-standalone/vcop-vr/releases/latest/download/vcop-vr-pcvr.zip)
- [Complete matching source](https://github.com/vr4all-standalone/vcop-vr/releases/latest/download/vcop-vr-source.zip)

Supply your own supported arcade files. The named source ZIP contains the complete port, pinned MAME bundle, patches and SDL/OpenXR sources; GitHub’s automatic source ZIP contains this project page.


<!-- overseer-20261028-update -->
Prepared version **0.1.6**, Android code **7**. Public release: Quest, PCVR and complete corresponding source downloads are available on the project page.

The Quest setup screen now uses the wide purple VR4ALL launch panel, with file/access status, help and a manual **Play** button. It enters VR automatically once the real game-file checks pass; returning from Quit or an error leaves Play available without a duplicate automatic launch.

The Arcade gun's body and complete grip follow logical player identity across hand swaps: P1 red / P2 blue. Grey screws, metal and lens details keep their original materials; other gun choices retain their appearance. Displayed **Gun angle** is -30..30 degrees in one-degree steps. New displayed 0 uses the old physical -10-degree pose consistently for the model, muzzle, laser, aiming and menu pointer. Every legacy saved angle resets to displayed 0 once; a persistent migration marker preserves later adjustments and other saved preferences.

This update has fresh software tests and Windows/Quest builds. Its APK was installed in place and matched the candidate hash; the new setup, colors and angle behavior have not been physically observed in a headset. Earlier accepted gameplay features remain separate historical evidence.

Complete matching source is `vcop-vr-source.zip`, including the pinned MAME bundle, full patch stack, exact SDL/OpenXR sources, manifests and licenses. See [build instructions](docs/BUILD.md) and [this update](docs/release-notes-0.1.6.md).
<!-- /overseer-20261028-update -->

Experimental port of original arcade Virtua Cop Revision B for Quest 3/3S and Windows OpenXR. Version 0.1.6, Android code7/package `com.vrlightgunhub.vcop`. This experimental VR port is available as Quest, PCVR and complete-source downloads.

This uses native Model 2 geometry, materials, HUD, sound, game logic and pacing. Each eye renders signed camera-space geometry with head rotation and lean. VC1's native cloud polygons render infinitely far away with their own transparency; original hearts, ammo cylinders and score graphics remain native HUD elements. In-game gun follows each player's native equipment. Its standard model is the original scene pistol and hand used cosmetically; Arcade and Zapper are the other two choices.

Quest setup requires your owned **Revision B** media under `Download/vcop-vr/`. Supply `vcop.zip` and `model1io2.zip`, or one complete `vcop.zip` with all 28 verified ROM payloads. An optional `hd44780.zip` may supply the LCD font if absent from the I/O archive. The LCD font has MAME's known BAD_DUMP status. Existing top-level Virtua Cop archives belong to other apps and are preserved. The app verifies actual size, CRC32 and SHA1 before launch. No game data is distributed.

For PCVR, enable the headset's OpenXR runtime, place the owned archives under the portable folder's `roms/`, and run `run-pcvr.cmd`. Settings and native NVRAM use its independent `userdata/` folder. `launch-pcvr.ps1 -RomDirectory D:/MyRoms` selects another media folder.

The purple menu includes Resume, Recenter, Gun, Gun angle, Reload, Two players, Laser, Haptics, World size, Panel distance, Gun size, Wider view, View width, Game zoom, How to play, Open Service Menu and Quit. Tap/release either grip to toggle native zoom, with a brief purple status message; it starts Off each launch. The grip help leader targets the squeeze button. Native shooting/reload white strobes are hidden during VR rendering while native gun reads remain intact. In-game gun follows Rifle, Automatic, Machine Gun, Shot Gun and Magnum pickups independently for each player. Common settings follow the overseer contract. Native Gun Setting calibration remains available through the service menu. Held controls are released across focus, menu and ownership changes.

Build the host with CMake/Ninja and the engine with `tools/build-engine.ps1`, supplying a dedicated source/build path without spaces. Both targets use pinned MAME 0.289 commit `f34f02505e32c1993c6a782b6814232cbfc74e36`; `tools/sync_engine_patches.py` verifies the complete cumulative patch state. `tools/build-quest.ps1` uses a separate signing key and builds the ARM64 host, engine and APK. Run `python -B -m unittest discover -s tests -v` and CTest after configuring the host. Private ROM, native-asset and capture tests run only when their own research fixtures exist.

See [validation](docs/validation.md), [design](docs/design.md), [credits](CREDITS.md) and `licenses/` for evidence, limitations and attribution.
