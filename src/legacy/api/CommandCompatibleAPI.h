#pragma once
#include "legacy/api/APIHelp.h"

#include <string>
#include <vector>

//////////////////// LLSE Event Callbacks ////////////////////

class Player;
// helper
std::vector<std::string> SplitCmdLine(std::string const& paras);

// 命令回调查询
std::string
LLSEFindCmdReg(bool isPlayerCmd, std::string const& cmd, std::vector<std::string>& receiveParas, bool* fromOtherEngine);
// 删除指定引擎的所有命令
bool LLSERemoveCmdRegister(std::shared_ptr<ScriptEngine> engine);

// 处理命令延迟注册
void ProcessRegCmdQueue();

// 玩家自定义命令注册回调
bool CallPlayerCmdCallback(Player* player, std::string const& cmdPrefix, std::vector<std::string> const& paras);
// 控制台自定义命令注册回调
bool CallServerCmdCallback(std::string const& cmdPrefix, std::vector<std::string> const& paras);
