//
// This is an include file for the cvars.cpp and cvars.h
// It's used to facilitate declaration of the console variables used in the animation subsystem
// so that any variable is required to be declared only once
// This file is included by the header to make the list of members of CVars structure
// and by cvars.cpp to initialize the members.
//
// NOTE: there's going to be a huge list of those with the time being.
// and this is used in 4 places. So this actually makes it easier to maintain and avoid bugs.


// the multiplier for the decal sizes
DECLARE_FLOAT_VARIABLE(ca_DecalSizeMultiplier, 1,"The multiplier for the decal sizes");

// This is an integer. If set to N != 0, then every Nth frame the normalization for the characters will be switched
// on and off. When on, the binormals, tangents and normals are normalized during the skinning paths;
// when off , then they are not. By default, the normalization is turned off (by Vladimir) because
// there are next to no visible artifacts and it gathers some performance.
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NormalizeBases, 0, "If set to N != 0, then every Nth frame the normalization for the characters will be switched on and off");

DECLARE_INT_VARIABLE(r_ShowTangents, 0);
DECLARE_INT_VARIABLE(r_ShowNormals, 0);

DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoDeform, 0, "the skinning is not performed during rendering if this is 1");

DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoMorph, 0, "the morph skinning step is skipped (it's part of overall skinning during rendering)");

DECLARE_INT_VARIABLE_IMMEDIATE(ca_MorphNormals, 0, "this variable enables changing of normals during morphing. Required for the reflection effect when only vertex animation is used");

// the animation isn't updated (the characters remain in the same pose)
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoAnim, 0, "the animation isn't updated (the characters remain in the same pose)");
// the physics is not applied (effectively, no IK)
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoPhys, 0, "the physics is not applied (effectively, no IK)");

// If this is set to non-0, the Plus matrix changes that are more than some threshold are all logged
DECLARE_INT_VARIABLE_IMMEDIATE(ca_MatPlusDebug, 0, "If this is set to non-0, the Plus matrix changes that are more than some threshold are all logged");


// the additional multiplier that MIGHT be used in a special (debug) build to make characters be animated slower/faster
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed, 1, "the additional multiplier that MIGHT be used in a special (debug) build to make characters be animated slower/faster");

// these are the additional multipliers that are used for layers
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed0, 1, "The additional speed multiplier that are used for layer 0 (primary)");
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed1, 1, "The additional speed multiplier that are used for layer 1");
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed2, 1, "The additional speed multiplier that are used for layer 2");
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed3, 1, "The additional speed multiplier that are used for layer 3");
DECLARE_FLOAT_VARIABLE(ca_UpdateSpeed4, 1, "The additional speed multiplier that are used for layer 4");

DECLARE_INT_VARIABLE_IMMEDIATE(ca_StopDeformingAfter, 0, "Debug variable: stops deforming characters after the given number of frames");
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoMatPlus, 0, "Debug variable: doesn't take into account the external 'Plus' matrix supplied by the Game");


#ifdef _DEBUG

// change in the character world position are displayed if this is non-zero
DECLARE_INT_VARIABLE_IMMEDIATE(ca_TrackWorldPos, 0, "Debug variable: change in the character world position are displayed if this is non-zero");

// If you set the DeformRange to non-empty range, then the vertices in that interval won't be 
// skinned for all animatable characters
// Used only in _DEBUG builds
// the interval is [Begin,End)
DECLARE_INT_VARIABLE_IMMEDIATE(ca_CollapseRangeBegin, 0, "Debug variable: If you set the DeformRange to non-empty range, then the vertices in that interval won't be skinned for all animatable characters");
DECLARE_INT_VARIABLE_IMMEDIATE(ca_CollapseRangeEnd, 0, "Debug variable: If you set the DeformRange to non-empty range, then the vertices in that interval won't be skinned for all animatable characters");

// This is a string variable. If you set it to non-empty string, those characters whose CID file paths
// contain this string as a substring, won't update the world position and rotation.
// This can be used to debug 1st person weapons
DECLARE_STRING_VARIABLE(ca_Freeze, "");

// if this is not empty string, then character instances whose model file paths contain this substring,
// will print their position each frame when skinned into a special file (Animation.log)
DECLARE_STRING_VARIABLE (ca_LogPosition, "");


