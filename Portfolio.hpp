#pragma once

#include "Types.hpp"
#include <vector>

class Portfolio{
    private:
        std::vector<OptionPosition> position;
        double spotShares{0.0};     //quantidade de ações à vista (spot)
    
    public:
        void addPosition(const OptionPosition& pos){
            position.push_back(pos);
        }
        void setSpotShares(double shares) noexcept{
            spotShares = shares;
        }

        [[nodiscard]] PortfolioGreeks calculateTotalGreeks() const noexcept;
        [[nodiscard]] double calculatePnLStress(double spotPctChange, double volAbsChande) const noexcept;        
};