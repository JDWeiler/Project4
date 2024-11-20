#include "poly.h"

polynomial::polynomial() {
    terms[0] = 0; 
}

template <typename Iter>
polynomial::polynomial(Iter begin, Iter end) {
    for (auto it = begin; it != end; ++it) {
        if (it->second != 0) { 
            terms[it->first] += it->second; 
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
    for (const auto &term : other.terms) {
        result.terms[term.first] += term.second; 
    }
    result.simplify(); 
    return result;
}

polynomial polynomial::operator+(int scalar)  {
    polynomial result = *this; 
    result.terms[0] += scalar; 
    result.simplify(); 
    return result;
}

polynomial operator+(int scalar, const polynomial &poly) {
    return poly + scalar; 
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
