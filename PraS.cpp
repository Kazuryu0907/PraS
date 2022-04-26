#include "pch.h"
#include "PraS.h"
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

BAKKESMOD_PLUGIN(PraS, "write a plugin description here", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void PraS::onLoad()
{
	gameWrapper->LogToChatbox("chat box");
	cvarManager->log("Plugin loaded!");
	if (!gameWrapper->IsInOnlineGame()) {
		cvarManager->log("you are not in Online");
	}
	else {
		cvarManager->log("you are in Online");
	}
	createNameTable();
	gameWrapper->LogToChatbox("initSock");
	//initSocket();
	sendSocket("send");
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
/*
void PraS::initSocket() {
	gameWrapper->LogToChatbox("initSock");
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		cvarManager->log("Winsock init error");
	}
	char serverIpAddr[] = "127.0.0.1";
	int port = 12345;
	struct sockaddr_in dst_addr;
	memset(&dst_addr,0,sizeof(dst_addr));
	dst_addr.sin_port = htons(port);
	dst_addr.sin_family = AF_INET;
	inet_pton(dst_addr.sin_family,serverIpAddr,&dst_addr.sin_addr.s_addr);
	dst_socket = socket(AF_INET,SOCK_STREAM,0);

	if (connect(dst_socket, (struct sockaddr*) & dst_addr, sizeof(dst_addr))) {
		cvarManager->log("socket error");
		return;
	}
	std::string str = "hello";
	cvarManager->log(str);
	send(dst_socket,str.c_str(),5,0);
}
*/
bool PraS::sendSocket(std::string str) {
	WSADATA wsaData;
	struct sockaddr_in server;
	SOCKET sock;
	char buf[32];
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		cvarManager->log("send error");
		return false;
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_pton(server.sin_family,ADDR.c_str(),&server.sin_addr.s_addr);
	connect(sock, (struct sockaddr*) & server, sizeof(server));
	bool res = send(sock, str.c_str(), str.length(), 0);
	WSACleanup();
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
	gameWrapper->LogToChatbox("Start GAme");
	//createNameTable();
	//CameraWrapper camera = gameWrapper->GetCamera();
	//camera.SetFocusActor(PlayerNames[0]);
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
		//gameWrapper->LogToChatbox(name);
		auto ppl = std::make_shared<PriWrapper>(pl);
		PlayerMap[name] = ppl;
	}
}

void PraS::updateScore(std::string eventName) {
	//cvarManager->log("updateScore:"+ currentFocusActorName);
	if (PlayerMap.count(currentFocusActorName) == 0) {
		cvarManager->log("Name is 0");
		return;
	}
	auto pl = PlayerMap[currentFocusActorName];
	currentFocusActorScore = pl->GetMatchScore();
	if (currentFocusActorScore != preFocusActorScore) {
		std::string msg = currentFocusActorName + ":" + std::to_string(currentFocusActorScore);
		sendSocket(std::to_string(currentFocusActorScore));
	}
	preFocusActorScore = currentFocusActorScore;
	//cvarManager->log(currentFocusActorName+std::to_string(currentFocusActorScore));
}
void PraS::updatePlayerCam(std::string eventName) {
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto replaydirector = server.GetReplayDirector();
	auto actorName = camera.GetFocusActor();
	auto cameraState = camera.GetCameraState();
	if (cameraState.find("Car") != std::string::npos) {
		currentFocusActorName = actorName;
		cvarManager->log(cameraState +actorName);
	}
	if (PlayerMap.count(actorName) == 0)return;
	auto pl = PlayerMap[actorName];
}

void PraS::updateAutoCam(std::string eventName){
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	auto cameraState = camera.GetCameraState();
	if (cameraState.find("Car") == std::string::npos) {
		//Found
		if (actorName == preAutoCamActorName) {
			cvarManager->log("double");
			cvarManager->log("new:"+ camera.GetFocusActor());
		}
		currentFocusActorName = actorName;
		cvarManager->log(cameraState + actorName);
	}
	preAutoCamActorName = actorName;
}


void PraS::onUnload()
{
	endSocket();
}