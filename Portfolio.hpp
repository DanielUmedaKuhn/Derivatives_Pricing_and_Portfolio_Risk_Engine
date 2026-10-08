#pragma once

#include "Types.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class Portfolio{
    private:
        std::vector<OptionPosition> position;
        std::unordered_map<std::string, double> spotShares;         //quantidade de ações à vista (spot)
        std::unordered_map<std::string, double> underlyingSpot;     //preço spot de referência para o ativo
    
    public:
        void addPosition(const OptionPosition& pos){
            position.push_back(pos);
            if (underlyingSpot[pos.symbol] == 0.0){   //verifica se o preço spot de referência foi informado
                underlyingSpot[pos.symbol] = pos.S;
            }
        }
        void setSpotShares(std::string symbol, double shares, double currentSpot = 0.0) noexcept{
            spotShares[symbol] = shares;
            if (currentSpot > 0.0){     // = se o preço atual foi informado:
                underlyingSpot[symbol] = currentSpot;
            }
        }

        [[nodiscard]] std::unordered_map<std::string, PortfolioGreeks> calculateTotalGreeks() const noexcept;
        [[nodiscard]] std::unordered_map<std::string, double> calculatePnLStress(double spotPctChange, double volAbsChange) const noexcept;        
        [[nodiscard]] std::unordered_map<std::string, StressMatrixResult> generateStressMatrix(const VolatilitySurface& config) const noexcept;
};