#endif

// if this is not empty string, the animations of characters with the given model will be logged
DECLARE_STRING_VARIABLE (ca_LogAnimation, "");

// this is generic variable for debugging the heat light sources. Don't use it (or use on your own own)
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DebugHeatSources, 0, "This is generic variable for debugging the heat light sources. Don't use it (or use on your own risk)");

// flag set:
// 0 - effectively does nothing (enables all coordinate transforms)
// 1 - disables Z rotation
// 2 - disables Y rotation
// 4 - disables X rotation
// can be used ORed: 1+4 disables X and Z rotations
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DisablePlusRotationMask, 0, "flag set:\n0 - effectively does nothing (enables all coordinate transforms)\n1 - disables Z rotation\n2 - disables Y rotation\n4 - disables X rotation\ncan be used ORed: 1+4 disables X and Z rotations");

// if set to 0, effectively disables creation of decals on characters
// 2 - alternative method of calculating/building the decals
DECLARE_INT_VARIABLE_IMMEDIATE_DUMP(ca_EnableDecals, 1, "if set to 0, effectively disables creation of decals on characters\n2 - alternative method of calculating/building the decals");

DECLARE_INT_VARIABLE_IMMEDIATE(ca_PerforatingDecals, 1, "if set to 1, all decals will be penetrating, i.e. the bullets will pierce through the character. 0 - decal will be placed only around the bullet income position; this can yield incorrect results with gutter [tangential] wounds");

// Multi-purpose variable
// Enables:
//  OnAnimationEvent (verbosity level 5) messages
DECLARE_INT_VARIABLE_IMMEDIATE(ca_Debug, 0, "Debug varialbe: enables some additional debug logging");

// Enables misc. timers
DECLARE_INT_VARIABLE_IMMEDIATE(ca_Profile, 0, "Enables a table of real-time profile statistics");

// if set to 1, the decals use the default texture
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DefaultDecalTexture, 0, "if set to 1, the decals use the default texture");

// if set to 1, the own bounding box of the character is drawn
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DrawBBox, 0, "if set to 1, the own bounding box of the character is drawn");

// if set to 1, the light helpers are drawn
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DrawLHelpers, 0, "if set to 1, the light helpers are drawn");

// if set to 1, the skeleton is drawn
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DrawBones, 0, "if set to 1, the skeleton is drawn");

// if you set this to 0, there won't be any frequest warnings from the animation system
// if you set this to more than default, then you'll got some additional info from the animation system.
// LIST of warnings/infos:
//   0 : None
//   1 : Warning: CryModelState::RunAnimation: Animation \"%s\" Not Found for character \"%s\"
//       Error: CryModelState::SetAnimationEvent: Animation not found: \"%s\", frameId:%d, userdata:%d, charInst:0x%08X
//   2 : Warning: CryModelState::SetAnimationEvent(anim %s, frame %d, user data 0x%X): animation event will never be called because the frame is out of the animation range
//   3 : CryModelState::SetAnimationSinkInstance ok(\"%s\",0x%08X): done
//
DECLARE_INT_VARIABLE_IMMEDIATE(ca_AnimWarningLevel, nDefaultAnimWarningLevel, "if you set this to 0, there won't be any\nfrequest warnings from the animation system");

// If this is not 0, then the OnAnimationEvent methods are not called upon animation events
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DisableAnimEvents, 0, "If this is not 0, then the OnAnimationEvent methods are not called upon animation events");

// If set to 1, will prevent models from unloading from memory upon destruction of the last referencing character
DECLARE_INT_VARIABLE_IMMEDIATE(ca_KeepModels, 0, "If set to 1, will prevent models from unloading from memory\nupon destruction of the last referencing character");

// if this is 1, will draw the decal outline in wireframe
DECLARE_INT_VARIABLE_IMMEDIATE(ca_DrawDecalOutline, 0, "if this is 1, will draw the decal outline in wireframe");

// if this is 1, will not draw the characters
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoDraw, 0, "if this is 1, will not draw the characters");

// if this is 1, will not draw the bound objects
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoDrawBound, 0, "if this is 1, will not draw the bound objects");

