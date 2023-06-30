// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE 
// This is an inline file: it is meant to be included in multiple places
// to generate code out of the PROFILER macro.
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE 

// This is the list of per-frame profilers used in the renderer system

// per-frame profilers: collect the infromation for each frame for 
// displaying statistics at the beginning of each frame
PROFILER(Screen_Begin, "0Screen:Begin"); // 
PROFILER(Screen_Update, "0Screen:Update"); // 
PROFILER(State_Sorting, "0State:Sorting of RI"); // 
PROFILER(State_SortingDist, "0State:Dist Sorting of RI"); // 
PROFILER(State_LightStates, "0State:VLights"); // 
PROFILER(State_RStates, "0State:RenderStates"); // 
PROFILER(Draw_ObjSprites, "0Draw:ObjSprites"); // 
PROFILER(Objects_Changes, "0O.Objects:Change"); // 
PROFILER(Objects_ChangesSkinning, "1O.Objects:Skinning"); // 
PROFILER(Objects_ObjInvTransform, "1O.Objects:InvTransform"); // 
PROFILER(Texture_Changes, "0T0.Textures:Set"); // 
PROFILER(Texture_ChangesUpload, "1T0.Textures:Stream (Immediat)"); // 
PROFILER(Texture_Precache, "0Textures:PreloadPredict"); // 
PROFILER(Texture_LoadFromCache, "0T1.Textures:LoadFromCache"); // 
PROFILER(Texture_LoadFromCacheSync, "1T1.Textures:LoadFromCacheSync"); // 
PROFILER(Texture_LoadFromCacheASync, "1T1.Textures:LoadFromCacheASync"); // 
PROFILER(Texture_LoadFromCache_UploadSync, "1T1.Textures:UploadSync"); // 
PROFILER(Texture_AsyncUpload, "0Textures:AsyncUpload"); // 
PROFILER(Shader_PShaders, "0PS.Shader:PShaders"); // 
PROFILER(Shader_PShaderActivate, "1PS.Shader:PShaderActivate"); // 
PROFILER(Shader_PShadersParms, "1PS.Shader:PShaderParms"); // 
PROFILER(Shader_VShaders, "0VS.Shader:VShaders"); // 
PROFILER(Shader_VShaderActivate, "1VS.Shader:VShaderActivate"); // 
PROFILER(Shader_VShadersParms, "1VS.Shader:VShaderParms"); // 
PROFILER(Shader_VShadersMatr, "1VS.Shader:VShaderMatrices"); // 
PROFILER(Mesh_CheckUpdate, "0CU.Mesh:CheckUpdate"); // 
PROFILER(Mesh_CheckUpdateCreateGBuf, "1CU.Mesh:CreateGeneralBuf"); // 
PROFILER(Mesh_CheckUpdateUpdateGBuf, "1CU.Mesh:UpdateGeneralBuf"); // 
PROFILER(Mesh_CheckUpdateCreateSysTang, "1CU.Mesh:CreateSystemTangBuf"); // 
PROFILER(Mesh_CheckUpdateCreateTBuf, "1CU.Mesh:CreateTangentBuf"); // 
PROFILER(Mesh_CheckUpdateUpdateTBuf, "1CU.Mesh:UpdateTangentBuf"); // 
PROFILER(Mesh_CheckUpdateCreateLMBuf, "1CU.Mesh:CreateLMBuf"); // 
PROFILER(Mesh_CheckUpdateUpdateLMBuf, "1CU.Mesh:UpdateLMBuf"); // 
PROFILER(Mesh_CheckUpdateUpdateInds, "1CU.Mesh:UpdateInds"); // 
PROFILER(Mesh_CheckUpdateCallback, "1CU.Mesh:Callback"); // 
PROFILER(Mesh_CheckUpdateRecreateSystem, "1CU.Mesh:CreateSystemBuffer"); // 
PROFILER(Mesh_CheckUpdateSkinning, "1CU.Anim:Skinning"); // 
PROFILER(Mesh_CreateVBuffers, "0Mesh:CreateVBuffer"); // 
PROFILER(Mesh_CreateIBuffers, "0Mesh:CreateIBuffer"); // 
PROFILER(Mesh_UpdateVBuffers, "0V.Mesh:UpdateVBuffer"); // 
PROFILER(Mesh_UpdateVBuffersLock, "1V.Mesh:UpdateVBufferLock"); // 
PROFILER(Mesh_UpdateVBuffersCopy, "1V.Mesh:UpdateVBufferCopy"); // 
PROFILER(Mesh_UpdateVBuffersDynMerge, "1V.Mesh:UpdateVBufferDynMerge"); // 
PROFILER(Mesh_UpdateIBuffers, "0I.Mesh:UpdateIBuffer"); // 
PROFILER(Mesh_UpdateIBuffersLock, "1I.Mesh:UpdateIBufferLock"); // 
PROFILER(Mesh_UpdateIBuffersCopy, "1I.Mesh:UpdateIBufferCopy"); // 
PROFILER(Mesh_UpdateIBuffersDynMerge, "1I.Mesh:UpdateIBufferDynMerge"); // 
PROFILER(Mesh_REPrepare, "0PRRE.Mesh:Preparing of RE"); // 
PROFILER(Mesh_REPrepare_FlushOcleaf, "1PRRE.Mesh:Flush REOcleaf"); // 
PROFILER(Mesh_REPrepare_Ocleaf, "1PRRE.Mesh:Prepare REOcleaf"); // 
PROFILER(Mesh_REPrepare_Flush3DPoly, "1PRRE.Mesh:Flush 3DPoly"); // 
PROFILER(Mesh_REPrepare_3DPoly, "1PRRE.Mesh:Prepare 3DPoly"); // 
PROFILER(Draw_Predraw, "0Draw:Predraw"); // 
PROFILER(Draw_IndexMesh, "0Draw:IndexedMesh"); // 
PROFILER(Draw_EFIndexMesh, "0Draw:ShaderIndexedMesh"); // 
PROFILER(Draw_2DImage, "0Draw:2DImage"); // 
PROFILER(Prep_PrepareDepthMap, "0Prep:PrepareDepthMap");


