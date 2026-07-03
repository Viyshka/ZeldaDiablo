import unreal

paths = [
    "/Game/FreeAnimsMixPack/Animation/AS_SwingSword",
    "/Game/FreeAnimsMixPack/Animation/AS_Combo",
    "/Game/FreeAnimsMixPack/Animation/AS_PracticeFencing",
    "/Game/FreeAnimsMixPack/Animation/AS_DyingFromWounds",
    "/Game/FreeSampleAnimationSet/Animations/MaleLocomotionSet/Mannequin/RootMotion/A_RunFwd_Loop",
    "/Game/FreeSampleAnimationSet/Animations/FemaleLocomotionSet/Mannequin/RootMotion/A_JogFwd_Loop",
    "/Game/RamsterZ_FreeAnims_Volume1/AnimationSequence/1HM/1HM_Idle",
    "/Game/RamsterZ_FreeAnims_Volume1/AnimationSequence/1HM/1HM_AerialSwing01_Forward",
    "/Game/RamsterZ_FreeAnims_Volume1/AnimationSequence/1HM/1HM_AerialSwing01_2HitCombo",
    "/Game/RamsterZ_FreeAnims_Volume1/AnimationSequence/Paired/Paired_CounterPunch_PalmStrike_Att",
    "/Game/RamsterZ_FreeAnims_Volume1/Demo/Mannequin/Character/Mesh/SK_Mannequin",
    "/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Manny",
    "/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Manny_Simple",
    "/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Quinn",
    "/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Quinn_Simple",
]

lines = []
for path in paths:
    asset = unreal.load_asset(path)
    if not asset:
        lines.append(f"{path}|MISSING")
        continue

    asset_class = asset.get_class().get_name()
    skeleton_path = "None"

    try:
        skeleton = asset.get_editor_property("skeleton")
        if skeleton:
            skeleton_path = skeleton.get_path_name()
    except Exception:
        pass

    lines.append(f"{path}|{asset_class}|{skeleton_path}")

output_path = unreal.Paths.project_saved_dir() + "AnimationAssetReport.txt"
with open(output_path, "w", encoding="utf-8") as output:
    output.write("\n".join(lines))
