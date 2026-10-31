# VC1 implementation ledger

Owned source: ports/vcop-vr only. Baseline ports/vcop2-vr accepted0.2.5 is read-only. Root owns registry/catalog/headset. Pin MAME0.289 f34f02505e32c1993c6a782b6814232cbfc74e36.

- Task1 RED: independent identity tests failed as expected for absent VC1 engine lock and manifest.
- Reused common OpenXR, rendering, assets, UI/input/persistence modules, portable dependency source and toolchain scripts. Game-specific state/camera/calibration/weapon mappings deliberately excluded and awaiting independently verified VC1 addresses.
- Ruling: shared dirty workspace and separate new port + dedicated compiler caches used; no isolated Git branch because entire ports tree is untracked and other chats own unrelated edits. Cost if wrong: source attribution needs recorded hashes instead of commit-only identity.

- Task1: complete. Independent project, verified 28-ROM original Model2/Model1IO2 baseline and native P1/P2/service captures. Initial firmware dependency omission corrected; no native readiness claim from the incomplete attempt.
- Task2: complete. VC1 mode/player/service, float SRAM calibration, focus producer/consumers, accepted-shot stores, six native assets and actual geometry handle anchors independently established. State/camera/accepted-shot/asset tests were RED then GREEN.
- Task3: complete in software. Native surrounding stereo, strict VC1 HUD, safe shared controls, three gun choices and native equipment following. Actual cloud parallax test was RED then rotation-only sky pass GREEN. 22/22 current CTest pass with assertions active.
- Task4: current Windows/ARM64 builds and signed Quest APK complete; 33/33 Python/Java tests pass. Portable package, root review, serialized headset log validation and physical user play remain separate evidence.
- Ruling: automatic native reload path548e8 is source verified but not separately exercised live; preserve the implementation with that runtime limitation reported. Cost if wrong: haptic/flash acknowledgement could be missed in that operator option until a live check.
- Ruling: strict neutral-raster98% exact gate remains not passed for one original ground-texture frame (97.016%, MAE0.1363). Preserve the accepted common renderer while root investigates native shading rounding; do not weaken the gate or claim exact parity. Cost if wrong: a subtle native material difference may remain visible.

- Final native service cycle: exit0/diagnostic_ok true, enters Gun Mark Check phase2 with distinct P1/P2 positions, returns through Gun Setting and main Exit; service0 by55sec, native attract mode4 at57sec.
- Independent review: all22 CTests pass; exact six camera operand guards pass240 additional source assertions. Cloud rotation/lean/occlusion/hit exclusion confirmed by independent actual-capture GPU harness. Proposed reference-mode cloud duplication finding withdrawn after inspecting existing outer immersive guard; no production change warranted.
- Ruling: sampled native wider draw list has no underfoot/behind mesh even in a raw double-sided triangle audit. Preserve actual authored content without invented terrain; report full surrounding geometry coverage as unproven and require user headset assessment. Cost if wrong: the user sees a finite scenery edge when looking down or behind.

- Final source audit caught and corrected a remaining inherited missing-ROM error hint; a real Java validator regression test failed with Download/vcop.zip, then passed with Download/vcop-vr/. An unused native observer was also mapped to verified VC1 addresses and explicitly names raw accepted flags rather than cumulative shots. Final Python/Java suite now34/34. Rebuilt APK for corrected hint before final delivery.

- Independent neutral replay correction: strict98% exact gate fails2/9 original captures,1728=97.016%,2016=96.984%, maximumRGBMAE0.1363, foreground0mismatches. The90% diagnostic sweep is measurement only, not a strict pass.
