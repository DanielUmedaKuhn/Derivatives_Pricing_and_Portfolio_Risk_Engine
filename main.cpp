#include "Portfolio.hpp"
#include "BinomialTree.hpp"
#include <iostream>
#include <iomanip>

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

    portfolio.addPosition(americanPut);

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

    std::cout << "\nPreço da Put Americana (CRR): R$ " << precoCRR << "\n\n";

    VolatilitySurface config {
        .eixoS = 10,
        .eixoSigma = 10,
        .minSpot = -0.40,
        .maxSpot = 0.40,
        .minVol = 0.0,
        .maxVol = 0.5
    };

    StressMatrixResult stressMatrix = portfolio.generateStressMatrix(config);

    for (int i = 0; i < stressMatrix.spot.size(); ++i){
        for (int j = 0; j < stressMatrix.vola.size(); ++j){
            //setprecision: 2 casas decimais
            //setw: 10 casa para cada número
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.pnlValues[i][j] << " ";    //setprecision: 2 casas decimais
        }
        std::cout << "\n";
    }

    return 0;
}