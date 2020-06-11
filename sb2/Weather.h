//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include "fastcsv_type.h"
#include <vector>

struct WeatherDay {
    int station;
    int julianday;
    double meanTempC;
    double minTempC;
    double maxTempC;
    double relHum;
    double satDef;
    double precipMm;
    double meanWindKn;
};

class Weather {
private:
public:
    Weather(
        const std::string & infile, 
        const size_t nday
    ) : data(nday)  {
        csv_reader<9> in_csv(infile, "station","julianday","meanTempC","minTempC","maxTempC","relHum","satDef","precipMm","meanWindKn");
        // fill this
        for (size_t ii(0); ii<nday; ii++){
            auto & the_day = data[ii];
            in_csv.read_row(the_day.station,the_day.julianday,the_day.meanTempC,the_day.minTempC,the_day.maxTempC,the_day.relHum,the_day.satDef,the_day.precipMm,the_day.meanWindKn);
        }
    }
    const WeatherDay & operator[](const size_t iday) const { 
        return data[iday];
    };
private:
    std::vector<WeatherDay> data;
};
