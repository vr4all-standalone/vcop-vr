# Virtua Cop VR 0.1.2 development preview

Reduces repeated VR aiming work and native emulation overhead. Both hands reuse the scene's prepared geometry, serial clocks share the original phase-aligned edge source, symmetric timers avoid per-edge rearming, already-pending sound IRQs avoid redundant synchronization, and the verified native video-sync polling loop folds complete iterations while preserving exact CPU cycles and registers.

The original native audio recording, game state and replay images remain byte-identical in the controlled before/after checks. Current tests:27native/GL and34Python/Java checks passed. Quest and PCVR previews use the same pinned MAME source and independent original VC1 app identity. Android updates preserve settings, NVRAM and game files; no media is included.

Full-speed sustained headset gameplay/audio acceptance is pending. Existing finite native scenery coverage and the strict neutral-raster rounding limitation remain documented in validation.md. PCVR has not been physically tested. This is a local preview; no GitHub release has been approved.
