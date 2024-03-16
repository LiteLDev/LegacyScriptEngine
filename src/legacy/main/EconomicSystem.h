#pragma once
#include "LLMoney.h"

#include <string>

using std::string;
class EconomySystem {
public:
    static bool init();

    static long long   getMoney(std::string player);
    static bool        setMoney(std::string player, long long money);
    static bool        addMoney(std::string player, long long money);
    static bool        reduceMoney(std::string player, long long money);
    static bool        transMoney(std::string player1, std::string player2, long long money, string const& notes);
    static std::string getMoneyHist(std::string player, int time);
    static void        clearMoneyHist(int time);
};
