add_executable(critterbits
    main.cpp assetpackresourceloader.cpp boxcollider.cpp engine.cpp
    engineconfiguration.cpp enginecounters.cpp engineeventqueue.cpp
    entity.cpp fileresourceloader.cpp flexrect.cpp fontmanager.cpp
    inputmanager.cpp memory.cpp rectregioncombiner.cpp rendering.cpp
    resourceloader.cpp scene.cpp scenemanager.cpp script.cpp scriptengine.cpp
    scriptsupport.cpp sprite.cpp spritemanager.cpp texturemanager.cpp
    tilemap.cpp tilemapregion.cpp viewport.cpp
    $<TARGET_OBJECTS:duktape> $<TARGET_OBJECTS:critterbits-gui>
    $<TARGET_OBJECTS:critterbits-toml> $<TARGET_OBJECTS:critterbits-anim>)
target_link_libraries(critterbits
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_GFX_LIBRARY} ${SDL2_TTF_LIBRARY}
    ${TMXPARSER_LIBRARIES} ${TINYXML2_LIBRARIES})
if(WIN32)
  target_link_libraries(critterbits wsock32 ws2_32)
endif()
install(TARGETS critterbits RUNTIME DESTINATION ${BIN_DIR})
add_subdirectory(anim)
add_subdirectory(gui)
add_subdirectory(toml)
add_subdirectory(tools/assetpacker)