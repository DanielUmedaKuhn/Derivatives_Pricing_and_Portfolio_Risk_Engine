#include "Portfolio.hpp"
#include "BinomialTree.hpp"
#include "BlackScholes.hpp"
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

    CRRResult americanPutResult = calculateCRRPrice(
        americanPut.type,
        americanPut.style,
        americanPut.S,
        americanPut.K,
        americanPut.T,
        americanPut.r,
        americanPut.sigma,
        200    //200 passos na árvore para maior precisão
    );

    std::cout << "Preço da Put Americana: " << americanPutResult.price << "\n";
    std::cout << "Delta da Put Americana: " << americanPutResult.delta << "\n";
    std::cout << "Gamma da Put Americana: " << americanPutResult.gamma << "\n";
    std::cout << "Theta da Put Americana: " << americanPutResult.theta << "\n";
    std::cout << "Vega da Put Americana: "  << americanPutResult.vega  << "\n\n";

    VolatilitySurface config {
        .eixoS = 10,
        .eixoSigma = 10,
        .minSpot = -0.40,
        .maxSpot = 0.40,
        .minVola = 0.0,
        .maxVola = 0.5
    };

    StressMatrixResult stressMatrix = portfolio.generateStressMatrix(config);

    for (int i = 0; i < stressMatrix.spot.size(); ++i){
        for (int j = 0; j < stressMatrix.vola.size(); ++j){
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.pnlValues[i][j] << " ";
        }
        std::cout << "\n";
    }

    //greeks via Black-Scholes
    PortfolioGreeks bsGreeks = calculateEuropeanGreeks(
        coveredCall.type, coveredCall.S, coveredCall.K, 
        coveredCall.T, coveredCall.r, coveredCall.sigma
    );

    //greeks via árvore binomial 500 passos
    CRRResult crrResult = calculateCRRPrice(
        coveredCall.type, coveredCall.style, coveredCall.S, coveredCall.K, 
        coveredCall.T, coveredCall.r, coveredCall.sigma, 500
    );

    std::cout << "\n Comparação BS e CRR: \n";
    std::cout << "Delta Black-Scholes: " << bsGreeks.delta << "\n";
    std::cout << "Delta CRR (500 passos): " << crrResult.delta << "\n\n";

    return 0;
}

