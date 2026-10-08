#pragma once 

#include "Types.hpp"
#include <cmath>
#include <numbers>
#include <algorithm>

[[nodiscard]] inline PortfolioGreeks calculateEuropeanGreeks (OptionType type, double S, double K, double T, double r, double q, double sigma) noexcept {
    PortfolioGreeks greeks{};
    
    if(S <= 0.0 || K <= 0.0 || T <= 0.0 || sigma <= 0.0){   //opção inválida
        return greeks;      
    }

    const double d1 = (std::log(S / K) + ((r - q) + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - (sigma * std::sqrt(T));

    const double pdf_d1 = std::exp(-0.5 * d1 * d1) / std::sqrt(2.0 * std::numbers::pi); //Probabilty Density Function, diz qual a probabilidade de uma variável aleatória assumir um certo valor
    const double cdf_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));  //Cumulative Density Function, diz qual a probabilidade de uma variável aleatória ser igual ao abaixo de um certo valor
    const double cdf_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2.0)));
    const double cdf_minus_d2 = 1.0 - cdf_d2;

    double exp_qT = std::exp(-q * T);
    double exp_rT = std::exp(-r * T);

    //cálculo de gamma e vega é idêntico na call e na put
    greeks.gamma = (exp_qT * pdf_d1) / (S * sigma * std::sqrt(T));      //sensiblidade do delta da opção à variação no preço da ação
    greeks.vega = (S * exp_qT * pdf_d1 * std::sqrt(T)) / 100.0;     //sensibilidade do preço da opção à volatilidade (em %)

    //cálculo de delta e theta varia conforme o tipo de opção
    if (type == OptionType::Call) {
        greeks.delta = exp_qT * cdf_d1;     //sensibilidade do preço da opção ao preço da ação
        greeks.theta = (-(S * exp_qT * pdf_d1 * sigma / (2.0 * std::sqrt(T))) + (q * S * exp_qT * cdf_d1) - (r * K * exp_rT * cdf_d2)) / 365.0;     //sensibilidade do preço da opção ao tempo (por dia)
        greeks.rho = (K * T * exp_rT * cdf_d2) / 100.0;     //sensibilidade do preço da opção ao risk-free rate (em %)
    } else {
        greeks.delta = exp_qT * (cdf_d1 - 1.0);     //sensibilidade do preço da opção ao preço da ação
        greeks.theta = (-(S * exp_qT * pdf_d1 * sigma / (2.0 * std::sqrt(T))) - (q * S * exp_qT * (1.0 - cdf_d1)) + (r * K * exp_rT * cdf_minus_d2)) / 365.0;   //sensibilidade do preço da opção ao tempo (por dia)
        greeks.rho = (- K * T * exp_rT * cdf_minus_d2) / 100.0;   //sensibilidade do preço da opção ao risk-free rate (em %)
    }

    return greeks;
}

[[nodiscard]] inline double calculateBlackScholesPrice (OptionType type, double S, double K, double T, double r, double q, double sigma) noexcept {
    if (S <= 0.0 || K <= 0.0 || sigma <= 0.0){      //opção inválida
        return 0.0;
    }

    if (T <= 0.0){   //opção expirada
        //se call, preço = max(ação - strike, 0). Se put, preço = max(strike - ação, 0)
        return (type == OptionType::Call) ? std::max(S - K, 0.0) : std::max(K - S, 0.0);  
    }

    const double d1 = (std::log(S / K) + ((r - q) + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - (sigma * std::sqrt(T));

    const double cdf_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));
    const double cdf_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2.0)));
    const double cdf_minus_d1 = 1.0 - cdf_d1;
    const double cdf_minus_d2 = 1.0 - cdf_d2;

    if (type == OptionType::Call){
        double callPrice = (S * std::exp(-q * T)) * cdf_d1 - K * std::exp(-r * T) * cdf_d2;
        return callPrice;
    } else {
        double putPrice = K * std::exp(-r * T) * cdf_minus_d2 - (S * std::exp(-q * T)) * cdf_minus_d1;
        return putPrice;
    }

}