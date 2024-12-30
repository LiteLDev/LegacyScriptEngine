#include "LLMoney.h"
#include "api/EventAPI.h"
#include "ll/api/io/Logger.h"
#include "main/EconomicSystem.h"

#include <string>

////////////// Helper //////////////

bool EconomySystem::init() {
    LLMoney_ListenBeforeEvent(MoneyBeforeEventCallback);
    LLMoney_ListenAfterEvent(MoneyEventCallback);
    return true;
}

long long EconomySystem::getMoney(std::string player) { return LLMoney_Get(player); }

bool EconomySystem::setMoney(std::string player, long long money) { return LLMoney_Set(player, money); }

bool EconomySystem::addMoney(std::string player, long long money) {
    return LLMoney_Set(player, LLMoney_Get(player) + money);
}

bool EconomySystem::reduceMoney(std::string player, long long money) { return LLMoney_Reduce(player, money); }

bool EconomySystem::transMoney(std::string player1, std::string player2, long long money, string const& notes) {
    return LLMoney_Trans(player1, player2, money, notes);
}

std::string EconomySystem::getMoneyHist(std::string player, int time) { return LLMoney_GetHist(player, time); }

void EconomySystem::clearMoneyHist(int time) { LLMoney_ClearHist(time); }
