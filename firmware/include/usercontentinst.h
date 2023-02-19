#pragma once


#if __has_include("usercontent.h") 
#include "usercontent.h"              // Custom user content

extern UserContentClass UserContentInst;
#else
#include "usercontentbase.h"          // Base user content

extern usercontentbase UserContentInst;
#endif