#include "Portfolio.hpp"
#include "BinomialTree.hpp"
#include "BlackScholes.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string readString(const std::string& prompt){
    std::string line; 
    while (true){
        std::cout << prompt;
        std::getline(std::cin, line);
        if (!line.empty()){
            return line;
        }
    }
}

double readDouble (const std::string& prompt){
    std::string line;
    double value;
    while (true){
        std::cout << prompt;
        std::getline(std::cin, line);
        std::stringstream ss (line);
        if (ss >> value){
            return value;
        }
        std::cout << "ERRO - Entrada inválida. Digite um número.\n";
    }
}

char readChar (const std::string& prompt, char valid1, char valid2){
    std::string line;
    while (true){
        std::cout << prompt; 
        std::getline(std::cin, line);
        if(!line.empty()){
            char value = std::toupper(line[0]);
            if (value == valid1 || value == valid2){
                return value;
            }
        }
        std::cout << "ERRO - Entrada inválida. Digite " << valid1 << " ou " << valid2 << ".\n";
    }
}

OptionPosition readPositionFromTerminal(Portfolio& portfolio){
    OptionPosition pos;

    pos.symbol = readString("Símbolo da Opção (Ex: PETR4_C32): ");

    char typeChar = readChar("Tipo (C = Call, P = Put): ", 'C', 'P');
    pos.type = (std::toupper(typeChar) == 'C') ? OptionType::Call : OptionType::Put;

    char styleChar = readChar("Estilo (E = Europeia, A = Americana): ", 'E', 'A');
    pos.style = (std::toupper(styleChar) == 'E') ? ExerciseStyle::European : ExerciseStyle::American;

    pos.S = readDouble("Preço do Ativo (S): ");
    pos.K = readDouble("Strike (K): ");
    pos.T = readDouble("Tempo até Vencimento (T) em anos: ");
    pos.r = readDouble("Risk-Free-Rate (r): ");
    pos.q = readDouble("Taxa de dividendos (q): ");
    pos.sigma = readDouble("Volatilidade Implícita (sigma): ");

    double spotShares = readDouble("Quantidade de ações (Spot) presentes na carteira: ");
    portfolio.setSpotShares(pos.symbol, spotShares);

    pos.quantity = readDouble("Quantidade (+ para Long, - para Short. Ex: +100 = Long 100, -100 = Short 100): ");

    return pos;
}

int main(){
    Portfolio portfolio;

    char addMore = 'S';    
    while (std::toupper(addMore) == 'S'){
        std::cout << "\n --- Adicionar Nova Opção --- \n";
        OptionPosition newPos = readPositionFromTerminal(portfolio);
        portfolio.addPosition(newPos);

        addMore = readChar("Deseja adicionar outra opção? (S/N): ", 'S', 'N');
    }

    // ---> ALTERAÇÃO: Recebe o mapa e itera sobre cada ativo para imprimir as gregas
    std::unordered_map<std::string, PortfolioGreeks> groupedGreeks = portfolio.calculateTotalGreeks();
    
    for (const auto& pair : groupedGreeks) {
        std::cout << "\nGregas Consolidadas [" << pair.first << "]: \n";
        std::cout << std::setw(12) << "Delta" 
                  << std::setw(12) << "Gamma"
                  << std::setw(12) << "Vega" 
                  << std::setw(12) << "Theta" 
                  << std::setw(12) << "Rho\n";

        std::cout << std::fixed << std::setprecision(4)
                  << std::setw(12) << pair.second.delta 
                  << std::setw(12) << pair.second.gamma 
                  << std::setw(12) << pair.second.vega 
                  << std::setw(12) << pair.second.theta 
                  << std::setw(12) << pair.second.rho << "\n\n";
    }

    VolatilitySurface config {
        .eixoS = 10,
        .eixoSigma = 10,
        .minSpot = -0.40,
        .maxSpot = 0.40,
        .minVola = 0.0,
        .maxVola = 0.5
    };

    //itera sobre os ativos individualmente para gerar a stress matrix
    std::unordered_map<std::string, StressMatrixResult> groupedMatrices = portfolio.generateStressMatrix(config);

    for (const auto& pair : groupedMatrices) {
        std::cout << "Heat map de P&L sob estresse (Spot/Volatilidade) [" << pair.first << "]: \n\n";
        std::cout << std::setw(10) << " ";

        const auto& stressMatrix = pair.second;
        
        // eixo superior (volatilidade)
        for (int i = 0; i < stressMatrix.vola.size(); ++i){
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.vola[i] << " ";
        }
        
        std::cout << "\n";

        // eixo lateral (spot) cruzado com volatilidade
        for (int i = 0; i < stressMatrix.spot.size(); ++i){
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.spot[i] << " ";
            for (int j = 0; j < stressMatrix.vola.size(); ++j){
                std::cout << std::fixed << std::setprecision(2) << std::setw(10) << stressMatrix.pnlValues[i][j] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n"; // Espaçamento extra entre matrizes de ativos diferentes
    }

    return 0;
}