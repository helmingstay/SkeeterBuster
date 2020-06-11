--    Copyright 2015-2020 Christian Gunning (code@x14n.org)
--    This file is part of Skeeter Buster.

CREATE TABLE "sql_weather" (
  "station" INTEGER NULL,
  "date" TEXT NULL,
  "julianday" REAL NULL,
  "meanTempC" REAL NULL,
  "minTempC" REAL NULL,
  "maxTempC" REAL NULL,
  "dewC" REAL NULL,
  "relHum" REAL NULL,
  "satDef" REAL NULL,
  "precipMm" REAL NULL,
  "meanWindKn" REAL NULL,
  "disable_leap_years" INTEGER NULL
);
