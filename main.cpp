#include "Portfolio.hpp"
#include <iostream>

int main(){
    Portfolio portfolio;
    portfolio.setSpotShares(100.0);     //100 ações spot

    OptionPosition coveredCall {
        .symbol = "PETR4_C32",
        .type = OptionType::Call,
        .style = ExerciseStyle::European, 
        .S = 30.0,
        .K = 32.0,
        .T = 30.0 / 365.0,
        .r = 0.10,
        .sigma = 0.30,
        .quantity = -100.0      //venda de 100 calls
    };

    portfolio.addPosition(coveredCall);

    PortfolioGreeks greeks = portfolio.calculateTotalGreeks();
    std::cout << "Delta: " << greeks.delta << "\n";
    std::cout << "Gamma: " << greeks.gamma << "\n";
    std::cout << "Vega: " << greeks.vega << "\n";
    std::cout << "Theta: " << greeks.theta << "\n";

    double pnl = portfolio.calculatePnLStress(-0.10, 0.05);
    std::cout << "P&L sob estresse (Spot -10%, Vol +5%)" << "\n";
    std::cout << "Resultado: R$ " << pnl << '\n';

    return 0;
}