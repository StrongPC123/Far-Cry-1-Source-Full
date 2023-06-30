Usage of multiple body parts feature.

Changes.

THere are a few changes to ICryAnimation. First, there is now LoadModel() function in
CryCharManager. This will let one load a skin/submesh/bodypart without having to 
actually create a character.

The old workflow is not disturbed by the changes, they're completely backward-compatible.
By the new concept, each character can have several parts, called submeshes. THey're
enumerated through GetSubmesh()/NumSubmeshes() methods in ICryCharInstance interface
To add/remove a submesh, you can use AddSubmesh()/RemoveSubmesh() functions.

Usage and Workflow.

First of all, you'll need to create several pieces of the same character. It can be several
objects bound to the same skeleton with different instances of Physique. Maksym Artisov
can provide the details of how to do that. After this is done, all objects can be batch-exported
into individual cgf files. The objects are independent character from now on, but they have
an important trait: they have the same skeleton structure. Since the character consisting
of multiple body parts has only ONE skeleton, all pieces it consists of must be bound to the
same skeleton. It is not allowed to export a body part with only part of the skeleton, the
whole skeleton must be exported starting with the same root as all the other body parts.

Programmatically, you'll have to first create some basic character. This character must be
a full character with full skeleton (like all the rest) and (unlike the rest body parts)
it must have the complete set of animations. The model of this character will define which
animations will be available on the character. This character contains initially one submesh,
with index 0 and this submesh may not be deleted.

After this, you can load other body parts with LoadModel function. Please use smart pointers
to keep the loaded models, unless you want to keep the model in memory in case of error
(like impossibility to attach the model to the character as a submesh due to incompatible
skeletons). Then you can attach body parts to the character with AddSubmesh() function.
You must not detach submeshes after the character has been drawn until the next frame, because
during drawing character may leave callback pointers to submeshes in Renderer, and these
callback pointers will be used during the end of the frame at the skinning phase.

Each submesh has its own set of morph targets, as you can see from descriptions of member
functions in ICryCharSubmesh interface. But submeshes do not have individual animations.
You can get a pointer to ICryCharModel from ICryCharSubmesh, and then get information about
animations that this model contains. But you may not start individual animations on a submesh
because submesh doesn't have its own skeleton. You may only start animations that are registered
in the main model (the one from which the initial character was created). If you start morphs
on the whole character, they're started in each submesh where they were found. You can however
start the morphs in each ICryCharSubmesh individually.

Submeshes work like attachments in many ways. For example, they're pretty much independent.
Each submesh uses its own LOD set, vertex buffers and material array. Each submesh has
its own model and can be turned off/on very fast (see SetVisibility()/IsVisible() in
ICryCharSubmesh). It may appear to be not cheap to use many small pieces in each character,
but it depends on the situation. In case it is a bottleneck, one possible optimization is
a special functionality that welds several pieces together. It's pretty heavy thing to do
so it must be done once on startup or after the character has been composed by the user.
After that, depending on the circumstances, rendering and skinning speed may improve.

The sample code that attaches a body part to an existing character and returns true if 
the operation was successful:

	ICryCharModel* pModel = m_pAnimationSystem->LoadModel(model);
	if (pModel)
	{
		ICryCharSubmesh* pSubmesh = m_character->AddSubmesh (pModel, true);
		return pSubmesh != NULL;
	}
	
after this, you can access pSubmesh to start a morph or make it visible/invisible.
to remove the submesh:

	m_character->RemoveSubmesh(pSubmesh);


In the future, perhaps it would be beneficial to make functionality to match different skeletons
by bone names so that different body parts may be exported only with parts of the initial
skeleton (e.g. a hand can be exported with the skeleton starting with shoulder joint as it
doesn't have any influences above that joint).

CryCharSubmesh is not collected into memory sizer object. This is a feature to do.

