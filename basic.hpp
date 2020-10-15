#ifndef BASIC_HPP
#define BASIC_HPP

#include <iostream>
//__GNUG__ g++
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define force_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define force_inline __forceinline
#endif


template<class A>
std::string str(const A& input){
    return std::to_string(input);
}

std::string str(std::string input){
    return input;
}

std::string str(bool input){
    if(input){
        return "true";
    }
    return "false";
}

std::string str(const char* input){
    return input;
}
void print(){
    std::cout << "\n";
}

template<class A,class... Args>
void print(const A& first,Args const&... args){
    std::cout << str(first) << " ";
    print(args...);
}

#include <typeinfo>

///*********************begin***********
//parts of following code for types is from
//https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname/4541470

std::string demangle(const char* name);

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

#else

// does nothing if not g++
std::string demangle(const char* name) {
    return name;
}

#endif
///*********************end***********


/**
 * @param type variable
 * @return std::string
*/
#define type( t ) demangle(typeid(t).name())

#endif
