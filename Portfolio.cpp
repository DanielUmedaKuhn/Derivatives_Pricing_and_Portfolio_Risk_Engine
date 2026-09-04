#include "Portfolio.hpp"
#include "BlackScholes.hpp"
#include "BinomialTree.hpp"
#include <algorithm>
#include <vector>


PortfolioGreeks Portfolio::calculateTotalGreeks() const noexcept {
    PortfolioGreeks total{};
    total.delta += spotShares;      //contibuição da posição em ações à vista (delta = 1.0 por ação)
            
    //agregação linear das opções na carteira    
    for (const auto& pos : position){       
        if (pos.style == ExerciseStyle::European){
            //cálculo das gregas unitárias (1 contrato/order)
            PortfolioGreeks unitGreeks = calculateEuropeanGreeks(pos.type, pos.S, pos.K, pos.T, pos.r, pos.sigma);

            total.delta += unitGreeks.delta * pos.quantity;
            total.gamma += unitGreeks.gamma * pos.quantity;
            total.vega  += unitGreeks.vega  * pos.quantity;
            total.theta += unitGreeks.theta * pos.quantity;
        
        } else {    //opção American
            CRRResult unitGreeks = calculateCRRPrice(pos.type, pos.style, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma);
            total.delta += unitGreeks.delta * pos.quantity;
            total.gamma += unitGreeks.gamma * pos.quantity;
            total.theta += unitGreeks.theta * pos.quantity;
            total.vega  += unitGreeks.vega  * pos.quantity;
        }
    }
    return total;
}

//spotPctChange é o choque percentual relativo no preço da ação aplicado durante o teste de estresse
//volAbsChange é o choque absoluto em pontos percentuais na volatilidade implícita (sigma)
[[nodiscard]] double Portfolio::calculatePnLStress(double spotPctChange, double volaAbsChange) const noexcept {
    double v0 = 0.0;
    double v1 = 0.0;
    double vOption = 0.0;
    double deltaSpotPnl = 0.0;

    for (const auto& pos : position) {
        double sNovo = pos.S * (1.0 + spotPctChange);
        double sigmaNovo = std::max(0.0001, (pos.sigma + volaAbsChange));
        if (ExerciseStyle::European == pos.style){
            vOption = calculateBlackScholesPrice(pos.type, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma) * pos.quantity;
            v0 += vOption;
            v1 += calculateBlackScholesPrice(pos.type, sNovo, pos.K, pos.T, pos.r, pos.q, sigmaNovo) * pos.quantity;
        } else {
            vOption = calculateCRRPrice(pos.type, pos.style, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma).price * pos.quantity;
            v0 += vOption;
            v1 += calculateCRRPrice(pos.type, pos.style, sNovo, pos.K, pos.T, pos.r, pos.q, sigmaNovo).price * pos.quantity;
        }
    }

    double spotRef = underlyingSpot;
    if (spotRef == 0.0 && !position.empty()){
        spotRef = position[0].S;
    }

    deltaSpotPnl = spotShares * (spotRef* spotPctChange);    //P&L do spot independe de existirem opções ou não

    return (v1 - v0) + deltaSpotPnl;
}

[[nodiscard]] StressMatrixResult Portfolio::generateStressMatrix(const VolatilitySurface& config) const noexcept {
    StressMatrixResult stressMatrix{};
    double spotStep = (config.maxSpot - config.minSpot) / (config.eixoS - 1.0);
    double volaStep = (config.maxVola - config.minVola) / (config.eixoSigma - 1.0);
    stressMatrix.pnlValues.resize(config.eixoS, std::vector<double>(config.eixoSigma));

    for (int i = 0; i < config.eixoS; ++i){
        stressMatrix.spot.push_back(config.minSpot + i * spotStep);     //preenche o eixo spot
    }

    for (int i = 0; i < config.eixoSigma; ++i){
        stressMatrix.vola.push_back(config.minVola + i * volaStep);      //preenche o eixo volatilidade
    }

    for (int i = 0; i < config.eixoS; ++i){
        for (int j = 0; j < config.eixoSigma; ++j){
            stressMatrix.pnlValues[i][j] = calculatePnLStress(stressMatrix.spot[i], stressMatrix.vola[j]);
        }
    }

    return stressMatrix;
}