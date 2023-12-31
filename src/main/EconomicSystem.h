#pragma once
#include <string>
using std::string;

typedef long long money_t;
typedef std::string xuid_t;

class EconomySystem {
public:
  static bool init();

  static money_t getMoney(xuid_t player);
  static bool setMoney(xuid_t player, money_t money);
  static bool addMoney(xuid_t player, money_t money);
  static bool reduceMoney(xuid_t player, money_t money);
  static bool transMoney(xuid_t player1, xuid_t player2, money_t money,
                         string const &notes);
  static std::string getMoneyHist(xuid_t player, int time);
  static void clearMoneyHist(int time);
};