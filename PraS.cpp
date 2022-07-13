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


	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
	//gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", [this](std::string eventName) {cvarManager->log("counting"); });
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&PraS::endGame, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&PraS::startGame, this, std::placeholders::_1));
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
	createNameTable();
}
void PraS::endGame(std::string eventName) {
	sendSocket("end");
}

void PraS::createNameTable(bool isForcedRun)
{
	

	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	
	//only run first or onload
	if (!isForcedRun && sw.GetTotalScore() != 0)return;
	cvarManager->log("PLS:"+TOS(pls.Count()));

	DisplayNameMap.clear();
	PlayerIdMap.clear();
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;


		std::string displayName = "";
		
		//本来はuniqueID
		std::string playerId = TOS(i);

		//観戦時のプレイヤー名に合わせるため
		if (pl.GetbBot())displayName = "Player_Bot_" + pl.GetOldName().ToString();
		else			 displayName = "Player_" + pl.GetPlayerName().ToString();
		
		auto ppl = std::make_shared<PriWrapper>(pl);
		DisplayNameMap[displayName] = ppl;
		PlayerIdMap[displayName] = playerId;
		
	}
}

void PraS::updateScore(std::string eventName) {
	auto gw = gameWrapper->GetOnlineGame();
	if (gw.IsNull())return;
	CameraWrapper camera = gameWrapper->GetCamera();
	auto actorName = camera.GetFocusActor();
	
	if (preActorName != actorName) {
		if(PlayerIdMap.count(actorName) == 0)return;
		sendSocket("p" + actorName + ":" + PlayerIdMap[actorName]);
		preActorName = actorName;
	}

	if (DisplayNameMap.count(actorName) == 0)return;

	std::shared_ptr<PriWrapper> pl = DisplayNameMap[actorName];
	int score = pl->GetMatchScore();
	if(preScore != score){
		sendSocket("s" + TOS(score));
		preScore = score;
	}
	
	
}

