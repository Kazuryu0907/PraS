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
	isSendSocket = true;
	//reset UI
	sendSocket("end");
	createNameTable(true);
	gameWrapper->HookEvent("Function TAGame.ReplayDirector_TA.Tick", std::bind(&PraS::tick, this, std::placeholders::_1));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&PraS::endGame, this, std::placeholders::_1));
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
	// Flag Down
	isSendSocket = false;
	sendSocket("f0");
	sendSocket("scored");
}
void PraS::startGame(std::string eventName) {
	isBoostWatching = true;
	cvarManager->log("startGame");
	//Flag Up
	isSendSocket = true;
	sendSocket("f1");
	createNameTable();
}
void PraS::endGame(std::string eventName) {
	isBoostWatching = false;
	isSendSocket = false;
	sendSocket("f0");
	sendSocket("end");
	
}

void PraS::sendDummmyData() {
	sendSocket("s : ");
	sendSocket("p : ");
}


void PraS::createNameTable(bool isForcedRun)
{
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	ArrayWrapper<CarWrapper> cars = sw.GetCars();
	for (int i = 0; i < 2; i++) {
		std::string teamName = sw.GetTeams().Get(i).GetSanitizedTeamName().ToString();
		cvarManager->log("T" + TOS(i) + ":" + teamName);
		//blue -> orange
		sendSocket("T"+TOS(i)+":"+teamName);
	}
	
	for (int i = 0; i < 6; i++) {
		sendSocket("b"+TOS(33)+":"+TOS(i));
	}
	
	//only run first or onload
	if (!isForcedRun && sw.GetTotalScore() != 0)return;
	sendSocket("f1");
	cvarManager->log("PLS:"+TOS(pls.Count()));

	//-----clear------//
	PlayerMap.clear();
	OwnerMap.clear();
	DisplayName2Id.clear();
	OwnerIndexMap.clear();
	Id2DisplayName.clear();
	//----------------//
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;
		
		std::string displayName = "";
		//本来はuniqueID
		std::string playerId = TOS(i);
		// if human
		if (!pl.GetbBot())playerId = "Player_" + pl.GetUniqueIdWrapper().GetIdString();


		//観戦時のプレイヤー名に合わせるため
		if (pl.GetbBot())displayName = "Player_Bot_" + pl.GetOldName().ToString();
		else			 displayName = pl.GetPlayerName().ToString();
		cvarManager->log(displayName);
		auto ppl = std::make_shared<PriWrapper>(pl);

		if (pl.GetbBot())PlayerMap[displayName] = ppl;
		else            PlayerMap[playerId] = ppl;
		
		DisplayName2Id[displayName] = playerId;
		Id2DisplayName[playerId] = displayName;
		cvarManager->log(":="+playerId);
		if (pl.GetTeamNum() != 255) {//not 観戦者
			playerData p = { displayName, pl.GetTeamNum()};//isblue
			if(!pl.GetbBot())p = { playerId,pl.GetTeamNum()};//not Bot

			OwnerMap.push_back(p);
		}
		
	}

	std::sort(OwnerMap.begin(), OwnerMap.end(), [](const playerData& a, const playerData& b) {return(a.team > b.team); });


	for (int i = 0; i < OwnerMap.size(); i++) {
		auto p = OwnerMap[i];
		OwnerIndexMap[p.name] = i;
		if(isSendSocket)sendSocket("t" + Id2DisplayName[p.name] + ":" + TOS(i));
	}
	for (auto ow : OwnerIndexMap) {
		cvarManager->log("OW:"+ow.first+":"+TOS(ow.second));
	}
}

void PraS::tick(std::string eventName) {
	//cvarManager->log(isSendSocket ? "true" : "false");
	auto gw = gameWrapper->GetOnlineGame();

	if (gw.IsNull())return;
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	if (actorName != preActorName) {
//		if (actorName == "") {//if flying
//			sendSocket("f0");
//		}
		//空白でもcountに引っかからないはず...
		//send FOCUS
		if (OwnerIndexMap.count(actorName) != 0) {
			//cvarManager->log(Id2DisplayName[actorName]);
			if (isSendSocket)sendSocket("p" + Id2DisplayName[actorName] + ":" + TOS(OwnerIndexMap[actorName]));
			preActorName = actorName;
		}
	}

	//----------boost-------------//
	//isSendSocketをisBootstWatchingの代用として使用
	if (isSendSocket) {
		auto cars = gw.GetCars();
		for (int i = 0; i < cars.Count(); i++) {
			auto car = cars.Get(i);
			//!!!!!!!!!----only noBot--!!!!!!!!//
			std::string name = car.GetOwnerName();//おそらく displayName
			std::string id = DisplayName2Id[name];
			//name = "Player_Bot_" + car.GetOwnerName();
			if (car.IsNull())continue;
			auto boostCom = car.GetBoostComponent();
			if (boostCom.IsNull())continue;
			int boost = int(boostCom.GetCurrentBoostAmount() * 100);
			if (OwnerIndexMap.count(id) == 0)return;
			if (boost != Boosts[i])sendSocket("b" + TOS(boost) + ":" + TOS(OwnerIndexMap[id]));
			Boosts[i] = boost;
		}
	}
	//---------------------------//

	if (PlayerMap.count(actorName) == 0) return;

	auto pl = PlayerMap[actorName];
	currentFocusActorScore = pl->GetMatchScore();
	if (currentFocusActorScore != preFocusActorScore) {
		std::string msg = actorName + ":" + TOS(currentFocusActorScore);
		if (isSendSocket)sendSocket("s" + TOS(OwnerIndexMap[actorName]) + ":" + TOS(currentFocusActorScore));
	}
	preFocusActorScore = currentFocusActorScore;
}