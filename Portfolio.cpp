#include "Portfolio.hpp"
#include "BlackScholes.hpp"
#include "BinomialTree.hpp"
#include <algorithm>
#include <vector>

std::unordered_map<std::string, PortfolioGreeks> Portfolio::calculateTotalGreeks() const noexcept {
    std::unordered_map<std::string, PortfolioGreeks> totals;
            
    for (const auto& pos : position){       
        if (pos.style == ExerciseStyle::European){
            PortfolioGreeks unitGreeks = calculateEuropeanGreeks(pos.type, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma);

            totals[pos.symbol].delta += unitGreeks.delta * pos.quantity;
            totals[pos.symbol].gamma += unitGreeks.gamma * pos.quantity;
            totals[pos.symbol].vega  += unitGreeks.vega  * pos.quantity;
            totals[pos.symbol].theta += unitGreeks.theta * pos.quantity;
            totals[pos.symbol].rho   += unitGreeks.rho   * pos.quantity;
        
        } else {    
            CRRResult unitGreeks = calculateCRRPrice(pos.type, pos.style, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma);
            totals[pos.symbol].delta += unitGreeks.delta * pos.quantity;
            totals[pos.symbol].gamma += unitGreeks.gamma * pos.quantity;
            totals[pos.symbol].theta += unitGreeks.theta * pos.quantity;
            totals[pos.symbol].vega  += unitGreeks.vega  * pos.quantity;
            totals[pos.symbol].rho   += unitGreeks.rho   * pos.quantity;
        }
    }
    
    for(const auto& par : spotShares){
        totals[par.first].delta += par.second;     //para ações spot, delta = quantidade de ações spot
    }
    return totals;
}

[[nodiscard]] std::unordered_map<std::string, double> Portfolio::calculatePnLStress(double spotPctChange, double volaAbsChange) const noexcept {
    std::unordered_map<std::string, double> v0_map;
    std::unordered_map<std::string, double> v1_map;
    std::unordered_map<std::string, double> deltaSpotPnl;
    std::unordered_map<std::string, double> result;

    for (const auto& pos : position) {
        double sNovo = pos.S * (1.0 + spotPctChange);
        double sigmaNovo = std::max(0.0001, (pos.sigma + volaAbsChange));
        double vOption0 = 0.0;
        double vOption1 = 0.0;

        if (ExerciseStyle::European == pos.style){
            vOption0 = calculateBlackScholesPrice(pos.type, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma) * pos.quantity;
            vOption1 = calculateBlackScholesPrice(pos.type, sNovo, pos.K, pos.T, pos.r, pos.q, sigmaNovo) * pos.quantity;
        } else {
            vOption0 = calculateCRRPrice(pos.type, pos.style, pos.S, pos.K, pos.T, pos.r, pos.q, pos.sigma).price * pos.quantity;
            vOption1 = calculateCRRPrice(pos.type, pos.style, sNovo, pos.K, pos.T, pos.r, pos.q, sigmaNovo).price * pos.quantity;
        }
        v0_map[pos.symbol] += vOption0;
        v1_map[pos.symbol] += vOption1;
    }

    for(const auto& par : spotShares){
        double spotRef = 0.0;
        if (underlyingSpot.count(par.first)) {
            spotRef = underlyingSpot.at(par.first);
        }
        deltaSpotPnl[par.first] += par.second * (spotRef * spotPctChange);    
    }
    
    for (const auto& pair : v1_map) {
        result[pair.first] = (pair.second - v0_map[pair.first]) + deltaSpotPnl[pair.first];
    }
    
    for (const auto& pair : deltaSpotPnl) {
        if (result.find(pair.first) == result.end()) {
            result[pair.first] = pair.second;
        }
    }

    return result;
}

[[nodiscard]] std::unordered_map<std::string, StressMatrixResult> Portfolio::generateStressMatrix(const VolatilitySurface& config) const noexcept {
    std::unordered_map<std::string, StressMatrixResult> matrices;
    double spotStep = (config.maxSpot - config.minSpot) / (config.eixoS - 1.0);
    double volaStep = (config.maxVola - config.minVola) / (config.eixoSigma - 1.0);
    
    std::vector<std::string> symbols;
    for (const auto& pos : position) {
        if (std::find(symbols.begin(), symbols.end(), pos.symbol) == symbols.end()) {
            symbols.push_back(pos.symbol);
        }
    }
    for (const auto& par : spotShares) {
        if (std::find(symbols.begin(), symbols.end(), par.first) == symbols.end()) {
            symbols.push_back(par.first);
        }
    }

    for (const auto& sym : symbols) {
        matrices[sym].pnlValues.resize(config.eixoS, std::vector<double>(config.eixoSigma));
        for (int i = 0; i < config.eixoS; ++i){
            matrices[sym].spot.push_back(config.minSpot + i * spotStep);     
        }
        for (int i = 0; i < config.eixoSigma; ++i){
            matrices[sym].vola.push_back(config.minVola + i * volaStep);      
        }
    }

    for (int i = 0; i < config.eixoS; ++i){
        for (int j = 0; j < config.eixoSigma; ++j){
            double spotShift = config.minSpot + i * spotStep;
            double volaShift = config.minVola + j * volaStep;
            auto pnlMap = calculatePnLStress(spotShift, volaShift);
            
            for (const auto& sym : symbols) {
                matrices[sym].pnlValues[i][j] = pnlMap[sym];
            }
        }
    }

    return matrices;
}