# Virtua Cop VR 0.1.5 development preview

- How to Play's grip leader and dot now land on the actual squeeze button on both controllers. Inward text placement and the other labels retain their existing layout.
- Hides the original white screen during native shooting/reload/empty gun reads in VR. The change is render-only: original gun reads, cooldown, ammo, game logic and sound are unchanged. Service/calibration and non-VR rendering retain the original mask.
- Verified every genuine native pickup through real ROM extraction, each player's state and the production held-gun renderer: Rifle, Automatic, Machine Gun, Shot Gun and Magnum. In-game gun already follows these independently for P1/P2; no invented extra meshes or equipment overrides were added. Rifle and Machine Gun return to the standard pistol when exhausted, and taking damage resets special weapons according to the original game.

Retains the physically accepted0.1.4 grip zoom, one-second status notice, speed and sound. The signed Quest and PCVR builds, software results and exact source identities are recorded in the build evidence. Actual0.1.5 headset help/flash/pickup acceptance remains pending. No GitHub publication is authorized.
