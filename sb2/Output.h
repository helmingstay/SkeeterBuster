//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once

// err towards caution
#define MAX_PATH_LEN 1024

#include<cstring>
// test paths
#include <iostream>
#include <ostream>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include "minicsv.h"

// 
#include "Globals.h"
using std::string;
using std::fstream;
class Building;

typedef std::shared_ptr<csv::ofstream> ptr_csv ;

//! Filehandles of all output logs
struct outLogs {
    FILE* Output;
    fstream Short_Output;
    ptr_csv minimal_Output;
    ptr_csv compare_cont;
    ptr_csv compare_house;
    ptr_csv day_house;
    //FILE* minimal_Output;
    FILE* Output_per_Container;
    FILE* Output_per_Container_Type;
    FILE* Output_Water_Level;
    FILE* Output_per_House;
    FILE* sim_log;
    FILE* forSbeed;
    FILE* xt_adulticide;
    FILE* HouseSetupOut;
    ptr_csv sample_cont;
    ptr_csv sample_adult;
    ptr_csv csv_house;
    ptr_csv migrate;
};

void checkIsDir (string path, string error_msg);

class fopenDirFuns {
    public:
        //fopenDirFuns() ;
        void set(const string dir_in, const string dir_out){
            checkIsDir(dir_in, "## dir_in: " );
            checkIsDir(dir_out, "## dir_out: " );
            _inpath = dir_in;
            _outpath = dir_out;
        };
    private:
        string _inpath;
        string _outpath;
        // Number of run, used in file naming
        int ref;
        char filename[MAX_PATH_LEN]; // tmp string to copy
    public:
        // methods
        // initial reporting, also sets ref 
        void fill_sim_log(outLogs &logs);
        void set_ref(const int ref_) {
            ref = ref_;
        }
        ptr_csv OpenCsv(string fn, std::vector<string> header, const char fieldsep, const string & quotesep);
        void OpenLogFiles(outLogs &logs);
        void SaveHouseSetupOut(FILE * out, vector<Building> & House);
        // writer functions
        void write_csv_house(ptr_csv pcsv, int date, std::vector<Building> & houses);
            
    
        //! construct filename w/sprintf from ref and extension
        FILE* outref( const char* basename, const char* exten=".txt", const char* mode="w") {
            snprintf(filename, MAX_PATH_LEN, "%s/%s%05d%s", _outpath.c_str(), basename, ref, exten);
            return fopen(filename, mode);
        }
        // as above, but for string basename
        FILE* outref( const string basename, const char* exten=".txt", const char* mode="w") {
            snprintf(filename, MAX_PATH_LEN, "%s/%s%05d%s", _outpath.c_str(), basename.c_str(), ref, exten);
            return fopen(filename, mode);
        }
        //! construct filename from output dir
        FILE* out( const char* fullname,  const char* mode) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _outpath.c_str(), fullname);
            return fopen(filename, mode);
        }
        // as above, for string input
        FILE* out( string fullname,  const char* mode) {
            string name = _outpath + "/" + fullname;
            return fopen(name.c_str(), mode);
        }
        //! construct filename from input dir
        FILE* in( const char* fullname,  const char* mode) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _inpath.c_str(), fullname);
            return fopen(filename, mode);
        }
        FILE* in( const string fullname,  const char* mode) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _inpath.c_str(), fullname.c_str());
            return fopen(filename, mode);
        }
        // returning char* is a world of trouble.
        // instead return string of fullpath, use ret.c_str() to get char* as needed
        // same behavior for overloads by in char* or string
        string inpath(const char* fullname) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _inpath.c_str(), fullname);
            return string(filename);
        }
        string inpath(const string fullname) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _inpath.c_str(), fullname.c_str());
            return string(filename);
        }
        string outpath(const char* fullname) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _outpath.c_str(), fullname);
            return string(filename);
        }
        string outpath(const string fullname) {
            snprintf(filename, MAX_PATH_LEN, "%s/%s", _outpath.c_str(), fullname.c_str());
            return string(filename);
        }
};


// convenience struct for daily summaries
// (instead of globals/extern)
// These are written by BuildingSummation
struct TotalStruct {
    vector<unsigned> Genotypes;
    vector<unsigned> Adult_Genotypes;
    vector<double> wolbachia_frequency;
    vector<double> allele_frequency;
    // apparently unused
    // vecVecDouble allele_frequencies;
    //
    // this initializes the counting per container type
    //?? 11 = nmeasures, 200=n ctypes
    // kinda crazy, =11, stores different measures
    unsigned n_cont_field;
    // 200
    unsigned n_cont_type;
    // fixed length
    vector< vector<double> > Census_by_container_type;
    // end of container-specific
    size_t Total_Mosquitoes;
    bool has_insects;
    double CV_Pupae;
    unsigned Eggs;
    unsigned EggsLaid;
    unsigned Male_Larvae;
    unsigned Female_Larvae;
    unsigned Male_Pupae;
    unsigned Female_Pupae;
    unsigned Nulliparous_Female_Adults;
    unsigned Parous_Female_Adults;
    unsigned Male_Adults;
    double CV_WILD_FEMALES;
    double CV_WILD_ALLELES;
    double NO_WILD_FEMALES;
    double NO_WILD_ALLELES;
    // container census
    /// first dimension : no. containers, no.eggs, no. larvae, no.pupae, no. pupae-positive containers, average pupae weight, water level, average food amount
    /// second dimension : by container type
    //!! MAGIC NUMBERS
    TotalStruct(
        unsigned n_genotype, unsigned n_wolbachia, unsigned n_chrom
    ) :
        // containers
        Genotypes(n_genotype, 0),
        Adult_Genotypes(n_genotype, 0),
        wolbachia_frequency(n_genotype, 0.0),
        allele_frequency(2*n_chrom, 0.0),
        // MAGIC NUMBERS!
        // field = container measure
        n_cont_field(11),
        // arbitrary upper bound on number of container types
        n_cont_type(200),
        // end containers, zeros from here down
        Total_Mosquitoes(0),
        has_insects(false),
        CV_Pupae(0), 
        Eggs(0),
        EggsLaid(0),
        Male_Larvae(0), Female_Larvae(0),
        Male_Pupae(0), Female_Pupae(0),
        Nulliparous_Female_Adults(0),
        Parous_Female_Adults(0),
        Male_Adults(0),
        CV_WILD_FEMALES(0.0),
        CV_WILD_ALLELES(0.0),
        NO_WILD_FEMALES(0.0),
        NO_WILD_ALLELES(0.0)
    {
        vector< double > vec_temp(n_cont_type, 0.0);
        Census_by_container_type = vector< vector < double > > ( n_cont_field, vec_temp);
    }
};


// daily summary 
void DayEndReport(
    int Date,
    fopenDirFuns & fopenF, 
    outLogs & logs, 
    std::vector<Building> & House,
    TotalStruct & Totals
);


template <typename T>
void printer(std::ostream & _out, const char* sep, T arg) {
    _out << arg;
}

// variadic fstream printing
template <typename T, typename ...Ts>
void printer(std::ostream & _out, const char* sep,  T arg, Ts ...args) {
    _out << arg << sep;
    printer(_out, sep, args...);
}
