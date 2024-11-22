#include "poly.h"
using namespace std;

polynomial::polynomial() {
    terms.insert(pair<int, int>(0, 0));
}

template <typename Iter>
polynomial::polynomial(Iter begin, Iter end) {
    for (auto it = begin; it != end; ++it) {
        if (it->second != 0) { 
            terms.insert(pair<int, int>(it.first, it.second));
        }
    }
    simplify(); 
}

polynomial::polynomial(const polynomial &other) : terms(other.terms) {}

polynomial &polynomial::operator=(const polynomial &other) {
    if (this != &other) {         
        terms = other.terms; 
    }
    return *this; 
}

polynomial polynomial::operator+(const polynomial &other)  {
    polynomial result = *this; 

    for (const auto & [power, coeff] : other.terms) {
        auto currValues = result.terms.find(power);

        if(terms.count(power)) {
            // term already exists in *this
            terms[power] += coeff;

            if(terms[power] == 0) {
                terms.erase(power);
            }
        } else {
            terms[power] = coeff; // new term 
        }
    }

    result.simplify(); 
    return result;
}

polynomial polynomial::operator+(int scalar)  {
    polynomial result = *this; 

    if (scalar == 0) {
        return result;
    }

    if(terms.count(0)) {
        terms[0] += scalar;

        if(terms[0] == 0) {
            terms.erase(0);
        }
    } else {
        terms[0] = scalar;
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

polynomial polynomial::operator*(const polynomial &other)  {
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

polynomial polynomial::operator*(int scalar) {
    polynomial result = *this;

    for (auto &term : result.terms) {
        term.second *= scalar; 
    }

    result.simplify(); 
    return result;
}

polynomial operator*(int scalar, const polynomial &poly) {
    return poly * scalar; 
}

size_t polynomial::find_degree_of() {
    if (terms.empty()) {
        return 0; 
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
    sort(result.rbegin(), result.rend());
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

polynomial polynomial::operator%(const polynomial &divisor) const {
    if (divisor.terms.empty() || divisor.terms.rbegin()->second == 0) {
        throw std::invalid_argument("Division by zero polynomial");
    }

    polynomial remainder = *this;

    while (remainder.find_degree_of() >= divisor.find_degree_of()) {
        power degree_diff = remainder.find_degree_of() - divisor.find_degree_of();
        coeff coeff_ratio = remainder.terms.rbegin()->second / divisor.terms.rbegin()->second;

        polynomial term({{degree_diff, coeff_ratio}});
        remainder = remainder - (term * divisor);
    }

    return remainder;
}
