
#ifndef ILIPSYNC_H
#define ILIPSYNC_H

struct CryCharMorphParams;

// callback interfaces
struct IDialogLoadSink
{
	virtual void OnDialogLoaded(struct ILipSync *pLipSync)=0;
	virtual void OnDialogFailed(struct ILipSync *pLipSync)=0;
};

struct ILipSync
{
	virtual bool Init(ISystem *pSystem, IEntity *pEntity)=0;											// initializes and prepares the character for lip-synching
	virtual void Release()=0;																											// releases all resources and deletes itself
	virtual bool LoadRandomExpressions(const char *pszExprScript, bool bRaiseError=true)=0;	// load expressions from script
	virtual bool UnloadRandomExpressions()=0;																			// release expressions
	// loads a dialog for later playback
	virtual bool LoadDialog(const char *pszFilename, int nSoundVolume, float fMinSoundRadius, float fMaxSoundRadius, float fClipDist, int nSoundFlags=0,IScriptObject *pAITable=NULL)=0;														
	virtual bool UnloadDialog()=0;																								// releases all resources
	virtual bool PlayDialog(bool bUnloadWhenDone=true)=0;													// plays a loaded dialog
	virtual bool StopDialog()=0;																									// stops (aborts) a dialog
	virtual bool DoExpression(const char *pszMorphTarget, CryCharMorphParams &MorphParams, bool bAnim=true)=0;	// do a specific expression
	virtual bool StopExpression(const char *pszMorphTarget)=0;										// stop animating the specified expression
	virtual bool Update(bool bAnimate=true)=0;																		// updates animation & stuff
	virtual void SetCallbackSink(IDialogLoadSink *pSink)=0;												// set callback sink (see above)
};

#endif // ILIPSYNC_H