#include <thread>
#include <mutex>
#include <vector>
#include "poly.h"
#include <iostream>

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

//poly + poly
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

//poly + scalar
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

//scalar + poly
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

// poly * poly without threads
// polynomial polynomial::operator*(const polynomial &other) const {
//     polynomial result;

//     for (const auto &val : terms) {
//         for (const auto &num : other.terms) {
//             power new_power = val.first + num.first;  
//             coeff new_coeff = val.second * num.second; 
//             result.terms[new_power] += new_coeff; 
//         }
//     }

//     result.simplify(); // Ensure canonical form
//     return result;
// }

//poly * poly using threads
// polynomial polynomial::operator*(const polynomial &other) const {
//     polynomial result;
//     vector<thread> threads;
//     std::mutex mx;

//     // Each thread will handle one term from 'this'
//     for (const auto &term : terms) {
//         threads.emplace_back([&, term]() {
//             polynomial local_result;
//             for (const auto &other_term : other.terms) {
//                 power pow = term.first + other_term.first;
//                 coeff cf = term.second * other_term.second;
//                 local_result.terms[pow] += cf;
//             }
//             mx.lock();
//             for (const auto &[power, coeff] : local_result.terms) {
//                 result.terms[power] += coeff;
//             }
//             mx.unlock();
//         });
//     }

//     // Join all threads
//     for (auto &t : threads) {
//         if (t.joinable()) {
//             t.join();
//         }
//     }

//     result.simplify();
//     return result;
// }

//this is all correct but its slow
//jordans slow version fr
// polynomial polynomial::operator*(const polynomial &other) const {
//     std::mutex mx; 
//     polynomial result;
//     vector<thread> threads;

//     //one thread per term of *this poly
//     //each thread multiplies the one term with the entire other poly
//     for(const auto & term : terms) {
//         threads.emplace_back([term, &other, &mx, &result]() {
//             polynomial localPoly;

//             for(const auto & otherTerm : other.terms) {
//                 power newPower = term.first + otherTerm.first;
//                 coeff newCoeff = term.second * otherTerm.second;
//                 localPoly.terms[newPower] += newCoeff;     
//             }

//             //update result
//             mx.lock();
//             for(const auto & term : localPoly.terms) {
//                 result.terms[term.first] += term.second;
//             }
//             mx.unlock();
//         });
//     }

//     //wait for all threads to finish
//     for(auto & thread : threads) {
//         thread.join();
//     }

//     result.simplify();
//     return (result);
// }

static std::mutex mx;

std::map<power, coeff> helper(std::map<power, coeff>::const_iterator start, std::map<power, coeff>::const_iterator end, const polynomial &other) {
    std::map<power, coeff> output;

    // std::cout << "bruh again" << std::endl; 
    for(auto it = start; it != end; ++it) {
        for(const auto &term : other.terms) {
            power newPower = it -> first + term.first;
            coeff newCoeff = it -> second * term.second;
            output[newPower] += newCoeff;
        }
    }

    return output; 
}

//jordans v2 fr
polynomial polynomial::operator*(const polynomial &other) const {
    polynomial result; 
    int num_of_threads = 8;
    int size = terms.size();
    int chunkSize = size / num_of_threads;
    auto it = terms.begin();
    std::mutex mx;
    std::vector<std::thread> threads;
    std::vector<map<power, coeff>> partials(num_of_threads);

    for(int i = 1; i <= num_of_threads; i++) {
        auto start = it;
        std::advance(it, chunkSize); //move it forward by chunksize
        auto end = (i != num_of_threads) ? it : terms.end(); //during the last chunk we dont want it...just want until the end

        threads.emplace_back([&partials, i, start, end, &other, &result, &mx] () {
            partials[i - 1] = helper(start, end, other);
            //merge the partials
            mx.lock();
            for(const auto & term : partials[i - 1]) {
                result.terms[term.first] += term.second;
            }
            mx.unlock();
        });
    }

    //wait for all threads to finish
    for(auto & thread : threads) {
        thread.join();
    }

    //merge all partials into terms
    // for(const auto &part : partials) {
    //     for(const auto & term : part) {
    //         result.terms[term.first] += term.second;
    //     }
    // }

    result.simplify();
    return result; 
}

// poly * poly with threads
// so this is super fast but for some reason the very last term is always wrong but the rest are all correct
// polynomial polynomial::operator*(const polynomial &other) const {
//     polynomial result;
//     std::mutex mx;

//     size_t num_threads = std::thread::hardware_concurrency();
//     if (num_threads == 0) {
//         num_threads = 4;
//     }
    
//     auto multiply_chunk = [&](auto start, auto end) { //lambdaaaa
//         std::map<power, coeff> local;

//     std::cout << "bruh but twice" << std::endl;
//         for (auto it = start; it != end; ++it) {
//             for (const auto &num : other.terms) {
//                 power pow = it->first + num.first;
//                 coeff cf = it->second * num.second;
//                 local[pow] += cf;
//             }
//         }
        
//         mx.lock();
//         for (const auto &[power, coeff] : local) {
//             result.terms[power] += coeff;
//         }
//         mx.unlock();
//     };

