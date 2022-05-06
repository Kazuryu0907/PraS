#include "pch.h"
#include "PraS.h"
#include <iostream>
#include <sstream>
#include <istream>
#include <thread>
#include <regex>
#include <vector>

BAKKESMOD_PLUGIN(PraS, "PraS(Private match artistic Stream)", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void PraS::onLoad()
{
	createNameTable();
	cvarManager->log("init Sock");
	initSocket();
	sendSocket("init");
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", std::bind(&PraS::updateScore, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor", std::bind(&PraS::updateAutoCam, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_TA.OnViewTargetChanged", std::bind(&PraS::updatePlayerCam, this, std::placeholders::_1));
	//Function GameEvent_Soccar_TA.Active.StartRound
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventPlayerScored", std::bind(&PraS::scored, this, std::placeholders::_1));
//Function TAGame.GameEvent_Soccar_TA.EventPlayerScored
	//Function GameFramework.GameThirdPersonCamera.GetFocusActor
	//
	//Function TAGame.Camera_Replay_TA.UpdateCameraState already

	//Function TAGame.CameraState_DirectorPlayerView_TA.FindFocusCar
	//Function TAGame.GFxData_ReplayViewer_TA.SetFocusActorString
	//Function TAGame.CameraState_Director_TA.UpdateSelector
	//Function TAGame.ReplayDirector_TA.EventScoreDataChanged
	
}

void PraS::initSocket() {
	WSADATA wsaData;
	struct sockaddr_in server;
	char buf[32];
	cvarManager->log("initilizing socket...");
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		cvarManager->log("send error");
		return;
	}
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_pton(server.sin_family, ADDR.c_str(), &server.sin_addr.s_addr);
	connect(sock, (struct sockaddr*) & server, sizeof(server));
}

bool PraS::sendSocket(std::string str) {
	bool res = send(sock, str.c_str(), str.length(), 0);
	return res;
}
void PraS::endSocket() {
	closesocket(dst_socket);
	WSACleanup();
}
void PraS::scored(std::string eventName) {
	std::string msg = "scored";
	sendSocket("scored");
}
void PraS::startGame(std::string eventName) {
	createNameTable();
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	currentFocusActorName = actorName;
}

std::string PraS::split(const std::string& s) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, '|')) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	if (elems.size() != 3)return "";
	return elems[1];
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
		//std::string id = split(name);
		if (pl.GetbBot())name = "Player_Bot_" + pl.GetOldName().ToString();
		else name = "Player_" + name;
		PlayerNames[i] = pl.GetOldName().ToString();
		//DEBUG
		//PlayerNames[i] = std::to_string(i);
		//PlayerToDisplayName[name] = std::to_string(i);
		cvarManager->log(pl.GetOldName().ToString());
		PlayerToDisplayName[name] = pl.GetOldName().ToString();
		auto ppl = std::make_shared<PriWrapper>(pl);
		PlayerMap[name] = ppl;
	}
}

void PraS::updateScore(std::string eventName) {
	if (PlayerMap.count(currentFocusActorName) == 0) {
		cvarManager->log(currentFocusActorName);
		cvarManager->log("Name is 0");
		return;
	}
	auto pl = PlayerMap[currentFocusActorName];
	currentFocusActorScore = pl->GetMatchScore();
	if (currentFocusActorScore != preFocusActorScore) {
		std::string msg = currentFocusActorName + ":" + std::to_string(currentFocusActorScore);
		sendSocket(PlayerToDisplayName[currentFocusActorName]+":"+std::to_string(currentFocusActorScore));
		//sendSocket(PlayerNames[currentFocusActorName] + ":" + std::to_string(currentFocusActorScore));
		//sendSocket(std::to_string(currentFocusActorScore));
	}
	preFocusActorScore = currentFocusActorScore;
	//cvarManager->log(currentFocusActorName);
}
void PraS::updatePlayerCam(std::string eventName) {
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	auto cameraState = camera.GetCameraState();
	if (cameraState.find("Car") != std::string::npos) {
		currentFocusActorName = actorName;
		//cvarManager->log(cameraState +actorName);
	}
}

void PraS::updateAutoCam(std::string eventName){
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	auto cameraState = camera.GetCameraState();
	if (cameraState.find("Car") == std::string::npos) {
		//Found
		if (actorName == preAutoCamActorName) {
			//cvarManager->log("double");
			//cvarManager->log("new:"+ camera.GetFocusActor());
		}
		currentFocusActorName = actorName;
		//cvarManager->log(cameraState + actorName);
	}
	preAutoCamActorName = actorName;
}


void PraS::onUnload()
{
	endSocket();
}