import unreal

MAP_DIR = "/Game/Maps"
MAP_PATH = f"{MAP_DIR}/PrototypeCombat"

unreal.EditorAssetLibrary.make_directory(MAP_DIR)
unreal.EditorLevelLibrary.new_level(MAP_PATH)

room_class = unreal.load_class(None, "/Script/ZeldaDiablo.ZDCombatRoom")
camera_class = unreal.load_class(None, "/Script/ZeldaDiablo.ZDCombatCameraActor")

room = unreal.EditorLevelLibrary.spawn_actor_from_class(room_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
camera = unreal.EditorLevelLibrary.spawn_actor_from_class(camera_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
directional_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(-400.0, -300.0, 800.0), unreal.Rotator(-55.0, -35.0, 0.0))
sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 300.0), unreal.Rotator(0.0, 0.0, 0.0))

if room:
    room.set_actor_label("PrototypeCombatRoom")

if camera:
    camera.set_actor_label("PrototypeCombatCamera")
    camera.set_editor_property("camera_height", 2200.0)
    camera.set_editor_property("distance_to_target", 850.0)
    camera.set_editor_property("field_of_view", 48.0)
    camera.call_method("UpdateCameraTransform")

if directional_light:
    directional_light.set_actor_label("PrototypeDirectionalLight")
    directional_component = directional_light.get_component_by_class(unreal.DirectionalLightComponent)
    if directional_component:
        directional_component.set_editor_property("intensity", 3.0)

if sky_light:
    sky_light.set_actor_label("PrototypeSkyLight")
    sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
    if sky_component:
        sky_component.set_editor_property("intensity", 0.8)

unreal.EditorLoadingAndSavingUtils.save_current_level()

