#include "LLMoney.h"
#include "api/EventAPI.h"
#include "engine/LocalShareData.h"
#include "ll/api/Logger.h"
#include "main/EconomicSystem.h"
#include <string>

ll::Logger economicLogger("EconomicSystem");

////////////// Helper //////////////

bool EconomySystem::init() {
  LLMoney_ListenBeforeEvent(MoneyBeforeEventCallback);
  LLMoney_ListenAfterEvent(MoneyEventCallback);
}

money_t EconomySystem::getMoney(xuid_t player) { return LLMoney_Get(player); }

bool EconomySystem::setMoney(xuid_t player, money_t money) {
  return LLMoney_Set(player, money);
}

bool EconomySystem::addMoney(xuid_t player, money_t money) {
  return LLMoney_Set(player, LLMoney_Get(player) + money);
}

bool EconomySystem::reduceMoney(xuid_t player, money_t money) {
  return LLMoney_Reduce(player, money);
}

bool EconomySystem::transMoney(xuid_t player1, xuid_t player2, money_t money,
                               string const &notes) {
  return LLMoney_Trans(player1, player2, money, notes);
}

std::string EconomySystem::getMoneyHist(xuid_t player, int time) {
  return LLMoney_GetHist(player, time);
}

void EconomySystem::clearMoneyHist(int time) { LLMoney_ClearHist(time); }
