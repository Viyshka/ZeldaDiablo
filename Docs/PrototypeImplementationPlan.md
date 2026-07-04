# Prototype Implementation Plan

This file converts the current design documents into the first C++ vertical slice.

## First playable loop

1. A fixed camera shows one rectangular combat room.
2. The player spawns on the left side of the room.
3. Basic enemies spawn on the right side of the room.
4. Enemies detect the player, chase, wind up, attack and recover.
5. The player can win with regular attacks, block incoming attacks, or counterattack during enemy windup.

## Implemented first

- Player states: idle, moving, attacking, blocking, counterattacking, hurt, dead.
- Enemy states: idle, chasing, windup, attacking, hurt, dead.
- All key tuning values exposed as `UPROPERTY(EditAnywhere, BlueprintReadWrite)`.
- Input mappings: gamepad left stick movement, right stick facing, X attack, LT block.
- Movement and facing are independent; regular attacks and block do not stop movement.
- Placed player Blueprint actors are possessed on play before spawning a fallback pawn.
- Basic combat cone checks for attack, block and counterattack.

## Next likely work

- Replace prototype cubes with Blueprint subclasses and art assets.
- Add attack and windup animation notifies.
- Add UI for player health and enemy counter-readability.
- Add real map asset once the first C++ loop is stable.
- Tune room size, attack radius, block angle and enemy timings in editor.
