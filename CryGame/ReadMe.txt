CryGame.dll:
============


possible cleanups:
	* remove IGame::Run()
	* remove IGame::SetTerrainSurface()
	* remove IGame::CreateTagPoint()
	* remove IGame::CreateTagPoint()
	* remove IGame::CreateTagPoint(), GetTagPoint(), RemoveTagPoint()
	* remove IGame::CreateArea()
	* remove IGame::DeleteArea()
	* remove IGame::GetArea()
	* remove weapons from IGame
	* remove equipacks from IGame
	* remove respawn points from IGame
	* remove keycards from XPlayer (not used at all - simple)


AIHandler.cpp
AIHandler.h

* CryGame.cpp CryGame.h
	CXGame implements the IGame interface
	CXGame::Run() run the main loop until another subsystem force the exit
	CXGame::Update() handles the updates per frame 


EntityClassRegistry.cpp
EntityClassRegistry.h
Flock.cpp
Flock.h
FlyBy.cpp
FlyBy.h
Game.cpp
Game.h
GameLoading.cpp
GameMemStats.cpp
GameObject.h
GameShared.h
IngameDialog.cpp
IngameDialog.h
IXSystem.h
LipSync.cpp
LipSync.h
MainMenu.cpp
MainMenu.h
MenuSystem.cpp
MenuSystem.h
NetEntityInfo.cpp
NetEntityInfo.h
PlayerSystem.h
ReadMe.txt						this readme file
ScriptObjectAI.cpp
ScriptObjectAI.h
ScriptObjectAnimation.cpp
ScriptObjectAnimation.h
ScriptObjectBoids.cpp
ScriptObjectBoids.h
ScriptObjectClient.cpp
ScriptObjectClient.h
ScriptObjectEntity.cpp
ScriptObjectEntity.h
ScriptObjectGame.cpp
ScriptObjectGame.h
ScriptObjectGUI.cpp
ScriptObjectGUI.h
ScriptObjectInput.cpp
ScriptObjectInput.h
ScriptObjectLanguage.cpp
ScriptObjectLanguage.h
ScriptObjectMath.cpp
ScriptObjectMath.h
ScriptObjectMovie.cpp
ScriptObjectMovie.h
ScriptObjectParticle.cpp
ScriptObjectParticle.h
ScriptObjectPlayer.cpp
ScriptObjectPlayer.h
ScriptObjectScript.cpp
ScriptObjectScript.h
ScriptObjectServer.cpp
ScriptObjectServer.h
ScriptObjectServerSlot.cpp
ScriptObjectServerSlot.h
ScriptObjectSound.cpp
ScriptObjectSound.h
ScriptObjectSpectator.cpp
ScriptObjectSpectator.h
ScriptObjectStream.cpp
ScriptObjectStream.h
ScriptObjectVector.h
ScriptObjectVehicle.cpp
ScriptObjectVehicle.h
ScriptObjectWeapon.cpp
ScriptObjectWeapon.h
ScriptTimerMgr.cpp
ScriptTimerMgr.h
Spectator.cpp
Spectator.h
StdAfx.cpp
StdAfx.h
StringTableMgr.cpp
StringTableMgr.h
TagPoint.h
UI.cpp
UI.h
UIHud.cpp
UIHud.h
vssver.scc
XArea.cpp
XArea.h
XButton.cpp
XButton.h
XClient.cpp
XClient.h
XClientSnapshot.cpp
XClientSnapshot.h
XControlPage.cpp
XControlPage.h
XDemoMgr.cpp
XDemoMgr.h
XEditBox.cpp
XEditBox.h
XEntityProcessingCmd.cpp
XEntityProcessingCmd.h
XFireMap.cpp
XFireMap.h
XGameStuff.cpp
XGameStuff.h
XGUIControl.cpp
XGUIControl.h
XHud.cpp
XHud.h
XListBox.cpp
XListBox.h
XNetwork.cpp
XNetwork.h
XObjectProxy.cpp
XObjectProxy.h
XPath.cpp
XPath.h
XPathSystem.cpp
XPathSystem.h
XPlayer.cpp
XPlayer.h
XPlayerCamera.cpp
XplayerVehicle.cpp
XPullDownMenu.cpp
XPullDownMenu.h
XPuppetProxy.cpp
XPuppetProxy.h
XServer.cpp
XServer.h
XServerRules.cpp
XServerRules.h
XServerSlot.cpp
XServerSlot.h
XSmthBuffer.cpp
XSmthBuffer.h
XSnapshot.cpp
XSnapshot.h
XStatic.cpp
XStatic.h
XSurfaceMgr.cpp
XSurfaceMgr.h
XSystemBase.cpp
XSystemBase.h
XSystemClient.cpp
XSystemClient.h
XSystemDummy.cpp
XSystemDummy.h
XSystemServer.cpp
XSystemServer.h
XVehicle.cpp
XVehicle.h
XVehicleProxy.cpp
XVehicleProxy.h
XVehicleSystem.cpp
XVehicleSystem.h
XWeapon.cpp
XWeapon.h
XWeaponSystem.cpp
XWeaponSystem.h


