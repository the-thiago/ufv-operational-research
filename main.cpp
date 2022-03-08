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
    IloModel Model(env);     // Declara o modelo dentro do ambiente

    // Dados do modelo
    // todo
    int CSTv = 10;
    int PEN = 10;
    int numDePassageiros = 10;
    int numDeVoos = 10;


    // Variaveis de decisao binarias
    IloNumVarArray passagNegligenciado(env, numDePassageiros, 0, 1, IloNumVar::Bool);
    Var2D passagEmVoo(env, numDePassageiros);

    for(int p = 0; p < numDePassageiros; p++)
    {
        passagEmVoo[p] = IloNumVarArray(env, numDeVoos, 0, 1, IloNumVar::Bool);
    }

    // Funcao objetivo
    IloExpr custoRealocacao(env);
    IloExpr penalidadeNeglig(env);
    IloExpr passagInsatisfacao(env);

    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            custoRealocacao += CSTv * passagEmVoo[p][v];
        }
    }
    Model.add(IloMinimize(env, custoRealocacao));

    for(int p = 0; p < numDePassageiros; p++)
    {
        penalidadeNeglig += PEN * passagNegligenciado[p];
    }
    Model.add(IloMinimize(env, penalidadeNeglig));

    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            // se o destino do passageiro for diferente do destino do voo v, então break
            passagInsatisfacao += PEN * passagNegligenciado[p]; // todo expressao errada
        }
    }
    Model.add(IloMinimize(env, passagInsatisfacao));

    // Restricoes


    return 0;
}
