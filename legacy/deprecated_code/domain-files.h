#ifndef __DOMAIN_FILES_H_INCLUDED__
#define __DOMAIN_FILES_H_INCLUDED__

#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "tsl/ordered_map.h"


template <size_t n>
using Domain = std::array<size_t, n>;

// BOG CODE
#include <unordered_map>

// https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys/42701876
template <size_t n>
struct ArrayHasher {
    std::size_t operator()(const std::array<int, n>& a) const {
        std::size_t h = 0;

        for (auto e : a) {
            h ^= std::hash<int>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2); 
        }
        return h;
    }   
};

template <size_t n>
using LookupTable = std::unordered_map<std::array<int, n>,int,ArrayHasher<n>>;

// ENDBOG

template <size_t n>
using DomainNames = std::array<std::string, n>;

template <size_t n>
using PopulatingFunction = std::function<long long(Domain<n>)>;

template <size_t n>
struct DomainCmp {
    bool operator()(const Domain<n>& d1, const Domain<n>& d2) const {
        // for (size_t i=0; i<n; i++) {
        //     if (d1[i] < d2[i])
        //         return true;
        // }
        // return false;
        // return true;
        if (d1[0] < d2[0]) {//different priority to order the map
            return true;
        } else if (d1[0] == d2[0]) {
            if (d1[1] < d2[1]) {
                return true;
            }else if (d1[1] == d2[1]) {
                if (d1[2] <= d2[2]) {//preserve equal value
                return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }

    }
};

template <size_t n>
using DomainMap = std::map<Domain<n>, std::vector<long long>, DomainCmp<n>>;  //DomainMap
// using DomainMap = std::map<Domain<n>, std::vector<long long>>;  //DomainMap

template <size_t n>
class DomainFile {
public:
    DomainFile(DomainNames<n> names);
    DomainFile(std::string filename);
    //~DomainFile();
    void add_column(std::string name);
    void add_columns(std::vector<std::string> names);
    void add_key(Domain<n> d);
    void set_value(Domain<n> d, std::string col, long long val);
    void output(std::string filename);
    void output(std::ostream &os = std::cout);

    void populate_column(std::string name, PopulatingFunction<n> f, bool overwrite = false);
    void populate_columns(std::vector<std::pair<std::string, PopulatingFunction<n>>> column_ops, bool overwrite = false);
    
    void populate_some_column(std::string name, PopulatingFunction<n> f, std::vector<Domain<n>> row_keys,bool overwrite = false);
    void populate_some_columns(std::vector<std::pair<std::string, PopulatingFunction<n>>> column_ops, std::vector<Domain<n>> row_keys,bool overwrite = false);

    // BOG CODE
    LookupTable<n> construct_lookup_table(int max_cores);
    // ENDBOG
    std::vector<Domain<n>> DomainVector;

private:
    int get_column_ix(std::string name);
    void read_file(std::string filename);
    DomainNames<n> dnames;
    std::vector<std::string> col_names;
    DomainMap<n> rows;

};
// BOG CODE
inline int ilog(int x) {
    int ret = 0;
    while (x > 1) {
        x /= 2;
        ret++;
    }
    return ret;
}

inline int iexp(int x) {
    int ret = 1;
    for (int i=x; i>0; i++) {
        ret *= 2;
    }
    return ret;
}

template <size_t n>
inline LookupTable<n> DomainFile<n>::construct_lookup_table(int max_cores) {
    if (max_cores > col_names.size())
        max_cores = col_names.size();

    LookupTable<n> table;
    for (auto row : rows) {
        std::array<int, n> key;//key
        for (int i=0; i<n; i++) {
            key[i] = ilog(row.first[i]);
        }

        auto times = row.second;
        int best_ncores = 1;
        for (int i=0; i<max_cores; i++) {
            if (times[i] < times[best_ncores-1])
                best_ncores = i+1;
        }

        if (table.count(key) == 0)//no repeat
            table.insert({key,best_ncores});//insert
    }

    return table;
}
// ENDBOG

template <size_t n>
inline DomainFile<n>::DomainFile(DomainNames<n> names) {
    dnames = names;
}

template <size_t n>
inline DomainFile<n>::DomainFile(std::string filename) {//reorder probably here
    std::ifstream file(filename);
    std::string s, t;

    // Read header
    getline(file, s);
    std::istringstream ss(s);
    for (int i=0; i<n; i++) {
        std::getline(ss, dnames[i], ',');
    }

    while (std::getline(ss, t, ',')) {
        col_names.push_back(t);
    }

    while (std::getline(file, s)) {// dom and val
        std::istringstream ss(s);
        Domain<n> dom;// n equals 3 for matrix
        for (int i=0; i<n; i++) {
            std::getline(ss, t, ',');
            dom[i] = std::stoi(t);
        }
        DomainVector.push_back(dom);//push back each domain

        std::vector<long long> val;
        while (std::getline(ss, t, ','))
            val.push_back(std::stoi(t));

        rows.insert({dom, val});//insert
    }
    file.close();//original values have been read in

}

//template <size_t n>
//DomainFile<n>::~DomainFile() {
//    
//}

template <size_t n>
inline void DomainFile<n>::add_key(Domain<n> d) {
    std::pair<Domain<n>, std::vector<long long>> p;
    p.first = d;
    rows.emplace(p);
}

template <size_t n>
inline void DomainFile<n>::add_column(std::string name) {
    if (std::find(col_names.begin(), col_names.end(), name) != col_names.end())
        return;

    col_names.push_back(name);
    for (auto &val : rows) {
        val.second.push_back(-1);
    }
}

template <size_t n>
inline void DomainFile<n>::add_columns(std::vector<std::string> names) {
    for (std::string name : names)
        this->add_column(name);
}

template <size_t n>
inline int DomainFile<n>::get_column_ix(std::string name) {
    for (int ix = 0; ix < col_names.size(); ix++) {
        if (col_names[ix] == name)
            return ix;
    }
    return -1;
}

template <size_t n>
inline void DomainFile<n>::set_value(Domain<n> d, std::string col, long long val) {
    auto itr = rows.find(d);
    if (itr == rows.end())
        return;

    int ix = this->get_column_ix(col);
    if (ix == -1)
        return;

    for (int i=itr->second.size(); i<=ix; i++) {
        itr->second.push_back(-1);
    }

    itr->second[ix] = val;
}

template <size_t n>
inline void DomainFile<n>::output(std::string filename) {
    std::ofstream out(filename);
    this->output(out);
    out.close();
}

template <size_t n>
inline void DomainFile<n>::output(std::ostream &ss) {
//std::string DomainFile<n>::to_string() {
    //std::stringstream ss;
    const char *sep = "";

    for (std::string name : dnames) {
        ss << sep << name;
        sep = ",";
    }
    for (std::string col : col_names) {
        ss << sep << col;
    }
    ss << std::endl;

    for (auto value : rows) {
        sep = "";
        for (size_t x : value.first) { //output domain size
            ss << sep << x;
            sep = ",";
        }

        for (long long y : value.second) //output domain time
            ss << sep << y;

        ss << std::endl;
    }
    //return ss.str();
}

template <size_t n>
inline void DomainFile<n>::populate_column(std::string name, PopulatingFunction<n> f, bool overwrite) {
    populate_columns({{name, f}}, overwrite);
}

// Can avoid reallocating arrays if we use closures. Don't do this 
// until we're sure it'll be faster, though
template <size_t n>
inline void DomainFile<n>::populate_columns(std::vector<std::pair<std::string, PopulatingFunction<n>>> column_ops, bool overwrite) {
    for (auto &val : rows) {
        for (auto column_op : column_ops) {
            int ix = get_column_ix(column_op.first);//first is name, second is function
            if (overwrite || val.second[ix] == -1)
                val.second[ix] = column_op.second(val.first);

        }
    }
}

template <size_t n>
inline void DomainFile<n>::populate_some_column(std::string name, PopulatingFunction<n> f,std::vector<Domain<n>> row_keys, bool overwrite) {
    populate_some_columns({{name, f}},  row_keys, overwrite);
}

//Populate row in a vector contatining keys
template <size_t n>
inline void DomainFile<n>::populate_some_columns(std::vector<std::pair<std::string, PopulatingFunction<n>>> column_ops, std::vector<Domain<n>> row_keys,bool overwrite) {
    for (auto row_key : row_keys) {
        for (auto &val : rows) {
            std::cout <<val.first[0]<<std::endl;
            std::cout <<val.first[1]<<std::endl;
            std::cout <<val.first[2]<<std::endl;

        }

        if(rows.count(row_key)>0) {
            auto val = rows[row_key]; //map[key] should have a key exist check

            for (auto column_op : column_ops) {
                int ix = get_column_ix(column_op.first);//first is name, second is function
                if (overwrite || val[ix] == -1)
                    std::cout <<val[0]<<std::endl;
            }

        } else {
            std::cout <<"Col not found"<<std::endl;
            return;
        }

    }
}

#endif