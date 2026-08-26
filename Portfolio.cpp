#include "Portfolio.hpp"
#include "BlackScholes.hpp"
#include <algorithm>


PortfolioGreeks Portfolio::calculateTotalGreeks() const noexcept {
    PortfolioGreeks total{};        //{} após nome da variável na inicialização garante que todos os campos internos da struct sejam zerados seguramente na criação
    total.delta += spotShares;      //contibuição da posição em ações à vista (delta = 1.0 por ação)
            
    //agregação linear das opções na carteira    
    for (const auto& pos : position){       
            //cálculo das gregas unitárias (1 contrato/order)
            PortfolioGreeks unitGreeks =  calculateEuropeanGreeks(pos.type, pos.S, pos.K, pos.T, pos.r, pos.sigma);

            total.delta += unitGreeks.delta * pos.quantity;
            total.gamma += unitGreeks.gamma * pos.quantity;
            total.vega  += unitGreeks.vega  * pos.quantity;
            total.theta += unitGreeks.theta * pos.quantity;
        
        }

    return total;
}

//spotPctChange é o choque percentual relativo no preço da ação aplicado durante o teste de estresse
//volAbsChange é o choque absoluto em pontos percentuais na volatilidade implícita (sigma)
[[nodiscard]] double Portfolio::calculatePnLStress(double spotPctChange, double volAbsChange) const noexcept {
    double v0 = 0.0;
    double v1 = 0.0;
    double vOption = 0.0;
    double deltaSpotPnl = 0.0;

    for (const auto& pos : position) {
        vOption = calculateBlackScholesPrice(pos.type, pos.S, pos.K, pos.T, pos.r, pos.sigma) * pos.quantity;
        v0 += vOption;
        double sNovo = pos.S * (1.0 + spotPctChange);
        double sigmaNovo = std::max(0.0001, (pos.sigma + volAbsChange));
        v1 += calculateBlackScholesPrice(pos.type, sNovo, pos.K, pos.T, pos.r, sigmaNovo) * pos.quantity;
    }

    if (!position.empty()) {
        deltaSpotPnl = spotShares * (position[0].S * spotPctChange);
    }

    return (v1 - v0) + deltaSpotPnl;
}