// if this is 1, shadow volumes won't draw
DECLARE_INT_VARIABLE_IMMEDIATE(ca_NoDrawShadowVolumes, 0, "if this is 1, shadow volumes won't draw");

// multiplier value used to calculate the LOD for characters
// Increase this to decrease the LOD distances
DECLARE_FLOAT_VARIABLE(ca_LodBias, 0.125f, "Multiplier value used to calculate the LOD for characters.\nIncrease this to decreasethe LOD distances");

// 1 - skin algorithm using relative to def matrices (not not effective)
// 2 - new optimized algorithm using global bone matrices
//DECLARE_INT_VARIABLE_IMMEDIATE(ca_SkinAlgorithm, 2);

// if this is not 0, then all animations will be started in the specified layer,
// useful for testing
DECLARE_INT_VARIABLE_IMMEDIATE(ca_OverrideLayer, 0, "if this is not 0, then all animations will be started in the specified layer,\nuseful for testing only");

// if this is not an empty string, then the animations that contain this substring
// will effectively stop the current animation instead of starting themselves;
// useful for testing in the Previewer
//DECLARE_STRING_VARIABLE(ca_StopAnimName, "");

// this is the threshold of LOD minimal vertex weight that will be truncated
// the vertex bindings with the given weight and less will be truncated
// Effectively, the value >0.5 sets rigid linking, because there is never more than 1 weight > 0.5
DECLARE_FLOAT_VARIABLE(ca_MinVertexWeightLOD0, 0.07f,"this is the threshold of LOD minimal vertex weight that will be truncated\nthe vertex bindings with the given weight and less will be truncated\nEffectively, the value >0.5 sets rigid linking, because there is never more than 1 weight > 0.5");
DECLARE_FLOAT_VARIABLE(ca_MinVertexWeightLOD1, 0.3f,"this is the threshold of LOD minimal vertex weight that will be truncated\nthe vertex bindings with the given weight and less will be truncated\nEffectively, the value >0.5 sets rigid linking, because there is never more than 1 weight > 0.5");
DECLARE_FLOAT_VARIABLE(ca_MinVertexWeightLOD2, 0.5f,"this is the threshold of LOD minimal vertex weight that will be truncated\nthe vertex bindings with the given weight and less will be truncated\nEffectively, the value >0.5 sets rigid linking, because there is never more than 1 weight > 0.5");

// this parameter determines how many vertices can be used in one tangent basis skin
// This is used for strip-mining optimization recommended by the Pentium4 Optimization Manual:
// only part of the skin is processed at a moment.
DECLARE_INT_VARIABLE_IMMEDIATE(ca_VertsPerTangSubskin, 3000, "this parameter determines how many vertices can be used in one tangent basis skin\nThis is used for strip-mining optimization recommended by the Pentium4 Optimization Manual:\nonly part of the skin is processed at a moment.");

// if this is 0, then the data that was used during construction of instances
// of CryGeometryInfo and all its clients will be retained in memory for debugging
// purposes.
DECLARE_INT_VARIABLE_IMMEDIATE (ca_ZDeleteConstructionData, 1, "if this is 0, then the data that was used during construction of instances\nof CryGeometryInfo and all its clients will be retained in memory for debugging purposes");

// if this is 1, then SSE is used for tangent space calculations and perhaps other things
DECLARE_INT_VARIABLE_IMMEDIATE (ca_SSEEnable, 1, "If this is 1, then SSE is used for tangent space calculations and perhaps other things (like skinning)");

/*
DECLARE_INT_VARIABLE_IMMEDIATE (ca_SSEEnable1, 0);
DECLARE_INT_VARIABLE_IMMEDIATE (ca_SSEEnable2, 0);
DECLARE_INT_VARIABLE_IMMEDIATE (ca_SSEEnable3, 0);
*/
// this is the number of repeats of skinning for testing
DECLARE_INT_VARIABLE_IMMEDIATE (ca_TestSkinningRepeats,1, "this is the number of repeats of skinning for testing");

// this is the type of stripification to use on new objects. Set 0 not to stripify
DECLARE_INT_VARIABLE_IMMEDIATE (ca_StripifyGeometry, 4, "this is the type of stripification to use on new objects. Set 0 not to stripify");

