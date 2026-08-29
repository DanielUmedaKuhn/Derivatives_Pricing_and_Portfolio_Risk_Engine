#pragma once

#include "Types.hpp"
#include <vector>

class Portfolio{
    private:
        std::vector<OptionPosition> position;
        double spotShares{0.0};         //quantidade de ações à vista (spot)
        double underlyingSpot{0.0};     //preço spot de referência para o ativo
    
    public:
        void addPosition(const OptionPosition& pos){
            position.push_back(pos);
            if (underlyingSpot == 0.0){   //verifica se o preço spot de referência foi informado
                underlyingSpot = pos.S;
            }
        }
        void setSpotShares(double shares, double currentSpot = 0.0) noexcept{
            spotShares = shares;
            if (currentSpot > 0.0){     // = se o preço atual foi informado:
                underlyingSpot = currentSpot;
            }
        }

        [[nodiscard]] PortfolioGreeks calculateTotalGreeks() const noexcept;
        [[nodiscard]] double calculatePnLStress(double spotPctChange, double volAbsChange) const noexcept;        
        [[nodiscard]] StressMatrixResult generateStressMatrix(const VolatilitySurface& config) const noexcept;
};