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
	createNameTable();
	cvarManager->log("init Sock");
	initSocket();
	sendSocket("init");
	//gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", std::bind(&PraS::updateScore, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.ReplayDirector_TA.Tick", std::bind(&PraS::updateScore, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor", std::bind(&PraS::updateAutoCam, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Camera_TA.OnViewTargetChanged", std::bind(&PraS::updatePlayerCam, this, std::placeholders::_1));
	//Function GameEvent_Soccar_TA.Active.StartRound
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", std::bind(&PraS::onMemberChanged,this,std::placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventPlayerScored", std::bind(&PraS::scored, this, std::placeholders::_1));
}

void PraS::onUnload()
{
	endSocket();
	gameWrapper->UnhookEvent("Function TAGame.ReplayDirector_TA.Tick");
	gameWrapper->UnhookEvent("Function TAGame.Camera_Replay_TA.SetFocusActor");
	gameWrapper->UnhookEvent("Function TAGame.Camera_TA.OnViewTargetChanged");
	//Function GameEvent_Soccar_TA.Active.StartRound
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.Active.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventPlayerScored");
}

void PraS::scored(std::string eventName) {
	sendSocket("scored");
}

void PraS::onMemberChanged(std::string eventName) {
	//createNameTable();
}
void PraS::startGame(std::string eventName) {
	createNameTable();
	ServerWrapper server = gameWrapper->GetOnlineGame();
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	currentFocusActorName = actorName;
}

void PraS::createNameTable()
{
	//initializing...
	TeamVec.clear();
	CarMapKeys.clear();
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull())return;
	//RUN ONLY FIRST
	//if (sw.GetTotalScore() != 0)return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	auto cars = sw.GetCars();
	//cvarManager->log(TOS(cars.Count()));
	
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;
		std::string name;
		name = pl.GetUniqueIdWrapper().GetIdString();
		//std::string id = split(name);
		if (pl.GetbBot())name = "Player_Bot_" + pl.GetOldName().ToString();
		else name = "Player_" + name;
		PlayerNames[i] = pl.GetOldName().ToString();
		
		teamStruct ts = { name,pl.GetTeamNum() == 0 };//is blue
		TeamVec.push_back(ts);
		//DEBUG
		PlayerToDisplayName[name] = std::to_string(i);
	//	PlayerToDisplayName[name] = pl.GetOldName().ToString();
		auto ppl = std::make_shared<PriWrapper>(pl);
		PlayerMap[name] = ppl;

		try{
			auto car = cars.Get(i);
			if (car.IsNull())continue;
			auto pcr = std::make_shared<CarWrapper>(car);
			CarMap[name] = pcr;
			CarMapKeys.push_back(name);
		}
		catch (...){}

	}
	std::sort(TeamVec.begin(), TeamVec.end(), [](const teamStruct& a, const teamStruct& b) {return (a.team > b.team); });
	for (int i = 0; i < TeamVec.size(); i++) {
		sendSocket("t"+TeamVec[i].name+":"+TOS(i));
	}
}

void PraS::updateScore(std::string eventName) {
	for (int i = 0; i < CarMap.size(); i++) {
		auto name = CarMapKeys[i];
		auto car = CarMap[name];
		if (car->IsNull())continue;
		auto boostCom = car->GetBoostComponent();
		if (boostCom.IsNull())continue;
		try {
			int boost = int(boostCom.GetCurrentBoostAmount() * 100);
			if (boost != Boosts[name]) {
				sendSocket("b" + name + ":" + TOS(boost));
				//cvarManager->log(TOS(i) + ":" + TOS(boost));
			}
			Boosts[name] = boost;
		}catch(...){}
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