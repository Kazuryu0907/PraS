#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unordered_map>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class PraS: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*//*, public BakkesMod::Plugin::PluginWindow*/
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	void createNameTable();
	void updatePlayerCam(std::string);
	void updateAutoCam(std::string);
	void updateScore(std::string);
	void startGame(std::string);
	void scored(std::string);
	void initSocket();
	void endSocket();
	bool sendSocket(std::string);
	std::string split(const std::string& s);
	// Inherited via PluginSettingsWindow
	/*
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
	*/

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "PraS";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
private:
	int PORT = 12345;
	std::string ADDR = "127.0.0.1";
	SOCKET sock;
	struct sockaddr_in server;
	std::unordered_map<std::string,std::shared_ptr<PriWrapper>> PlayerMap;
<<<<<<< Updated upstream
	std::string PlayerNames[10];
	std::unordered_map<std::string, std::string> PlayerToDisplayName;
	bool onAutoCam = false;
	bool onPlayerView = false;
=======
	
	struct playerData {
		std::string name;
		unsigned char team;//isblue
	};
	struct carData {
		std::shared_ptr<CarWrapper> car;
		unsigned char isBot;
	};
	struct resultData{
		std::string name;
		unsigned char team;
		int score;
		int goals;
		int assists;
		int saves;
		int shots;
		int demos;
		int touches;
	};
	std::vector<playerData> OwnerMap;
	std::unordered_map<std::string,int> OwnerIndexMap;
	std::vector<resultData> MatchResults;
	int Boosts[10];
	std::unordered_map<std::string, std::string> DisplayName2Id;
	std::unordered_map<std::string, std::string> UniqueID2DisplayName;
	std::string preActorName = "";
>>>>>>> Stashed changes
	int currentFocusActorScore = 0;
	std::string preAutoCamActorName = "";
	std::string currentFocusActorName = "";
	std::string preFocusActorName = "";
	int preFocusActorScore = 0;
	int dst_socket;
	std::string preMsg = "";
	std::string msg = "";
};


