#include <iostream>
#include <ilcplex/ilocplex.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

typedef IloArray<IloNumVarArray> Var2D; // Nos permite definir variaveis de decisão 2D

void printa(string nome, int valor)
{
    cout << nome << ": " << valor << endl;
}

void printa(string nome, int* valores, int tam)
{
    cout << nome << ": ";
    for(int i = 0; i < tam; i++)
    {
        cout << valores[i] << " ";
    }
    cout << endl;
}

int main(int argc, char *argv[])
{
    string fname = argv[1];

    // Definicao e leitura dos Dados do modelo
    vector<vector<string>> content;
    vector<string> row;
    string line, word;

    fstream file(fname, ios::in);
    if (file.is_open())
    {
        while (getline(file, line))
        {
            row.clear();

            stringstream str(line);

            while (getline(str, word, ','))
            {
                row.push_back(word);
            }

            content.push_back(row);
        }
    }
    else
    {
        cout << "Não foi possível abrir o arquivo\n";
        cout << "Confirme se o arquivo '"<< fname <<"' está na pasta raiz do projeto\n";
        exit(1);
    }

    int numDePassageiros = stoi(content[0][1]);
    int numDeVoos = stoi(content[1][1]);
    int PEN = stoi(content[2][1]);
    int ATR = stoi(content[3][1]);
    int MNC = stoi(content[4][1]);
    int MXC = stoi(content[5][1]);

    int* AOv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        AOv[i] = stoi(content[6][i + 1]); // i + 1 pra ignorar a primeira coluna
    }

    int* ADv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        ADv[i] = stoi(content[7][i + 1]);
    }

    int* CAPv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        CAPv[i] = stoi(content[8][i + 1]);
    }

    int* CSTv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        CSTv[i] = stoi(content[9][i + 1]);
    }

    int* HPv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        HPv[i] = stoi(content[10][i + 1]);
    }

    int* HCv = new int[numDeVoos];
    for (int i = 0; i < numDeVoos; i++)
    {
        HCv[i] = stoi(content[11][i + 1]);
    }

    int* DESTp = new int[numDePassageiros];
    for (int i = 0; i < numDePassageiros; i++)
    {
        DESTp[i] = stoi(content[12][i + 1]);
    }

    int* PARTp = new int[numDePassageiros];
    for (int i = 0; i < numDePassageiros; i++)
    {
        PARTp[i] = stoi(content[13][i + 1]);
    }

    int* CHEGp = new int[numDePassageiros];
    for (int i = 0; i < numDePassageiros; i++)
    {
        CHEGp[i] = stoi(content[14][i + 1]);
    }

    // Printa dados
    cout << "Dados de entrada\n";
    printa("numDePassageiros", numDePassageiros);
    printa("numDeVoos", numDeVoos);
    printa("PEN", PEN);
    printa("ATR", ATR);
    printa("MNC", MNC);
    printa("MXC", MXC);
    printa("AOv", AOv, numDeVoos);
    printa("ADv", ADv, numDeVoos);
    printa("CAPv", CAPv, numDeVoos);
    printa("CSTv", CSTv, numDeVoos);
    printa("HPv", HPv, numDeVoos);
    printa("HCv", HCv, numDeVoos);
    printa("DESTp", DESTp, numDePassageiros);
    printa("PARTp", PARTp, numDePassageiros);
    printa("CHEGp", CHEGp, numDePassageiros);
    cout << endl << "Processando..." << endl;

    // Declara o ambiente e o modelo matematico
    IloEnv env;
    IloModel model;
    model = IloModel(env);

    // Variaveis de decisao binarias =====================================================
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

    // Restricoes ========================================================================
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
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            if(a != 0 && a != DESTp[p])
            {
                IloExpr expr1(env);
                IloExpr expr2(env);
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
        expr += (PARTp[p] * x[p]);
        IloRange restricao(env, PARTp[p], expr, IloInfinity);
        model.add(restricao);
    }

    // Restricao (7) assegura que o tempo mı́nimo entre conexões seja respeitado
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            if (a != 0 && a != DESTp[p])
            {
                IloExpr expr1(env);
                IloExpr expr2(env);
                for(int v = 0; v < numDeVoos; v++)
                {
                    if(ADv[v] == a)
                    {
                        expr1 += ((HCv[v] * MNC) * y[p][v]);
                    }
                    if(AOv[v] == a)
                    {
                        expr2 += (HPv[v] * y[p][v]);
                    }
                }
                IloRange restricao(env, -IloInfinity, expr1 - expr2, 0);
                model.add(restricao);
            }
        }
    }

    // Restricao (8) refere-se ao tempo máximo em que uma conexão pode ocorrer
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            if (a != 0 && a != DESTp[p])
            {
                IloExpr expr1(env);
                IloExpr expr2(env);
                for(int v = 0; v < numDeVoos; v++)
                {
                    if(AOv[v] == a)
                    {
                        expr1 += (HPv[v] * y[p][v]);
                    }
                    if(ADv[v] == a)
                    {
                        expr2 += (HCv[v] * y[p][v]);
                    }
                }
                IloRange restricao(env, -IloInfinity, expr1 - expr2, MXC);
                model.add(restricao);
            }
        }
    }

    // Restricao (9) e (10) evitam a criação de ciclos em um itinerário, garantindo
    // que um passageiro não retorne a um aeroporto já visitado.
    // (9)
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            IloExpr expr(env);
            for(int v = 0; v < numDeVoos; v++)
            {
                if(AOv[v] == a)
                {
                    expr += y[p][v];
                }
            }
            IloRange restricao(env, -IloInfinity, expr, 1);
            model.add(restricao);
        }
    }
    // (10)
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int a = 0; a < numDeVoos; a++)
        {
            IloExpr expr(env);
            for(int v = 0; v < numDeVoos; v++)
            {
                if(ADv[v] == a)
                {
                    expr += y[p][v];
                }
            }
            IloRange restricao(env, -IloInfinity, expr, 1);
            model.add(restricao);
        }
    }

    // Resolvendo o modelo
    IloCplex cplex(model);
    cplex.exportModel("ModeloExportado.lp");

    if(!cplex.solve())
    {
        env.error() << "Erro ao rodar modelo!" << endl;
        exit(2);
    }

    double obj = cplex.getObjValue();
    cout << "\nValor da FO: " << obj << endl;

    cout << "Valores de x:" << endl;
    for(int p = 0; p < numDePassageiros; p++)
    {
        double xValue = cplex.getValue(x[p]);
        cout << "\t\t x[" << p << "] = " << xValue << endl;
    }

    cout << "Valores de y:" << endl;
    for(int p = 0; p < numDePassageiros; p++)
    {
        for(int v = 0; v < numDeVoos; v++)
        {
            double yValue = cplex.getValue(y[p][v]);
            cout << "\t\t y[" << p << "][" << v << "] = " << yValue << endl;
        }
    }

    return 0;
}
