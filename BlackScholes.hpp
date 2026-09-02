#pragma once 

#include "Types.hpp"
#include <cmath>
#include <numbers>
#include <algorithm>

[[nodiscard]] inline PortfolioGreeks calculateEuropeanGreeks (OptionType type, double S, double K, double T, double r, double sigma) noexcept {
    PortfolioGreeks greeks{};
    
    if(S <= 0.0 || K <= 0.0 || T <= 0.0 || sigma <= 0.0){   //opção inválida
        return greeks;      
    }

    const double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - (sigma * std::sqrt(T));

    const double pdf_d1 = std::exp(-0.5 * d1 * d1) / std::sqrt(2.0 * std::numbers::pi); //Probabilty Density Function, diz qual a probabilidade de uma variável aleatória assumir um certo valor
    const double cdf_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));  //Cumulative Density Function, diz qual a probabilidade de uma variável alratória se igual ao abaixo de um certo valor
    const double cdf_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2.0)));
    const double cdf_minus_d2 = 1.0 - cdf_d2;


    //cálculo de gamma e vega é idêntico na call e na put
    greeks.gamma = pdf_d1 / (S * sigma * std::sqrt(T));
    greeks.vega = S * pdf_d1 * std::sqrt(T);

    //cálculo de delta e theta varia conforme o tipo de opção
    if (type == OptionType::Call) {
        greeks.delta = cdf_d1;
        greeks.theta = -(S * pdf_d1 * sigma / (2.0 * std::sqrt(T))) - (r * K * std::exp(-r * T) * cdf_d2);
    } else {
        greeks.delta = cdf_d1 - 1.0;
        greeks.theta = -(S * pdf_d1 * sigma / (2.0 * std::sqrt(T))) + r * K * std::exp(-r * T) * cdf_minus_d2;
    }

    return greeks;
}

[[nodiscard]] inline double calculateBlackScholesPrice (OptionType type, double S, double K, double T, double r, double sigma) noexcept {
    if (S <= 0.0 || K <= 0.0 || sigma <= 0.0){      //opção inválida
        return 0.0;
    }

    if (T <= 0.0){   //opção expirada
        //se call, preço = max(ação - strike, 0). Se put, preço = max(strike - ação, 0)
        return (type == OptionType::Call) ? std::max(S - K, 0.0) : std::max(K - S, 0.0);  
    }

    const double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - (sigma * std::sqrt(T));

    const double cdf_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));
    const double cdf_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2.0)));
    const double cdf_minus_d1 = 1.0 - cdf_d1;
    const double cdf_minus_d2 = 1.0 - cdf_d2;

    if (type == OptionType::Call){
        double precoCall = S * cdf_d1 - K * std::exp(-r * T) * cdf_d2;
        return precoCall;
    } else {
        double precoPut = K * std::exp(-r * T) * cdf_minus_d2 - S * cdf_minus_d1;
        return precoPut;
    }

}