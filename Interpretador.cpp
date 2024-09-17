#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cctype>

using namespace std;

vector<string> buffer;
map<string, int> Label;
vector<pair<int, string>> label;

struct tipo {
    int type;
    string valor;

    tipo() : type(0), valor("") {}
    tipo(int a, const string& x) : type(a), valor(x) {}
};

map<string, tipo> var;

void leitura(const string& file) {
    ifstream a(file);
    if (!a.is_open()) {
        cerr << "Erro --> Nao abriu o arquivo" << endl;
        exit(EXIT_FAILURE);
    }

    string linha;
    while (getline(a, linha)) {
        istringstream aux(linha);
        string token;
        bool dentroAspas = false;
        string stringCompleta;

        while (aux >> token) {
            if (!dentroAspas && token.front() == '"') {
                dentroAspas = true;
                stringCompleta = token;
                if (token.back() == '"') {
                    dentroAspas = false;
                    buffer.push_back(stringCompleta);
                }
            } else if (dentroAspas) {
                stringCompleta += " " + token;
                if (token.back() == '"') {
                    dentroAspas = false;
                    buffer.push_back(stringCompleta);
                }
            } else {
                buffer.push_back(token);
            }
        }
        buffer.push_back(";");
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
        cerr << "Erro -> Variavel nao existe: " << varName << endl;
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
    string valor;
    cin >> valor;

    if (valor.find('.') != string::npos) {
        var[varName] = tipo(1, valor);
    } else {
        var[varName] = tipo(0, valor);
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
        cerr << "Erro -> Label nao encontrado: " << buffer[pos] << endl;
        exit(EXIT_FAILURE);
    }
}

bool IF(size_t& pos) {
    pos++;
    string condition = buffer[pos];

    size_t opPos = condition.find_first_of("<>!=");
    if (opPos == string::npos) {
        cerr << "Erro -> Operador nao encontrado na condicao: " << condition << endl;
        exit(EXIT_FAILURE);
    }

    string op;
    size_t opLength = 1;
    if (condition[opPos + 1] == '=') {
        opLength = 2;
    }
    op = condition.substr(opPos, opLength);

    string varA = condition.substr(0, opPos);
    string varB = condition.substr(opPos + opLength);

    auto getValue = [](const string& varName) -> pair<int, string> {
        if (var.count(varName)) {
            return make_pair(var[varName].type, var[varName].valor);
        } else {
            if (varName.find('.') != string::npos) {
                return make_pair(1, varName);
            } else if (isdigit(varName[0]) || (varName[0] == '-' && isdigit(varName[1]))) {
                return make_pair(0, varName);
            } else {
                return make_pair(2, varName);
            }
        }
    };

    pair<int, string> valueA = getValue(varA);
    int typeA = valueA.first;
    string valA = valueA.second;

    pair<int, string> valueB = getValue(varB);
    int typeB = valueB.first;
    string valB = valueB.second;


    if (!((typeA == 0 || typeA == 1) && (typeB == 0 || typeB == 1))) {
        if (typeA != typeB) {
            cerr << "Erro -> Tipos incompativeis na comparacao: " << varA << " e " << varB << endl;
            exit(EXIT_FAILURE);
        }
    }

    bool result = false;
    if (typeA == 0 || typeA == 1) {
        double x = stod(valA);
        double y = stod(valB);
        if (op == ">")
            result = x > y;
        else if (op == "<")
            result = x < y;
        else if (op == ">=")
            result = x >= y;
        else if (op == "<=")
            result = x <= y;
        else if (op == "==")
            result = x == y;
        else if (op == "!=")
            result = x != y;
    } else if (typeA == 2) {
        if (op == ">")
            result = valA > valB;
        else if (op == "<")
            result = valA < valB;
        else if (op == ">=")
            result = valA >= valB;
        else if (op == "<=")
            result = valA <= valB;
        else if (op == "==")
            result = valA == valB;
        else if (op == "!=")
            result = valA != valB;
    }

    pos++;
    return result;
}

vector<string> tokenize_expr(const string& expr) {
    vector<string> tokens;
    size_t i = 0;
    while (i < expr.size()) {
        if (isspace(expr[i])) {
            ++i;
        } else if (expr[i] == '+' || expr[i] == '-') {
            tokens.push_back(string(1, expr[i]));
            ++i;
        } else {
            size_t j = i;
            while (j < expr.size() && (isalnum(expr[j]) || expr[j] == '.')) {
                ++j;
            }
            tokens.push_back(expr.substr(i, j - i));
            i = j;
        }
    }
    return tokens;
}

void expressao(size_t pos, const string& varName) {
    string expr = buffer[pos].substr(buffer[pos].find('=') + 1);
    vector<string> tokens = tokenize_expr(expr);

    double result = 0.0;
    char op = '+';

    for (const string& t : tokens) {
        if (t == "+" || t == "-") {
            op = t[0];
        } else {
            double val = 0.0;
            if (var.count(t)) {
                val = stod(var[t].valor);
            } else {
                val = stod(t);
            }
            if (op == '+')
                result += val;
            else if (op == '-')
                result -= val;
        }
    }


    double intpart;
    if (modf(result, &intpart) == 0.0) {
        var[varName] = tipo(0, to_string(int(result)));
    } else {
        var[varName] = tipo(1, to_string(result));
    }
}

void interpretador() {
    cria_label();
    for (size_t pos = 0; pos < buffer.size();) {
        if (buffer[pos] == "HALT" || buffer[pos] == "halt")
            break;

        if (buffer[pos] == "rem") {
            while (buffer[pos] != ";")
                pos++;
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
                while (buffer[pos] != ";")
                    pos++;
                pos++;
            }
            continue;
        }
        
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
    string file = "/Users/arthurcosta/Desktop/Faculdade/Compiladores/Fonte.txt";
    leitura(file);
    interpretador();
    return 0;
}