// the period, in frames, when the tangents will be recalculated for the character.
// 1 means ever frame, 2 means every 2nd frame and so on
DECLARE_INT_VARIABLE_IMMEDIATE (ca_TangentBasisCalcPeriod, 1, "the period, in frames, when the tangents will be recalculated for the character. 1 means ever frame, 2 means every 2nd frame and so on");

// 0 - the same shadow vertex buffer is used for every frame shadow volume rendering
// 1 - the shadow vertex buffers are switched
DECLARE_INT_VARIABLE_IMMEDIATE (ca_ShadowDoubleBuffer, 1, "0 - the same shadow vertex buffer is used for every frame shadow volume rendering\n1 - the shadow vertex buffers are switched");

// If 0, then the character's shadow volume isn't rendered (but the attached objects' volumes are rendered)
DECLARE_INT_VARIABLE_IMMEDIATE (ca_EnableCharacterShadowVolume, 1, "If 0, then the character's shadow volume isn't rendered (but the attached objects' volumes are rendered)");

// if this is not 0, then it will always replace the shadow volume extent
DECLARE_FLOAT_VARIABLE (ca_ShadowVolumeExtent, 0, "if this is not 0, then it will always replace the shadow volume extent");

#ifdef _DEBUG
// if this is non-0, in debug mode there's no assertion when something is detached from a non-existing bone
// DECLARE_INT_VARIABLE_IMMEDIATE (ca_NoAttachAssert, 0);
DECLARE_INT_VARIABLE_IMMEDIATE (ca_DebugRebuildShadowVolumes, 0, "if this is non-0, in debug mode there's no assertion when something is detached from a non-existing bone");
DECLARE_INT_VARIABLE_IMMEDIATE (ca_DebugGetParentWQuat, 0, "if this is non-0, in debug mode there's no assertion when something is detached from a non-existing bone");
#endif

// This is the maximum number of shadow buffers per instance
// With double buffering, there must be 2 * max number of light sources
DECLARE_INT_VARIABLE_IMMEDIATE (ca_ShadowBufferLimit, 6, "This is the maximum number of shadow buffers per instance.\nWith double buffering, there must be 2 * max number of light sources");

// enables a special kind of log: Animation.log file, solely for debugging
DECLARE_INT_VARIABLE_IMMEDIATE (ca_EnableAnimationLog, 0, "enables a special kind of log: Animation.log file, solely for debugging");

// this is the "power" with which the rain drops the character:
// with infinity, only the vertex that's the most parallel to the rain will react to the rain;
// with 0, random point (including those not facing the rain flow) will be chosen.
DECLARE_INT_VARIABLE_IMMEDIATE (ca_RainPower, 5, "this is the \"power\" with which the rain drops the character:\nwith infinity, only the vertex that's the most parallel to the rain will react to the rain;\nwith 0, random point (including those not facing the rain flow) will be chosen.");

// if this is not 0, then no material sorting is performed
DECLARE_INT_VARIABLE_IMMEDIATE (ca_NoMtlSorting, 1, "if this is not 0, then no material sorting is performed");

// this is the maximum LOD to use for shadow volumes; if it's 0, it effectively does nothing
// (all the range of LODs is available for shadow builder), if it's 1, then objects will use
// LOD 1 shadows for LOD 0 characters if available .  Etc.
DECLARE_INT_VARIABLE_IMMEDIATE (ca_LimitShadowLOD, 0, "this is the maximum LOD to use for shadow volumes; if it's 0, it effectively does nothing\n(all the range of LODs is available for shadow builder), if it's 1, then objects will use\nLOD 1 shadows for LOD 0 characters if available .  Etc.");

// this is for testing: all characters are immediately multiplied by this scale factor
DECLARE_FLOAT_VARIABLE (ca_RuntimeScale, 1, "Debug variable: all characters are immediately multiplied by this scale factor");

// if this is true, then the blending between layers is performed by 
// cubic (spline) function, not linear. This should look more natural
DECLARE_INT_VARIABLE_IMMEDIATE(ca_EnableCubicBlending, 1, "if this is true, then the blending between layers is performed by \ncubic (spline) function, not linear. This should look more natural");

// If this is 1, CCG searching and loading is enabled
DECLARE_INT_VARIABLE_IMMEDIATE(ca_EnableCCG, 1, "If this is 1, CCG searching and loading is enabled");

