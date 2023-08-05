#pragma once

#include "ContextNode.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdarg.h>
#include <cstdarg>

void bootstrap();
void finalize();
void trace_func_call(std::string funcName, int num_args, ...);
void trace_func_call_end();