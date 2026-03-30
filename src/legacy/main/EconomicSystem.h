#pragma once
#include <string>

using std::string;
class EconomySystem {
public:
    static bool init();

    static long long getMoney(std::string const& player);
    static bool      setMoney(std::string const& player, long long money);
    static bool      addMoney(std::string const& player, long long money);
    static bool      reduceMoney(std::string const& player, long long money);
    static bool
    transMoney(std::string const& player1, std::string const& player2, long long money, string const& notes);
    static std::string getMoneyHist(std::string const& player, int time);
    static void        clearMoneyHist(int time);
};
