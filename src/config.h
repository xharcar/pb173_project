#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>

#define BIN_DIR                "../bin"
#define WORLD_BIN      BIN_DIR "/" "world"
#define TANK_BIN       BIN_DIR "/" "tank"
#define TANKCLIENT_BIN BIN_DIR "/" "tankclent"

// static constexpr const char* BIN_DIR = "../bin";
// static constexpr const char* WORLD_BIN      = BIN_DIR "/" "world";
// static constexpr const char* TANK_BIN       = BIN_DIR "/" "tank";
// static constexpr const char* TANKCLIENT_BIN = BIN_DIR "/" "tankclent";

