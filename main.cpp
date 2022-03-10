#include <iostream>
#include <ilcplex/ilocplex.h>

using namespace std;

typedef IloArray<IloNumVarArray> Var2D; // Nos permite definir variaveis de decisão 2D

int main()
{
    string escolha = "";

    do
    {
        cout << "Digite o numero da opcao de entrada:" << endl;
        cout << "1 - entrada1.txt" << endl;
        cout << "2 - entrada2.txt" << endl;
        cin >> escolha;
    }
    while(escolha != "1" && escolha != "2");

    cout << endl << "Processando..." << endl;

    IloEnv env;             // Declara o ambiente
    IloModel Model(env);    // Declara o modelo dentro do ambiente

    // Dados do modelo
    // todo

    int numDePassageiros = 2;
    int numDeVoos = 2;
    int ATR = 100;
    int PEN = 100;
    int* CSTv = new int[numDeVoos] { 0, 0 };
    int* HCv = new int[numDeVoos] { 0, 0 };
    int* ADv = new int[numDeVoos] { 0, 0 };
    int* AOv = new int[numDeVoos] { 0, 0 };
    int* CHEGp = new int[numDePassageiros] { 0, 0 };
    int* DESTp = new int[numDePassageiros] { 0, 0 };


    // Variaveis de decisao binarias
    // Variavel booleana: passageiro p foi negligenciado na realocacao?
    IloNumVarArray x(env, numDePassageiros, 0, 1, IloNumVar::Bool);
    // Variavel booleana de duas dimensoes: passageiro p está alocado ao voo v?
    Var2D y(env, numDePassageiros);

    for(int p = 0; p < numDePassageiros; p++)
    {
        y[p] = IloNumVarArray(env, numDeVoos, 0, 1, IloNumVar::Bool);
    }

    // Funcao objetivo (1) ===============================================================
    IloExpr custoRealocacao(env);

    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            custoRealocacao += (CSTv[v] * y[p][v]);
        }
    }

    IloExpr penalidadeNeglig(env);

    for(int p = 0; p < numDePassageiros; p++)
    {
        penalidadeNeglig += (PEN * x[p]);
    }

    IloExpr passagInsatisfacao(env);

    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            if(DESTp[p] == ADv[v])
            {
                passagInsatisfacao += (ATR * (HCv[v] - CHEGp[p]) * y[p][v]);
            }
        }
    }
    Model.add(IloMinimize(env, custoRealocacao + penalidadeNeglig + passagInsatisfacao));

    // Restricoes =======================================================================
    // Restricao (2) garante que cada passageiro saia do aeroporto de origem ou, então,
    // que seja indicada a sua não realocação
    for(int p = 0; p < numDePassageiros; p++)
    {
        IloExpr expr(env);
        for(int v = 0; v < numDeVoos; v++)
        {
            if (AOv[v] == 0)
            {
                expr += y[p][v];
            }
        }
        expr += x[p];
        Model.add(expr == 1);
    }

    // Restricao (3) impõem a condição que cada passageiro realocado chegue a seu
    // destino final
    for(int p = 0; p < numDePassageiros; p++)
    {
        IloExpr expr(env);
        for(int v = 0; v < numDeVoos; v++)
        {
            if (ADv[v] == DESTp[p])
            {
                expr += y[p][v];
            }
        }
        expr += x[p];
        Model.add(expr == 1);
    }

    // Restricao (4) assegura a continuidade de um itinerário durante as conexões
    // entre aeroportos

    // Restricao (5) certifica que um voo obedeça sua capacidade máxima

    // Restricao (6) garante que um passageiro parta do aeroporto de origem somente
    // após a hora de partida do seu itinerário original

    // Restricao (7) assegura que o tempo mı́nimo entre conexõesseja respeitado

    // Restricao (8) refere-se ao tempo máximo em que uma conexão pode ocorrer

    // Restricao (9) e (10) evitam a criação de ciclos em um itinerário, garantindo
    // que um passageiro não retorne a um aeroporto já visitado.

    // Resolvendo o modelo
    /*
    IloCplex cplex(Model);
    cplex.exportModel("ModeloExportado.lp");
    if(!cplex.solve())
    {
        env.error() << "Erro ao rodar modelo!" << endl;
        throw(-1);
    }
    double obj = cplex.getObjValue();
    cout << "Valor da FO: " << obj << endl;

    cout << "Valores de x:" << endl;
    for(int p = 0; p < numDePassageiros; p++)
    {
        double xValue = cplex.getValue(x[p]);
        count << "\t\t x[" << p << "] = " << xValue << endl;
    }

    cout << "Valores de y:" << endl;
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            double yValue = cplex.getValue(y[p][v]);
            count << "\t\t y[" << p << "][" << v << "] = " << yValue << endl;
        }
    }
    */
    return 0;
}













