#include "pch.h"
#include "PraS.h"
#include <iostream>
#include <sstream>
#include <istream>
#include <thread>
#include <regex>
#include <vector>
#include <ctime>

#define TOS(s) std::to_string(s)
BAKKESMOD_PLUGIN(PraS, "PraS(Private match artistic Stream)", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void PraS::onLoad()
{
	cvarManager->log("init Sock");
	initSocket();
	sendSocket("init");
	createNameTable(true);
	gameWrapper->HookEvent("Function TAGame.ReplayDirector_TA.Tick", std::bind(&PraS::updateScore, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor", std::bind(&PraS::updateAutoCam, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_TA.OnViewTargetChanged", std::bind(&PraS::updatePlayerCam, this, std::placeholders::_1));


	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
	//gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventPlayerScored", std::bind(&PraS::scored, this, std::placeholders::_1));
}

void PraS::onUnload()
{
	endSocket();
	gameWrapper->UnhookEvent("Function TAGame.ReplayDirector_TA.Tick");
	gameWrapper->UnhookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor");
	gameWrapper->UnhookEvent("Function TAGame.Camera_TA.OnViewTargetChanged");

	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventPlayerScored");
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
	sendSocket("scored");
}
void PraS::startGame(std::string eventName) {
	cvarManager->log("Count Down");
	createNameTable();
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	currentFocusActorName = actorName;
}

void PraS::createNameTable(bool isForcedRun)
{
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	ArrayWrapper<CarWrapper> cars = sw.GetCars();
	OwnerMap.clear();
	//only run first or onload
	if (!isForcedRun && sw.GetTotalScore() != 0)return;
	cvarManager->log("PLS:"+TOS(pls.Count()));
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;
		std::string name;
		//name = pl.GetUniqueIdWrapper().GetIdString();
		name = pl.GetOldName().ToString();
		//観戦時のプレイヤー名に合わせるため
		if (pl.GetbBot())name = "Player_Bot_" + pl.GetOldName().ToString();
		else name = "Player_" + name;
		PlayerNames[i] = pl.GetOldName().ToString();
		//DEBUG
		PlayerToDisplayName[name] = std::to_string(i);
	//	PlayerToDisplayName[name] = pl.GetOldName().ToString();
		auto ppl = std::make_shared<PriWrapper>(pl);
		PlayerMap[name] = ppl;
		if (pl.GetTeamNum() != 255) {//not 観戦者
			playerData p = { name, pl.GetTeamNum()};//isblue
			OwnerMap.push_back(p);
		}
		
	}
	//チームでsort
	std::sort(OwnerMap.begin(), OwnerMap.end(), [](const playerData& a, const playerData& b) {return(a.team > b.team); });

	for (int i = 0; i < OwnerMap.size(); i++) {
		auto p = OwnerMap[i];
		OwnerIndexMap[p.name] = i;
		sendSocket("t"+TOS(i)+":"+p.name);
	}
}

void PraS::updateScore(std::string eventName) {
	auto gw = gameWrapper->GetOnlineGame();
	if (gw.IsNull())return;
	auto cars = gw.GetCars();
	for (int i = 0; i < cars.Count(); i++) {
		auto car = cars.Get(i);

		std::string name = "Player_Bot_" + car.GetOwnerName();
		if (car.IsNull())continue;
		auto boostCom = car.GetBoostComponent();
		if (boostCom.IsNull())continue;
		int boost = int(boostCom.GetCurrentBoostAmount()*100);
		if(boost != Boosts[i])sendSocket("b"+ TOS(OwnerIndexMap[name]) + ":" + TOS(boost));
		Boosts[i] = boost;
	}
	if (PlayerMap.count(currentFocusActorName) == 0) {
		return;
	}
	auto pl = PlayerMap[currentFocusActorName];
	currentFocusActorScore = pl->GetMatchScore();
	if (currentFocusActorScore != preFocusActorScore) {
		std::string msg = currentFocusActorName + ":" + std::to_string(currentFocusActorScore);
		sendSocket(PlayerToDisplayName[currentFocusActorName]+":"+std::to_string(currentFocusActorScore));
	}
	preFocusActorScore = currentFocusActorScore;
	//cvarManager->log(currentFocusActorName);
}
void PraS::updatePlayerCam(std::string eventName) {
	//cvarManager->log("fire");
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


