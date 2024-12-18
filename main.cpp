#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <numeric>
#include <filesystem>
#include <cstring>

using namespace std;

struct Edge {
    int start, end;
    double attribute;
};

struct Vertex {
    double attribute;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file: " << argv[1] << endl;
        cerr << "Current working directory: " << endl;
        
        // Добавим вывод содержимого директории
        cerr << "\nDirectory contents:" << endl;
        try {
            for (const auto& entry : filesystem::directory_iterator(".")) {
                cerr << entry.path().filename() << endl;
            }
        } catch (const filesystem::filesystem_error& e) {
            cerr << "Error listing directory: " << e.what() << endl;
        }

        // Добавим проверку прав доступа
        try {
            filesystem::path inputPath(argv[1]);
            if (filesystem::exists(inputPath)) {
                cerr << "\nFile permissions:" << endl;
                auto perms = filesystem::status(inputPath).permissions();
                cerr << "Read permission: " 
                     << ((perms & filesystem::perms::owner_read) != filesystem::perms::none) 
                     << endl;
            }
        } catch (const filesystem::filesystem_error& e) {
            cerr << "Error checking permissions: " << e.what() << endl;
        }

        // Добавим информацию о системных ошибках
        cerr << "\nSystem error: " << strerror(errno) << endl;
        
        cerr << "File exists check: " << (filesystem::exists(argv[1]) ? "Yes" : "No") << endl;
        cerr << "Full path: " << filesystem::absolute(argv[1]) << endl;
        return 1;
    }

    ofstream outputFile(argv[2]);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << argv[2] << endl;
        cerr << "Current working directory: ";
        system("cd");
        return 1;
    }

    int NV, NE;
    string firstLine;
    getline(inputFile, firstLine);
    
    // Удаляем BOM, если он присутствует
    if (!firstLine.empty() && (unsigned char)firstLine[0] == 0xEF &&
        (unsigned char)firstLine[1] == 0xBB && 
        (unsigned char)firstLine[2] == 0xBF) {
        firstLine = firstLine.substr(3);
    }
    
    cout << "First line: '" << firstLine << "'" << endl;
    
    istringstream iss(firstLine);
    if (!(iss >> NV >> NE)) {
        cerr << "Error reading NV and NE from input file" << endl;
        cerr << "NV should be number of vertices, NE should be number of edges" << endl;
        cerr << "First line should contain two numbers separated by space" << endl;
        return 1;
    }
    inputFile.ignore(); // Ignore the newline after the first line

    vector<Edge> edges(NE);
    vector<Vertex> vertices(NV);

    // Инициализация атрибутов
    for(auto& vertex : vertices) {
        vertex.attribute = 0.0;
    }
    for(auto& edge : edges) {
        edge.attribute = 0.0;
    }

    // Сначала обрабатываем правила для рёбер
    for (int i = 0; i < NE; ++i) {
        string line;
        getline(inputFile, line);
        istringstream lineStream(line);
        
        // Читаем начальную и конечную вершины ребра
        lineStream >> edges[i].start >> edges[i].end;
        
        // Читаем правило для атрибута
        string rule;
        getline(lineStream, rule);
        // Удаляем начальные пробелы из правила
        rule.erase(0, rule.find_first_not_of(" \t"));
        
        cout << "Edge " << i + 1 << ": " << edges[i].start << " -> " << edges[i].end << " Rule: " << rule << endl;

        if (isdigit(rule[0]) || rule[0] == '.') {
            istringstream ruleStream(rule);
            ruleStream >> edges[i].attribute;
            cout << "Edge " << i + 1 << " value: " << edges[i].attribute << endl;
        } else if (rule[0] == 'e') {
            int sourceEdgeIndex;
            rule = rule.substr(1); // Удаляем 'e'
            istringstream ruleStream(rule);
            ruleStream >> sourceEdgeIndex;
            edges[i].attribute = edges[sourceEdgeIndex - 1].attribute;
            cout << "Edge " << i + 1 << " copying from edge " << sourceEdgeIndex << endl;
        }
    }

    // Затем правила для вершин
    for (int i = 0; i < NV; ++i) {
        string rule;
        getline(inputFile, rule);
        cout << "Rule " << i + 1 << ": " << rule << endl;
        istringstream ruleStream(rule);

        if (isdigit(rule[0]) || rule[0] == '.') {
            ruleStream >> vertices[i].attribute;
            cout << "Vertex " << i + 1 << " value: " << vertices[i].attribute << endl;
        } else if (rule[0] == 'v') {
            int vertexIndex;
            ruleStream.ignore(1); // Ignore 'v'
            ruleStream >> vertexIndex;
            vertices[i].attribute = vertices[vertexIndex - 1].attribute;
            cout << "Vertex " << i + 1 << " copying from vertex " << vertexIndex << endl;
        } else if (rule == "min") {
            vector<double> edgeAttributes;
            for (const auto& edge : edges) {
                if (edge.end == i + 1) {
                    edgeAttributes.push_back(edge.attribute);
                    cout << "Found incoming edge with value: " << edge.attribute << endl;
                }
            }
            if (!edgeAttributes.empty()) {
                vertices[i].attribute = *min_element(edgeAttributes.begin(), edgeAttributes.end());
            }
            cout << "Vertex " << i + 1 << " min value: " << vertices[i].attribute << endl;
        }
    }

    // Output results
    for (const auto& vertex : vertices) {
        outputFile << vertex.attribute << endl;
    }
    for (const auto& edge : edges) {
        outputFile << edge.attribute << endl;
    }

    inputFile.close();
    outputFile.close();

    return 0;
}