//     auto it = terms.begin();
//     size_t total_terms = terms.size();
//     size_t chunk_size = (total_terms + num_threads - 1) / num_threads;

//     vector<thread> threads;
//     for (size_t i = 0; i < num_threads && it != terms.end(); ++i) {
//         auto start = it;
//         advance(it, chunk_size);
//         threads.emplace_back(multiply_chunk, start, it);
//     }

//     for (auto &t : threads) {
//         if (t.joinable()) {
//             t.join();
//         }
//     }

//     result.simplify();
//     return result;
// }

//poly * scalar no threads
// polynomial polynomial::operator*(int scalar) const {
//     polynomial result = *this;

//     if (scalar == 1){
//         return *this;
//     }

//     for (auto &term : result.terms) {
//         term.second *= scalar; 
//     }

//     result.simplify(); 
//     return result;
// }

//scalar * poly without threads
// polynomial operator*(int scalar, const polynomial &poly) {
//      if (scalar == 1) {
//         return poly; 
//     }

//     polynomial result = poly;

//     for (auto &term : result.terms) {
//         term.second *= scalar; 
//     }

//     result.simplify(); 
//     return result;
// }

//poly * scalar with threads
polynomial polynomial::operator*(int scalar) const {
    if (scalar == 1) {
        return *this;
    }

    polynomial result = *this;

    if (result.terms.size() > 100) {
        size_t num_threads = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), result.terms.size());
        if (num_threads == 0) num_threads = 1;

        size_t chunk_size = (result.terms.size() + num_threads - 1) / num_threads;
        std::vector<std::thread> threads;

        auto it = result.terms.begin();
        for (size_t i = 0; i < num_threads; ++i) {
            auto start = it;
            for (size_t j = 0; j < chunk_size && it != result.terms.end(); ++j) {
                ++it;
            }
            auto end = it;

            threads.emplace_back([start, end, scalar]() {
                for (auto it = start; it != end; ++it) {
                    it->second *= scalar;
                }
            });
        }

        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else {
        for (auto &term : result.terms) {
            term.second *= scalar;
        }
    }

    result.simplify();
    return result;
}

//scalar * poly with threads
polynomial operator*(int scalar, const polynomial &poly) {
    if (scalar == 1) {
        return poly;
    }

    polynomial result = poly;

    if (result.terms.size() > 100) {
        size_t num_threads = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), result.terms.size());
        if (num_threads == 0) num_threads = 1;

        size_t chunk_size = (result.terms.size() + num_threads - 1) / num_threads;
        std::vector<std::thread> threads;

        auto it = result.terms.begin();
        for (size_t i = 0; i < num_threads; ++i) {
            auto start = it;
            for (size_t j = 0; j < chunk_size && it != result.terms.end(); ++j) {
                ++it;
            }
            auto end = it;

            threads.emplace_back([start, end, scalar]() {
                for (auto it = start; it != end; ++it) {
                    it->second *= scalar;
                }
            });
        }

        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else {
        for (auto &term : result.terms) {
            term.second *= scalar;
        }
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
    for (auto it = terms.rbegin(); it != terms.rend(); ++it) {
    // for (auto it = terms.rend(); it != terms.rbegin(); --it) {
        if (it -> second != 0) {
            result.emplace_back( it -> first, it -> second); //emplace is like push back but like constrcuts it in place 
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

// void polynomial::simplify() {
//     size_t num_terms = terms.size();
//     if (num_terms <= 100) { // Single-threaded small polynomials
//         for (auto it = terms.begin(); it != terms.end();) { // same thing as other one we made earlier
//             if (it->second == 0) {
//                 it = terms.erase(it);
//             } else {
//                 ++it;
//             }
//         }
//     } else { // Multithreaded  large polynomials
//         size_t num_threads = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), num_terms);
//         if (num_threads == 0) num_threads = 1; //need at least 1 thread

//         size_t chunk_size = (num_terms + num_threads - 1) / num_threads; // divides into even chunks//rounds up so nun left out 
        
        
//         std::vector<std::map<power, coeff>> local_results(num_threads); //stores thread results
//         std::vector<std::thread> threads; // stores threads

//         auto it = terms.begin();
//         for (size_t i = 0; i < num_threads; ++i) {
//             auto start = it;
//             for (size_t j = 0; j < chunk_size && it != terms.end(); ++j) {
//                 ++it;
//             }
//             auto end = it;  //finds end of chunk for thread

//             threads.emplace_back([&, i, start, end]() { // new thread and process chunk
//                 for (auto it = start; it != end; ++it) { // lambda iterates over uhnk if coeff not 0 stores in local results
//                     if (it->second != 0) {
//                         local_results[i][it->first] = it->second;
//                     }
//                 }
//             });
//         }

//         for (auto &t : threads) {
//             if (t.joinable()) {
//                 t.join();
//             }
//         }

//         terms.clear();
//         for (const auto &local_result : local_results) {
//             for (const auto &[power, coeff] : local_result) {
//                 terms[power] = coeff;
//             }
//         }

//         if (terms.empty()) {
//             terms[0] = 0;
//         }
//     }
// }

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

void polynomial::print() const {
    for(const auto &term : terms) {
        std::cout << "power : " << term.first << " || coeff : " << term.second << std::endl;
    }
}