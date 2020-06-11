//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#include <cstdio>
#include <vector>

struct outLogs;
class fopenDirFun;
class Building;

void OpenLogFiles(outLogs &logs, int ref, fopenDirFuns & fopenF);
int Get_Number(outLogs &logs, fopenDirFuns & fopenF);
void SaveHouseSetupOut(FILE * HouseSetupOut, int ref, std::vector<Building> & House);
