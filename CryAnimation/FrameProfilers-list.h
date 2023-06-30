// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE 
// This is an inline file: it is meant to be included in multiple places
// to generate code out of the PROFILER macro.
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE 

// This is the list of per-frame profilers used in the char animation system

// per-frame profilers: collect the infromation for each frame for 
// displaying statistics at the beginning of each frame
PROFILER(CharacterUpdate_Total, "TOTAL Up"); // total character update
PROFILER(CharacterUpdate, "Up"); // total character update
PROFILER(AllCallbacks, "?:Callbacks"); // all callbacks the animation system makes
PROFILER(BoneUpdate, "Up:Bones"); // bone matrices update from the animations
PROFILER(AnimationUpdate, "Up:Anim"); // animation effector update, including all callback times

PROFILER(ModelDeform_Total, "TOTAL Deform");  // normal skinning, including decal realization
PROFILER(ModelDeform, "Deform");  // normal skinning, including decal realization
PROFILER(ModelDeformVideoMemCopy, "Deform:Video:Copy"); // copying to videomemory and updating buffers
PROFILER(PureSkin, "?:skin"); // the pure skinning

PROFILER(ModelShadow_Total, "TOTAL Shadow");
PROFILER(ModelShadowDeform, "Shadow:Deform"); // skinning/extruding for shadow volume, 
PROFILER(ModelShadowVideoAlloc, "Shadow:Video:Alloc"); // skinning for shadow volume
PROFILER(ModelShadowVideoUpdate, "Shadow:Video:Copy"); // skinning for shadow volume
PROFILER(ModelShadowBoundObjects, "Shadow:BndObj"); // skinning for shadow volume
PROFILER(BBoxUpdate, "?:BBox"); // all calls to BBox
PROFILER(CharacterPhysicsUpdate, "PhUp"); // physics update on ICryChar level


/*
PROFILER(Temp1,"Temp1");
PROFILER(Temp2,"Temp2");
PROFILER(Temp3,"Temp3");

PROFILER(Reserved1,"Reserved1");
PROFILER(Reserved2,"Reserved2");
PROFILER(Reserved3,"Reserved3");
*/