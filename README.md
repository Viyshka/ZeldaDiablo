# ZeldaDiablo

Unreal Engine 5.7 C++ prototype for a top-down combat game inspired by Zelda-style rooms and Diablo-like close combat.

## Prototype scope

- Fixed top-down combat camera.
- Rectangular combat test room with collision walls.
- Gamepad left stick movement.
- Gamepad right stick facing direction.
- Basic attack on gamepad X.
- Block on gamepad LT.
- Counterattack while blocking against an enemy in windup.
- Basic enemy state machine: idle, chase, windup, attack, hurt, death.
- Tunable values exposed through C++ properties for Blueprint/editor iteration.

## Open

Open `ZeldaDiablo.uproject` in Unreal Engine 5.7. If Unreal asks to rebuild modules, allow it.

## Main classes

- `AZDGameMode` spawns the prototype room and camera when a map starts.
- `AZDPlayerCharacter` owns movement, block, basic attack, counterattack, damage and death state.
- `AZDBasicEnemy` owns detection, chase, windup, attack and death state.
- `AZDCombatRoom` builds a simple test arena and spawns enemies.
- `AZDCombatCameraActor` sets a fixed camera view over the arena.
- `UZDHealthComponent` stores health and death events.

## Current animation set

The local prototype can use `FreeAnimsMixPack` because its Manny and Quinn meshes share the same skeleton as the selected sword animations. Fab/Marketplace packs are kept local and are not pushed to this repository.

- Player mesh: `/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Manny`
- Enemy mesh: `/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Quinn`
- Basic attack: `AS_SwingSword`
- Counterattack placeholder: `AS_Combo`
- Enemy windup/combat stance: `AS_PracticeFencing`
- Hurt/death placeholder: `AS_DyingFromWounds`

`RamsterZ_FreeAnims_Volume1` has useful counterattack ideas, but it is on a different UE4 mannequin skeleton. Use it after retargeting.

If the Fab packs are not present, the C++ prototype still opens and falls back to simple placeholder visuals.

## Camera tuning

Open `/Game/Maps/PrototypeCombat` and select `PrototypeCombatCamera`.

- `Camera Height`: higher means a wider top-down view.
- `Distance To Target`: higher means a more angled view; lower means closer to straight top-down.
- `Field Of View`: higher shows more of the room, but too high can distort the scene.
- `Target Point`: the world point the camera looks at; keep it near the room center.

Good starting values are `Camera Height = 2200`, `Distance To Target = 850`, `Field Of View = 48`.
