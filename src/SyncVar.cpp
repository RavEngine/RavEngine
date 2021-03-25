#include "SyncVar.hpp"

using namespace RavEngine;

SyncVar_base::queue_t SyncVar_base::queue_A, SyncVar_base::queue_B;

std::atomic<SyncVar_base::queue_t*> SyncVar_base::writingptr = &queue_A, SyncVar_base::readingptr = &queue_B;

decltype(SyncVar_base::all_syncvars) SyncVar_base::all_syncvars;
