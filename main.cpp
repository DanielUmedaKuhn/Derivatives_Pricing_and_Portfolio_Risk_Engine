#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <numbers>
#include <iostream>

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
};

[[nodiscard]] inline PortfolioGreeks calcularGregasEuropeias (OptionType type, double S, double K, double T, double r, double sigma) noexcept {
    PortfolioGreeks greeks{};
    
    if(S <= 0.0 || K <= 0.0 || T <= 0.0 || sigma <= 0.0){
        return greeks;
    }

    const double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - (sigma * std::sqrt(T));

    const double pdf_d1 = std::exp(-0.5 * d1 * d1) / std::sqrt(2.0 * std::numbers::pi);
    const double cdf_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));
    const double cdf_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2)));

    //cálculo de gamma e vega é idêntico na call e na put
    greeks.gamma = pdf_d1 / (S * sigma * std::sqrt(T));
    greeks.vega = S * pdf_d1 * sqrt(T);

    //cálculo de delta e theta varia conforme o tipo de opção
    if (type == OptionType::Call) {
        greeks.delta = cdf_d1;
        greeks.theta = -(S * pdf_d1 * sigma / (2.0 * std::sqrt(T))) - (r * K * std::exp(-r * T) * cdf_d2);
    } else {
        greeks.delta = cdf_d1 - 1.0;
        const double cdf_minus_d2 = 0.5 * (1.0 + std::erf(-d2 / std::sqrt(2.0)));
        greeks.theta = -(S * pdf_d1 * sigma / (2.0 * std::sqrt(T))) + r * K * std::exp(-r * T) * cdf_minus_d2;
    }

    return greeks;
}

PortfolioGreeks Portfolio::calculateTotalGreeks() const noexcept {
    PortfolioGreeks total{};        //{} após nome da variável na inicialização garante que todos os campos internos da struct sejam zerados seguramente na criação
    total.delta += spotShares;      //contibuição da posição em ações à vista (delta = 1.0 por ação)
            
    //agregação linear das opções na carteira    
    for (const auto& pos : position){       
            //cálculo das gregas unitárias (1 contrato/order)
            PortfolioGreeks unitGreeks =  calcularGregasEuropeias(pos.type, pos.S, pos.K, pos.T, pos.r, pos.sigma);

            total.delta += unitGreeks.delta * pos.quantity;
            total.gamma += unitGreeks.gamma * pos.quantity;
            total.vega  += unitGreeks.vega  * pos.quantity;
            total.theta += unitGreeks.theta * pos.quantity;
        
        }

    return total;
}
