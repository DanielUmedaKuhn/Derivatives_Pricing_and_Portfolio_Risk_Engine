#pragma once

#include "Types.hpp"
#include <vector>
#include <cmath>
#include <algorithm>


[[nodiscard]] inline CRRResult calculateCRRPrice(OptionType type, ExerciseStyle style, double S, double K, double T, double r, double q, double sigma, std::size_t steps = 100) noexcept {
    if (S <= 0.0 || K <= 0.0 || sigma <= 0.0) {
        return {0.0, 0.0, 0.0};
    }

    if (T <= 0.0) {
        return {(type == OptionType::Call) ? std::max(S - K, 0.0) : std::max(K - S, 0.0), 0.0, 0.0};
    }

    double dT = T / steps;
    double u = std::exp(sigma * std::sqrt(dT));
    double d = 1.0 / u;
    double p = (std::exp((r - q) * dT) - d) / (u - d);    //probabilidade neutra ao risco
    double discount = std::exp(-r * dT);

    std::vector<double> values(steps + 1);
    double fu = 0.0;
    double fd = 0.0;
    
    double fuu = 0.0;
    double fud = 0.0;
    double fdd = 0.0;

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

        if (i == 2){    //para cálculo de gamma, precisa-se do valor da opção em três nós: fuu, fud e fdd, que será no passo 2 da árvore (antes do passo 1)
            fdd = values[0];
            fud = values[1];
            fuu = values[2];
        }

        if (i == 1){    //para cálculo de delta, precisa-se do valor da opção em dois nos: fu e fd, que será no passo 1 da árvore (logo antes do passo atual)
            fd = values[0];
            fu = values[1];
        }
    }

    double Su = S * u;
    double Sd = S * d;
    double delta = (fu - fd) / (Su - Sd);

    double deltaUp = (fuu - fud) / (S * u * u - S);
    double deltaDown = (fud - fdd) / (S - S * d * d);
    double gamma = (deltaUp - deltaDown) / (Su - Sd);
    double theta = (fud - values[0])/(2 * dT);

    //início da árvore de volatilidade
    double dSigma = 0.01;
    double sigmaBumped = sigma + dSigma;

    double uBumped = std::exp(sigmaBumped * std::sqrt(dT));
    double dBumped = 1.0 / uBumped;
    double pBumped = (std::exp((r - q) * dT) - dBumped) / (uBumped - dBumped);

    std::vector<double> valuesBumped(steps + 1);

    //loop com valores de volatilidade atualizada
    for (std::size_t i = 0; i<= steps; ++i){
        double ST = S * std::pow(uBumped, i) * std::pow(dBumped, steps - i);      
        valuesBumped[i] = (type == OptionType::Call ? std::max(ST - K, 0.0) : std::max(K - ST, 0.0));
    }

    //loop de backward induction com volatilidade atualizada
    for (std::size_t i = steps; i-- > 0; ) {    
        for (std::size_t j = 0; j <= i; ++j){   
            double continuationValue = discount * (pBumped * valuesBumped[j + 1] + (1.0 - pBumped) * valuesBumped[j]);   
            if (style == ExerciseStyle::American) {
                double St = S * std::pow(uBumped, j) * std::pow(dBumped, i - j);   
                double instrinsicValue = (type == OptionType::Call ? std::max(St - K, 0.0) : std::max(K - St, 0.0));
                valuesBumped[j] = std::max(continuationValue, instrinsicValue);
            } else {
                valuesBumped[j] = continuationValue;
            }
        }
    }

    double vega = (valuesBumped[0] - values[0]) / dSigma;   //sensibilidade do preço da opção à volatilidade  

    return {values[0], delta, gamma, theta, vega};       //prêmio da opção hoje (t = 0) estará no primeiro elemento
}