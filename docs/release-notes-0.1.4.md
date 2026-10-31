# Virtua Cop VR 0.1.4 development preview

Brings the user-requested zoom controls and visual help from VC2 into VC1.

- Tap and release either grip to toggle the original game's camera zoom animations. `Game zoom` in the menu controls the same choice. Zoom starts Off on every new launch; other preferences continue to persist.
- A purple `Zoom enabled` or `Zoom disabled` notice appears for one second. How to Play shows `Grip - Toggle zoom` pointing inward from both controllers, with the matching menu footer.
- Existing grip+stick size adjustments keep working and consume the gesture. Menu, Service, tracking or focus interruptions discard unfinished taps.
- World magnification uses VC1's verified native camera data and30-degree reference, captured with submitted geometry. Aiming uses its inverse. Real depth/head tracking, held guns, native HUD and infinite clouds retain their established behavior.

The physically accepted0.1.3performance fixes are retained. With zoom Off, the45-native-second comparison against0.1.3matches all22state/image/audio captures byte-for-byte. Native quickstate and raster/coverage limitations remain documented. Quest/PCVR software checks and fresh matching builds are recorded in the build evidence; physical0.1.4zoom/help acceptance is pending. No GitHub publication is authorized.
