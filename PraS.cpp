#include "pch.h"
#include "PraS.h"
#include <iostream>
#include <sstream>

BAKKESMOD_PLUGIN(PraS, "write a plugin description here", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void PraS::onLoad()
{
	gameWrapper->LogToChatbox("chat box");
	cvarManager->log("Plugin loaded!");
	if (!gameWrapper->IsInOnlineGame()) {
		cvarManager->log("you are not in Online");
		return;
	}
	else {
		cvarManager->log("you are in Online");
	}
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	const ViewTarget &v = camera.GetViewTarget();
	createNameTable();
	cvarManager->log(camera.GetFocusActor());
	gameWrapper->HookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor", [this](std::string eventName) {
		onAutoCam = true;
		//autoCam
		updateCamera(eventName);
		onAutoCam = false;
		});
	//Function GameFramework.GameThirdPersonCamera.GetFocusActor
	//
	//Function TAGame.Camera_Replay_TA.UpdateCameraState already
	gameWrapper->HookEvent("Function TAGame.Camera_TA.OnViewTargetChanged", [this](std::string eventName) {
		onPlayerView = true;
		//cvarManager->log("FindFocus");
		updateCamera(eventName);
		onPlayerView = false;
	});
	//Function TAGame.CameraState_DirectorPlayerView_TA.FindFocusCar
	//Function TAGame.GFxData_ReplayViewer_TA.SetFocusActorString
	//Function TAGame.CameraState_Director_TA.UpdateSelector
	//Function TAGame.ReplayDirector_TA.EventScoreDataChanged
	
	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&PraS::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&PraS::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	cvarManager->log("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&PraS::YourPluginMethod, this);
}

void PraS::createNameTable()
{
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;
		std::string name;
		name = pl.GetUniqueIdWrapper().GetIdString();
		if (pl.GetbBot())name = "Player_Bot_"+pl.GetOldName().ToString();
		else {
			std::stringstream ss;
			for (const auto& item : name) {
				ss << std::hex << int(item);
			}
			cvarManager->log(ss.str());
		}
		PlayerNames[i] = name;
		cvarManager->log(name);
		gameWrapper->LogToChatbox(name);
		auto ppl = std::make_shared<PriWrapper>(pl);
		PlayerMap[name] = ppl;
	}
}

void PraS::updateCamera(std::string eventName) {
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto replaydirector = server.GetReplayDirector();
	auto actorName = camera.GetFocusActor();
	auto cameraState = camera.GetCameraState();
	if (cameraState != "CameraState_Car_TA") {

	}
	auto view = camera.GetViewTarget();
	auto cont = gameWrapper->GetPlayerController();
	auto contpri = cont.GetFollowTarget();
	//gameWrapper->LogToChatbox(actorName);
	if (onAutoCam && onPlayerView) gameWrapper->LogToChatbox("DOUBLE!");
	ActorWrapper focusCar = replaydirector.GetFocusCar();
	std::stringstream ss;
	for (const auto& item : actorName) {
		ss << std::hex << int(item);
	}
	cvarManager->log(ss.str());
	//gameWrapper->LogToChatbox(std::to_string(PlayerMap.count(actorName)));
	gameWrapper->LogToChatbox(eventName);
	if (PlayerMap.count(actorName) == 0)return;
	auto pl = PlayerMap[actorName];
	gameWrapper->LogToChatbox(actorName);
	//gameWrapper->LogToChatbox(std::to_string(pl->GetMatchScore()));
}
void PraS::onUnload()
{
}