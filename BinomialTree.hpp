#pragma once

#include "Types.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

[[nodiscard]] inline double calculateCRRPrice(OptionType type, ExerciseStyle style, double S, double K, double T, double r, double sigma, std::size_t steps = 100) noexcept {
    if (S <= 0.0 || K <= 0.0 || sigma <= 0.0) {
        return 0.0;
    }

    if (T <= 0.0) {
        return (type == OptionType::Call) ? std::max(S - K, 0.0) : std::max(K - S, 0.0);
    }

    double dT = T / steps;
    double u = std::exp(sigma * std::sqrt(dT));
    double d = 1.0 / u;
    double p = (std::exp(r * dT) - d) / (u - d);    //probabilidade neutra ao risco
    double discount = std::exp(-r * dT);

    std::vector<double> values(steps + 1);

    for (std::size_t i = 0; i<= steps; ++i){
        double ST = S * std::pow(u, i) * std::pow(d, steps - i);       //preço da ação no vencimento
        values[i] = (type == OptionType::Call ? std::max(ST - K, 0.0) : std::max(K - ST, 0.0));
    }

    for (std::size_t i = steps; i-- > 0; ) {    //Backward Induction, calcula o valor da continuação descontado
        for (std::size_t j = 0; j <= i; ++j){   
            //valor de continuação esperado (descontado), values[j+1] representa movimento de alta e values[j] representa movimento de baixa
            double continuationValue = discount * (p * values[j + 1] + (1.0 - p) * values[j]);   
            if (style == ExerciseStyle::American) {
                double St = S * std::pow(u, j) * std::pow(d, i - j);   //preço da ação no nó corrente (passo i, subida j)
                double instrinsicValue = (type == OptionType::Call ? std::max(St - K, 0.0) : std::max(K - St, 0.0));
                //opção americana: max entre continuar e exercer antecipadamente
                values[j] = std::max(continuationValue, instrinsicValue);
            } else {
                //opção europeia: apenas o valor de continuação
                values[j] = continuationValue;
            }
        }
    }
    

    return values[0];       //prêmio da opção hoje (t = 0) estará no primeiro elemento
}