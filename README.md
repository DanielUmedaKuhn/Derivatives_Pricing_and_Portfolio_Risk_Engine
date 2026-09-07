# Motor de Precificação de Opções e Análise de Risco de Portfólio

Um motor de precificação de opções e análise de risco (Greeks e P&L) desenvolvido em C++. O sistema suporta opções Europeias e Americanas, incorporando a extensão de Merton para dividendos contínuos e simulações de estresse de portfólio por meio de matrizes bidimensionais.

## Funcionalidades Principais

* **Precificação Multimodelo:**
  * **Black-Scholes-Merton:** Cálculo analítico para opções Europeias (Calls e Puts), englobando taxas de juros e dividendos contínuos.
  * **Árvore Binomial (CRR):** Precificação numérica para opções Americanas com Backward Induction.
* **Cálculo de Gregas (Sensibilidades):**
  * Soluções fechadas (Black-Scholes) e por diferenças finitas (CRR) para: **Delta, Gamma, Theta, Vega e Rho**.
* **Risk Management:**
  * Consumo dinâmico de múltiplas posições (*Long/Short*) via terminal.
  * Neutralização automática de risco direcional considerando ações Spot e derivativos.
* **Matriz de Estresse (Heat Map):**
  * Simulação de P&L cruzando choques direcionais no preço do ativo-objeto (Spot) com choques na volatilidade implícita.

## Arquitetura do Sistema

O projeto adota uma arquitetura modular orientada a objetos e separação de responsabilidades matemáticas:

* `Types.hpp`: Estruturas de dados fundamentais (`OptionPosition`, `PortfolioGreeks`, `CRRResult`) e *Enums*.
* `BlackScholes.hpp`: Implementação matemática das CDF/PDF da distribuição normal e equações de Merton.
* `BinomialTree.hpp`: Motor numérico com árvores paralelas para cálculo de Vega e Rho via choque de variáveis.
* `Portfolio.hpp / .cpp`: Agregador de risco vetorial e gerador da matriz de estresse bidimensional.
* `main.cpp`: Interface de linha de comando interativa para montagem da carteira e exibição de relatórios formatados.

## Para Compilar e Executar:

Possuir um compilador C++ (suporte a C++17 ou superior) instalado.

---
Autor: [Daniel Umeda Kuhn](https://github.com/DanielUmedaKuhn)
