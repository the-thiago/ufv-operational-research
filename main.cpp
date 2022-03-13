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

    // Dados do modelo
    int numDePassageiros = 2;
    int numDeVoos = 2;
    int PEN = 100;
    int ATR = 100;
    int MNC = 100;
    int MXC = 100;
    int* AOv = new int[numDeVoos] { 0, 0 };
    int* ADv = new int[numDeVoos] { 0, 0 };
    int* CAPv = new int[numDeVoos] { 0, 0 };
    int* CSTv = new int[numDeVoos] { 0, 0 };
    int* HPv = new int[numDeVoos] { 0, 0 };
    int* HCv = new int[numDeVoos] { 0, 0 };
    int* DESTp = new int[numDePassageiros] { 0, 0 };
    int* PARTp = new int[numDePassageiros] { 0, 0 };
    int* CHEGp = new int[numDePassageiros] { 0, 0 };

    // todo: ler dados e printar eles

    // Declara o ambiente e o modelo matematico
    IloEnv env;
    IloModel model;
    model = IloModel(env);

    // Variaveis de decisao binarias
    // Variavel booleana: passageiro p foi negligenciado na realocacao?
    IloNumVarArray x(env, numDePassageiros, 0, 1, IloNumVar::Bool);
    for(int p = 0; p < numDePassageiros; p++)
    {
        stringstream var;
        var << "x[" << p << "]";
        x[p].setName(var.str().c_str());
        model.add(x[p]);
    }

    // Variavel booleana de duas dimensoes: passageiro p está alocado ao voo v?
    Var2D y(env, numDePassageiros);

    for(int p = 0; p < numDePassageiros; p++)
    {
        y[p] = IloNumVarArray(env, numDeVoos, 0, 1, IloNumVar::Bool);
    }
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            stringstream var;
            var << "y[" << p << "][" << v << "]";
            y[p][v].setName(var.str().c_str());
            model.add(y[p][v]);
        }
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
    model.add(IloMinimize(env, custoRealocacao + penalidadeNeglig + passagInsatisfacao));

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
        IloRange restricao(env, 1, expr, 1);
        model.add(restricao);
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
        IloRange restricao(env, 1, expr, 1);
        model.add(restricao);
    }

    // Restricao (4) assegura a continuidade de um itinerário durante as conexões
    // entre aeroportos
    for(int pas = 0; pas < numDePassageiros; pas++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            if(a != 0 && a != DESTp[pas])
            {
                IloExpr expr1(env);
                IloExpr expr2(env);
                for(int p = 0; p < numDePassageiros; p++)
                {
                    for(int v = 0; v < numDeVoos; v++)
                    {
                        if(ADv[v] == a)
                        {
                            expr1 += y[p][v];
                        }
                        if(AOv[v] == a)
                        {
                            expr2 += y[p][v];
                        }
                    }
                }
                IloRange restricao(env, 0, expr1 - expr2, 0);
                model.add(restricao);
            }
        }
    }

    // Restricao (5) certifica que um voo obedeça sua capacidade máxima
    for(int v = 0; v < numDeVoos; v++)
    {
        IloExpr expr(env);
        for(int p = 0; p < numDePassageiros; p++)
        {
            expr += y[p][v];
        }
        IloRange restricao(env, -IloInfinity, expr, CAPv[v]);
        model.add(restricao);
    }

    // Restricao (6) garante que um passageiro parta do aeroporto de origem somente
    // após a hora de partida do seu itinerário original
    for(int p = 0; p < numDePassageiros; p++)
    {
        IloExpr expr(env);
        for(int v = 0; v < numDeVoos; v++)
        {
            if(AOv[v] == 0)
            {
                expr += (HPv[v] * y[p][v]);
            }
        }
        // (PARTp[p] * (1 - x[p]))
        // todo fix this: IloRange restricao(env, (PARTp[p] * (1 - x[p])), expr, IloInfinity);
        //model.add(restricao);
    }

    // Restricao (7) assegura que o tempo mı́nimo entre conexõesseja respeitado

    // Restricao (8) refere-se ao tempo máximo em que uma conexão pode ocorrer

    // Restricao (9) e (10) evitam a criação de ciclos em um itinerário, garantindo
    // que um passageiro não retorne a um aeroporto já visitado.
    // (9)
    for(int pas = 0; pas < numDePassageiros; pas++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            IloExpr expr(env);
            for(int p = 0; p < numDePassageiros; p++)
            {
                for(int v = 0; v < numDeVoos; v++)
                {
                    if(AOv[v] == a)
                    {
                        expr += y[p][v];
                    }
                }
            }
            IloRange restricao(env, -IloInfinity, expr, 1);
            model.add(restricao);
        }
    }
    // (10)
    for(int pas = 0; pas < numDePassageiros; pas++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            IloExpr expr(env);
            for(int p = 0; p < numDePassageiros; p++)
            {
                for(int v = 0; v < numDeVoos; v++)
                {
                    if(ADv[v] == a)
                    {
                        expr += y[p][v];
                    }
                }
            }
            IloRange restricao(env, -IloInfinity, expr, 1);
            model.add(restricao);
        }
    }

    // Resolvendo o modelo
    /*
    IloCplex cplex(model);
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
