# ZeldaDiablo

Unreal Engine 5.7 C++ prototype for a top-down combat game inspired by Zelda-style rooms and Diablo-like close combat.

## Prototype scope

- Fixed top-down combat camera.
- Rectangular combat test room with collision walls.
- Player movement with WASD.
- Basic attack on left mouse button.
- Block on right mouse button.
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

