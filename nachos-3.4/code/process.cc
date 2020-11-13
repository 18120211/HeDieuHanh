#include "process.h"

Process::Process(Thread * myExecutor, Process * myParent){
    parent = myParent;
    container = myExecutor;
}



