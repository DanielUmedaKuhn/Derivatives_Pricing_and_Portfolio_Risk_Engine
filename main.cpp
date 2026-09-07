#include "Portfolio.hpp"
#include "BinomialTree.hpp"
#include "BlackScholes.hpp"
#include <iostream>
#include <iomanip>

OptionPosition readPositionFromTerminal(){
    OptionPosition pos;
    char typeChar, StyleChar;

    std::cout << "Símbolo da Opção (Ex: PETR4_C32): ";
    std::cin >> pos.symbol;

    std::cout <<"Tipo (C = Call, P = Put): ";
    std::cin >> typeChar;
    pos.type = (std::toupper(typeChar) == 'C') ? OptionType::Call : OptionType::Put;

    std::cout << "Estilo (E = Europeia, A = Americana): ";
    std::cin >> StyleChar;
    pos.style = (std::toupper(StyleChar) == 'E') ? ExerciseStyle::European : ExerciseStyle::American;

    std::cout << "Preço do Ativo (S): ";
    std::cin >> pos.S;
    
    std::cout << "Strike (K): ";
    std::cin >> pos.K;

    std::cout << "Tempo até Vencimento (T) em anos:  ";
    std::cin >> pos.T;

    std::cout << "Risk-Free-Rate (r): ";
    std::cin >> pos.r;

    std::cout << "Taxa de dividendos (q): ";
    std::cin >> pos.q;

    std::cout << "Volatilidade Implícita (sigma): ";
    std::cin >> pos.sigma;

    std::cout << "Quantidade (+ para Long, - para Short. Ex: +100 = Long 100, -100 = Short 100): ";
    std::cin >> pos.quantity;

    return pos;
}

int main(){
    Portfolio portfolio;
    double spotShares = 0.0;

    std::cout << "Quantidade de ações (Spot) presentes na carteira atualmente: ";
    std::cin >> spotShares;
    portfolio.setSpotShares(spotShares);

    char addMore = 'S';
    while (std::toupper(addMore) == 'S'){
        std::cout << "\n --- Adicionar Nova Opção --- \n";
        OptionPosition newPos = readPositionFromTerminal();
        portfolio.addPosition(newPos);

        std::cout << "Deseja adcionar outra opção? (S/N): ";
        std::cin >> addMore;
    }

    //relatório das gregas consolidadadas da carteira
    PortfolioGreeks greeks = portfolio.calculateTotalGreeks();
    
    std::cout << "\nGregas Consolidadas: \n";
    std::cout << std::setw(12) << "Delta" 
              << std::setw(12) << "Gamma"
              << std::setw(12) << "Vega" 
              << std::setw(12) << "Theta" 
              << std::setw(12) << "Rho\n";

    std::cout << std::fixed << std::setprecision(4)
              << std::setw(12) << greeks.delta 
              << std::setw(12) << greeks.gamma 
              << std::setw(12) << greeks.vega 
              << std::setw(12) << greeks.theta 
              << std::setw(12) << greeks.rho << "\n\n";

    //configuração e geração da Stress Matrix
    VolatilitySurface config {
        .eixoS = 10,
        .eixoSigma = 10,
        .minSpot = -0.40,
        .maxSpot = 0.40,
        .minVola = 0.0,
        .maxVola = 0.5
    };

    StressMatrixResult stressMatrix = portfolio.generateStressMatrix(config);

    std::cout << "Heat map de P&L sob estresse (Spot/Volatilidade): \n\n";
    std::cout << std::setw(10) << " ";

    //eixo superior (volatilidade)
    for (int i = 0; i < stressMatrix.vola.size(); ++i){
        std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.vola[i] << " ";
    }
    
    std::cout << "\n";

    //eixo lateral (spot) cruzado com volatilidade
    for (int i = 0; i < stressMatrix.spot.size(); ++i){
        std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.spot[i] << " ";
        for (int j = 0; j < stressMatrix.vola.size(); ++j){
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.pnlValues[i][j] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}

