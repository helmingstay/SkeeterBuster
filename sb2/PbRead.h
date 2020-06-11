//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

//! Read a protobuf message of arbitrary message type from a file
// For message details see "proto/PbInput.pb.h"
template <typename Type>
void PbRead(Type &msg, const char *filename){
    std::fstream infile(filename, std::ios::in | std::ios::binary);
    if (!infile) {
       throw std::runtime_error("Setup message file not found");
    } else if (!msg.ParseFromIstream(&infile)) {
       throw std::runtime_error("Parse error in message file");
    }
}
