#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <winsock2.h>
#include <ws2tcpip.h>

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
	std::map<std::string,std::shared_ptr<PriWrapper>> PlayerMap;
	std::string PlayerNames[10];
	bool onAutoCam = false;
	bool onPlayerView = false;
	int currentFocusActorScore = 0;
	std::string preAutoCamActorName = "";
	std::string currentFocusActorName = "";
	int preFocusActorScore = 0;
	int dst_socket;
	std::string preMsg = "";
	std::string msg = "";
};


