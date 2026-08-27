#include "Portfolio.hpp"
#include "BinomialTree.hpp"
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
        .quantity = -100.0      //short 100 calls
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

    OptionPosition americanPut {
        .symbol = "VALE3_P60",
        .type = OptionType::Put,
        .style = ExerciseStyle::American,
        .S = 62.0,
        .K = 60.0,
        .T = 60.0 / 365.0,
        .r = 0.10,
        .sigma = 0.35,
        .quantity = 100.0       //long 100 puts
    };

    double precoCRR = calculateCRRPrice(
        americanPut.type,
        americanPut.style,
        americanPut.S,
        americanPut.K,
        americanPut.T,
        americanPut.r,
        americanPut.sigma,
        200    //200 passos na árvore para maior precisão
    );

    std::cout << "\nPreço da Put Americana (CRR): R$ " << precoCRR << "\n";

    return 0;
}