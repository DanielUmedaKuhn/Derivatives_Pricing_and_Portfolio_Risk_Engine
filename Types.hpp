#pragma once    //garante que o arquvio seja incluído somente uma vez

#include <string>
#include <cstdint>

enum class OptionType : uint8_t {
    Call, 
    Put
};

enum class ExerciseStyle : uint8_t {
    European, 
    American
};

struct OptionPosition { 
    std::string symbol;     //símbolo da ação ex: PETR4
    OptionType type;        //call ou put
    ExerciseStyle style;    //americana ou europeia
    double S;               //preço da ação
    double K;               //strike
    double T;               //tempo até vencimento (em anos)
    double r;               //taxa livre de risco
    double sigma;           //volatilidade implícita
    double quantity;        //quantidade de ações ex: +100 = long, -100 = short
};

struct PortfolioGreeks {
    double delta{0.0};
    double gamma{0.0};
    double vega{0.0};
    double theta{0.0};

    //operador de soma como membro da função
    PortfolioGreeks& operator+=(const PortfolioGreeks& rhs) noexcept {
        delta += rhs.delta;
        gamma += rhs.gamma; 
        vega += rhs.vega;
        theta += rhs.theta;
        
        return *this;
    }
};