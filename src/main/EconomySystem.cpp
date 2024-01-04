#include "api/EventAPI.h"
#include "engine/LocalShareData.h"
#include "ll/api/Logger.h"
#include "main/EconomicSystem.h"
#include <string>

ll::Logger economicLogger("EconomicSystem");

////////////// Helper //////////////

money_t EconomySystem::getMoney(xuid_t player) {
  return Money::getMoney(player);
}

bool EconomySystem::setMoney(xuid_t player, money_t money) {
  return Money::setMoney(player, money);
}

bool EconomySystem::addMoney(xuid_t player, money_t money) {
  return Money::setMoney(player, Money::getMoney(player) + money);
}

bool EconomySystem::reduceMoney(xuid_t player, money_t money) {
  return Money::reduceMoney(player, money);
}

bool EconomySystem::transMoney(xuid_t player1, xuid_t player2, money_t money,
                               string const &notes) {
  return Money::createTrans(player1, player2, money, notes);
}

std::string EconomySystem::getMoneyHist(xuid_t player, int time) {
  return Money::getTransHist(player, time);
}

void EconomySystem::clearMoneyHist(int time) { Money::purgeHist(time); }