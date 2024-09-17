#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

vector<string> buffer;
map<string, int> Label;
vector<pair<int, string>> label;

struct tipo {
    int type;       // 0: int, 1: float, 2: char
    string valor;   // Valor armazenado

    tipo() : type(0), valor("") {}
    tipo(int a, const string& x) : type(a), valor(x) {}
};

map<string, tipo> var;

void leitura(const string& file) {
    ifstream a(file);
    if (!a.is_open()) {
        cerr << "ERRO --> NÃO ABRIU O ARQUIVO" << endl;
        exit(EXIT_FAILURE);
    }

    string linha;
    while (getline(a, linha)) {
        string s;
        istringstream aux(linha);
        while (aux >> s) {
            buffer.push_back(s);
        }
        buffer.push_back(";"); // Marca o fim da linha
    }
}

void cria_label() {
    for (size_t pos = 0; pos < buffer.size(); pos++) {
        if (buffer[pos] == ";") {
            pos++;
            if (pos < buffer.size() && !Label.count(buffer[pos])) {
                Label[buffer[pos]] = static_cast<int>(pos);
                label.emplace_back(static_cast<int>(pos), buffer[pos]);
            }
        }
    }
}

string print_var(size_t pos) {
    string varName = buffer[pos];
    if (!var.count(varName)) {
        cerr << "ERRO -> Variável não existe: " << varName << endl;
        exit(EXIT_FAILURE);
    }
    return var[varName].valor;
}

void Print(size_t& pos) {
    pos++;
    if (buffer[pos][0] == '"') {
        string str = buffer[pos].substr(1, buffer[pos].size() - 2);
        cout << str << endl;
        pos++;
    } else {
        cout << print_var(pos) << endl;
        pos++;
    }
}

void input_var(const string& varName) {
    // Não precisa verificar se a variável já existe, pois queremos permitir sobrescrever
    string valor;
    cin >> valor;

    if (valor.size() == 1 && isalpha(valor[0])) {
        var[varName] = tipo(2, valor); // Char
    } else if (valor.find('.') != string::npos) {
        var[varName] = tipo(1, valor); // Float
    } else {
        var[varName] = tipo(0, valor); // Int
    }
}

void Input(size_t& pos) {
    pos++;
    string varName = buffer[pos];
    input_var(varName);
    pos++;
}

int busca_goto(const string& labelName) {
    if (Label.count(labelName)) {
        return Label[labelName];
    }
    return -1;
}

void Goto(size_t& pos) {
    pos++;
    int gotoPos = busca_goto(buffer[pos]);
    if (gotoPos != -1) {
        pos = static_cast<size_t>(gotoPos);
    } else {
        cerr << "ERRO -> Label não encontrado: " << buffer[pos] << endl;
        exit(EXIT_FAILURE);
    }
}

bool IF(size_t& pos) {
    pos++;
    string condition = buffer[pos];

    size_t opPos = condition.find_first_of("<>!=");
    if (opPos == string::npos) {
        cerr << "ERRO -> Operador não encontrado na condição: " << condition << endl;
        exit(EXIT_FAILURE);
    }

    string varA = condition.substr(0, opPos);
    string op;
    size_t opLength = 1;

    if (condition[opPos + 1] == '=') {
        opLength = 2;
    }
    op = condition.substr(opPos, opLength);
    string varB = condition.substr(opPos + opLength);

    auto getValue = [](const string& varName) -> pair<int, string> {
        if (var.count(varName)) {
            return make_pair(var[varName].type, var[varName].valor);
        } else {
            // Tenta converter para número
            if (varName.find('.') != string::npos) {
                return make_pair(1, varName); // Float
            } else if (isdigit(varName[0]) || (varName[0] == '-' && isdigit(varName[1]))) {
                return make_pair(0, varName); // Int
            } else {
                return make_pair(2, varName); // Char ou string
            }
        }
    };

    pair<int, string> valueA = getValue(varA);
    int typeA = valueA.first;
    string valA = valueA.second;

    pair<int, string> valueB = getValue(varB);
    int typeB = valueB.first;
    string valB = valueB.second;

    if (typeA != typeB) {
        cerr << "ERRO -> Tipos incompatíveis na comparação: " << varA << " e " << varB << endl;
        exit(EXIT_FAILURE);
    }

    bool result = false;
    if (typeA == 0) { // Int
        int x = stoi(valA);
        int y = stoi(valB);
        if (op == ">") result = x > y;
        else if (op == "<") result = x < y;
        else if (op == ">=") result = x >= y;
        else if (op == "<=") result = x <= y;
        else if (op == "==") result = x == y;
        else if (op == "!=") result = x != y;
    } else if (typeA == 1) { // Float
        double x = stod(valA);
        double y = stod(valB);
        if (op == ">") result = x > y;
        else if (op == "<") result = x < y;
        else if (op == ">=") result = x >= y;
        else if (op == "<=") result = x <= y;
        else if (op == "==") result = x == y;
        else if (op == "!=") result = x != y;
    } else if (typeA == 2) { // Char ou string
        if (op == ">") result = valA > valB;
        else if (op == "<") result = valA < valB;
        else if (op == ">=") result = valA >= valB;
        else if (op == "<=") result = valA <= valB;
        else if (op == "==") result = valA == valB;
        else if (op == "!=") result = valA != valB;
    }

    pos++;
    return result;
}

void expressao(size_t pos, const string& varName) {
    string expr = buffer[pos].substr(buffer[pos].find('=') + 1);
    istringstream iss(expr);
    string token;
    vector<string> tokens;
    while (iss >> token) {
        tokens.push_back(token);
    }

    double result = 0.0;
    char op = '+';

    for (const auto& t : tokens) {
        if (t == "+" || t == "-") {
            op = t[0];
        } else {
            double val = 0.0;
            if (var.count(t)) {
                val = stod(var[t].valor);
            } else {
                val = stod(t);
            }
            if (op == '+') result += val;
            else if (op == '-') result -= val;
        }
    }

    var[varName] = tipo(1, to_string(result));
}

void interpretador() {
    cria_label();
    for (size_t pos = 0; pos < buffer.size();) {
        if (buffer[pos] == "halt") break;

        if (buffer[pos] == "rem") {
            while (buffer[pos] != ";") pos++;
            pos++;
            continue;
        }

        if (buffer[pos] == ";") {
            pos++;
            continue;
        }

        if (buffer[pos] == "print") {
            Print(pos);
            continue;
        }

        if (buffer[pos] == "input") {
            Input(pos);
            continue;
        }

        if (buffer[pos] == "goto") {
            Goto(pos);
            continue;
        }

        if (buffer[pos] == "if") {
            if (IF(pos)) {
                if (buffer[pos] == "goto") {
                    Goto(pos);
                } else if (buffer[pos] == "print") {
                    Print(pos);
                } else {
                    string varName = buffer[pos].substr(0, buffer[pos].find('='));
                    expressao(pos, varName);
                    pos++;
                }
            } else {
                while (buffer[pos] != ";") pos++;
                pos++;
            }
            continue;
        }

        // Atribuição direta
        if (buffer[pos].find('=') != string::npos) {
            string varName = buffer[pos].substr(0, buffer[pos].find('='));
            expressao(pos, varName);
            pos++;
            continue;
        }

        pos++;
    }
}

int main() {
    string file = "Fonte.txt";
    leitura(file);
    interpretador();
    return 0;
}
