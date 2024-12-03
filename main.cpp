#include <iostream>
#include <chrono>
// #include <optional>
#include <string>
#include <utility> // for std::pair
#include <fstream>
#include <sstream>
#include "poly.h"

//from chat
std::vector<std::pair<power, coeff>> parseConfigFile(const std::string& filename) {
    std::vector<std::pair<power, coeff>> terms;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return terms; // return empty vector on error
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        coeff c;
        power p;
        char x, caret;  // for 'x' and '^'

        // Parse the coefficient and the power
        if (ss >> c >> x >> caret >> p) {
            terms.emplace_back(p, c);
        } else {
            std::cerr << "Invalid line format: " << line << std::endl;
        }
    }

    file.close();
    std::cout << "Open successful" << std::endl;
    return terms;
}

void writeConfigFile(const std::map<power, coeff>& terms, const std::string& filename) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Write each term in the configuration format: coeff x^power
    for (const auto& term : terms) {
        file << term.second << "x^" << term.first << std::endl;
    }

    file.close();
    std::cout << "File written successfully: " << filename << std::endl;
}

void writeConfigFileVector(const std::vector<std::pair<power, coeff>>& terms, const std::string& filename) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Write each term in the configuration format: coeff x^power
    for (const auto& term : terms) {
        file << term.second << "x^" << term.first << std::endl;
    }

    file.close();
    std::cout << "File written successfully: " << filename << std::endl;
}

std::optional<double> poly_test(polynomial& p1,
                                polynomial& p2,
                                std::vector<std::pair<power, coeff>> solution)

{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    polynomial p3 = p1 * p2;
    std::cout << "bruh" << std::endl;

    auto p3_can_form = p3.canonical_form();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // p3.print();
    // std::vector<std::pair<power, coeff>> poly = p3.terms;

    // writeConfigFile(p3.getTerms(), "output.txt");
    writeConfigFileVector(p3_can_form, "output.txt");
    // if (p3_can_form != solution)
    // {
    //     return std::nullopt;
    // }

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

int main()
{
    /** We're doing (x+1)^2, so solution is x^2 + 2x + 1*/
    // std::vector<std::pair<power, coeff>> solution = {{2,1}, {1,2}, {0,1}};

    /** This holds (x+1), which we'll pass to each polynomial */
    std::vector<std::pair<power, coeff>> poly1 = parseConfigFile("poly1.txt");
    std::vector<std::pair<power, coeff>> poly2 = parseConfigFile("poly2.txt");
    std::vector<std::pair<power, coeff>> solution = parseConfigFile("result.txt");

    //  printer for polys to make sure the file reading worked ( it should be good )
    // for(const auto &term : solution) {
    //     std::cout << "power : " << term.first << " || coeff : " << term.second << std::endl;
    // }
    
    polynomial p1(poly1.begin(), poly1.end());
    polynomial p2(poly2.begin(), poly2.end());

    std::optional<double> result = poly_test(p1, p2, solution);

    if (result.has_value())
    {
        std::cout << "Passed test, took " << result.value()/1000 << " seconds" << std::endl;
    } 
    else 
    {
        std::cout << "Failed test" << std::endl;
    }
}