//
// Created by Luis Gonzalez on 3/23/20.
//

#include "utils.h"
#include <chrono>

namespace Odo {
    bool has_init_generator = false;
    typedef std::chrono::high_resolution_clock myclock;
    std::default_random_engine generator;

    bool contains_type(std::vector<Lexing::TokenType> arr, Lexing::TokenType t) {
        return std::find(arr.begin(), arr.end(), t) != arr.end();
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool starts_with(std::string const &fullString, std::string const &beginning) {
        if (fullString.length() >= beginning.length()) {
            return (0 == fullString.compare (0, beginning.length(), beginning));
        } else {
            return false;
        }
    }

    int rand_int(int min, int max) {
        if (!has_init_generator) {
            myclock::time_point beginning = myclock::now();
            auto time_since = beginning.time_since_epoch();
            auto in_millis = std::chrono::duration_cast<std::chrono::nanoseconds>(time_since).count();
            generator = std::default_random_engine(in_millis);
            has_init_generator = true;
        }
        std::uniform_int_distribution<int> distribution(min, max-1);
        return distribution(generator);
    }

    double rand_double(double min, double max) {
        if (!has_init_generator) {
            myclock::time_point beginning = myclock::now();
            auto time_since = beginning.time_since_epoch();
            auto in_millis = std::chrono::duration_cast<std::chrono::nanoseconds>(time_since).count();
            generator = std::default_random_engine(in_millis);
            has_init_generator = true;
        }
        std::uniform_real_distribution<double> distribution(min, max);
        return distribution(generator);
    }

    double rand_double(double max) { return rand_double(0, max); }
}