// if set to 1, during loading of the model, the vertex colors will be loaded and they will
// be updated in the vertex buffer
DECLARE_INT_VARIABLE_IMMEDIATE(ca_EnableVertexColors, 0, "if set to 1, during loading of the model, the vertex colors will be loaded and they will be updated in the vertex buffer");

// if set to 1, the lights (heat and light sources, dynamic and bound during CGF load) will be updated
// upon each Draw() call
DECLARE_INT_VARIABLE_IMMEDIATE(ca_EnableLightUpdate, 1, "if set to 1, the lights (heat and light sources, dynamic and bound during CGF load) will be updated upon each Draw() call");

// If not 0, then the unused animations get unloaded after the specified number of frames being not used
DECLARE_INT_VARIABLE_IMMEDIATE(ca_AnimationUnloadDelay, nDefaultAnimUnloadDelay, "If not 0, then the unused animations get unloaded after the specified number of frames being not used");

// if this is non-0, the animation are not loaded immediately upon request (when loading CGF), but
// rather on first request. This feature should not be used with ca_AnimationUnloadDelay, because loading
// all animations first and then automatically unloading them doesn't have much sense.
DECLARE_INT_VARIABLE_IMMEDIATE(ca_AnimationDeferredLoad,nDefaultAnimDeferredLoad,"if this is non-0, the animation are not loaded immediately upon request (when loading CGF), but\nrather on first request. This feature should not be used with ca_AnimationUnloadDelay, because loading\nall animations first and then automatically unloading them doesn't have much sense.");

// if this is 0, the original tangents will be used rather than the tangent skin to update the tangets during rendering
DECLARE_INT_VARIABLE_IMMEDIATE (ca_EnableTangentSkinning, 1, "if this is 0, the original tangents will be used rather than the tangent skin to update the tangets during rendering");

// This (binary) mask is applied to the frame id to determine if the calculation of character BBox based on the skin vertex coordinates is enabled
DECLARE_INT_VARIABLE_IMMEDIATE (ca_SkinBasedBBoxMask, 0xFFFFFFFF, "This (binary) mask is applied to the frame id to determine if the calculation of character BBox based on the skin vertex coordinates is enabled");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_SafeReskin, 0, "If this is enabled, the character is reskinned every 2^6 frames even if it's doesn't seem to be required");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_PrebuildShadowConnectivity, 0, "If this is enabled, the shadow connectivity is built upon load");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_TestMirror, 0, "The lowest 3 bits determine whether the characters will be mirrored in the corresponding axis");

DECLARE_FLOAT_VARIABLE (ca_DrawAnims, 0, "Draws the debug information on the character in realtime");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_TickVersion, 1, "Sets the version of blending routine Tick(). Dev-time-only variable");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_RestartBehaviour, 0, "0 - restarts the same animation if half of animation has been played; 1 - doesn't restart it at all");

/*! Controls the presence of an additional directional light, which brings out the bumps on objects in shadows.
	A radius of 0 disables the light.
	Initial value = 0 */
DECLARE_FLOAT_VARIABLE_DUMP(ca_ambient_light_range, 0, "Controls the presence of an additional directional light, which brings out the bumps on objects in shadows.\nA radius of 0 disables the light.\nInitial value = 0");
/*! Controls the intensity of the additional directional light. Right now, we only allow for different gray levels.
	Initial value = 0.2f */
DECLARE_FLOAT_VARIABLE_DUMP(ca_ambient_light_intensity, 0.2f, "Controls the intensity of the additional directional light. Right now, we only allow for different gray levels.\nInitial value = 0.2f");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_DebugShaders, 0, "1 - draws a label under each character, saying which shaders it uses");

DECLARE_FLOAT_VARIABLE (ca_DeathBlendTime, 0.3, "Specifies the blending time between low-detail dead body skeleton and current skeleton");

DECLARE_INT_VARIABLE_IMMEDIATE (ca_DecalAntiflickerHack, 1, "Enable this to draw decals only during light or fog pass - can be used to reduce decal flickering on characters");

DECLARE_FLOAT_VARIABLE(ca_BoundZOffset, 0.0015f, "This is the relative offset of the bound objects with the corresponding flag set. It's a hack to avoid hemlets from penetrating the head when looking at a character from far away");