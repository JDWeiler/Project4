#include "poly.h"
using namespace std;

polynomial::polynomial() {
    terms.insert(pair<int, int>(0, 0));
}


polynomial::polynomial(const polynomial &other) : terms(other.terms) {}

polynomial &polynomial::operator=(const polynomial &other) {
    if (this != &other) {         
        terms = other.terms; 
    }
    return *this; 
}


polynomial polynomial::operator+(const polynomial &other) const {
    polynomial result = *this;

    for (const auto &[power, coeff] : other.terms) {
        auto it = result.terms.find(power);

        if (it != result.terms.end()) {
            // Term exists in result
            it->second += coeff;

            // Remove term if coefficient becomes 0
            if (it->second == 0) {
                result.terms.erase(it);
            }
        } else {
            // Add new term
            result.terms[power] = coeff;
        }
    }

    result.simplify();
    return result;
}

polynomial polynomial::operator+(int scalar) const {
    polynomial result = *this; 

    if (scalar == 0) {
        return result;
    }

    if(result.terms.count(0)) {
        result.terms[0] += scalar;

        if(result.terms[0] == 0) {
            result.terms.erase(0);
        }
    } else {
        result.terms[0] = scalar;
    }

    result.simplify(); 
    return result;
}

polynomial operator+(int scalar, const polynomial &poly) {
    polynomial result = poly;
    
    if(scalar == 0) {
        return result;
    } 

    if(result.terms.count(0)) {
        result.terms[0] += scalar;

        if(result.terms[0] == 0) {
            result.terms.erase(0);
        }
    } else {
        result.terms[0] = scalar;
    }

    result.simplify();
    return result;
}

polynomial polynomial::operator*(const polynomial &other) const {
    polynomial result;

    for (const auto &val : terms) {
        for (const auto &num : other.terms) {
            power new_power = val.first + num.first;  
            coeff new_coeff = val.second * num.second; 
            result.terms[new_power] += new_coeff; 
        }
    }

    result.simplify(); // Ensure canonical form
    return result;
}

polynomial polynomial::operator*(int scalar) const {
    polynomial result = *this;

    if (scalar == 1){
        return *this;
    }

    for (auto &term : result.terms) {
        term.second *= scalar; 
    }

    result.simplify(); 
    return result;
}

polynomial operator*(int scalar, const polynomial &poly) {
     if (scalar == 1) {
        return poly; 
    }

    polynomial result = poly;

    for (auto &term : result.terms) {
        term.second *= scalar; 
    }

    result.simplify(); 
    return result;
}

size_t polynomial::find_degree_of() const {
    if (terms.empty()) {
        return 0; //should it be 0?
    }
    return terms.rbegin()->first; 
}

std::vector<std::pair<power, coeff>> polynomial::canonical_form() const {
    std::vector<std::pair<power, coeff>> result;
    for (auto &term : terms) {
        if (term.second != 0) {
            result.emplace_back(term.first, term.second); //emplace is like push back but like constrcuts it in place 
        }
    }

    if (result.empty()) {
        result.emplace_back(0, 0);
    }
    return result;
}

bool comparePoly(std::pair<power, coeff> pair1, std::pair<power, coeff> pair2) {
    return(pair1.first < pair2.first);
}

void polynomial::simplify() {
    for (auto it = terms.begin(); it != terms.end();) {
        if (it->second == 0) {
            it = terms.erase(it); 
        } else {
            ++it;
        }
    }
    if (terms.empty()) {
        terms[0] = 0; 
    }
}

polynomial polynomial::operator-(const polynomial &other) const {
    polynomial result = *this; 

    for (const auto &[power, coeff] : other.terms) {
        result.terms[power] -= coeff;

        if (result.terms[power] == 0) {
            result.terms.erase(power);
        }
    }

    return result;
}

polynomial polynomial::operator%(const polynomial &divisor) const {
     if (divisor.terms.empty()) {
        polynomial x;
        x.terms[0] = 0;
        return x;
    }

    polynomial remainder = *this; 
    polynomial quotient;

    while (!remainder.terms.empty() && remainder.find_degree_of() >= divisor.find_degree_of()) {
        
        power degree = remainder.find_degree_of() - divisor.find_degree_of();
        coeff leading = remainder.terms.rbegin()->second / divisor.terms.rbegin()->second;

        polynomial term;
        term.terms[degree] = leading; // term = (lead)x^(degree)

        remainder = remainder - (term * divisor);
        remainder.simplify();
    }

    return remainder;
}
