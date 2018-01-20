

// RooBarb
#include "XmlConfig.h"
#include "TaskEngine.h"
using namespace jdb;

// STL
#include <iostream>
#include <exception>


#include "ANN/PairTreeMaker.h"
#include "ANN/TrainNN.h"
#include "ANN/Eval.h"

#define LOGURU_IMPLEMENTATION 1
#include "vendor/loguru.h"

int main( int argc, char* argv[] ) {
	// loguru::add_file("everything.log", loguru::Truncate, loguru::Verbosity_MAX);
	
	TaskFactory::registerTaskRunner<PairTreeMaker>( "PairTreeMaker" );
	TaskFactory::registerTaskRunner<TrainNN>( "TrainNN" );
	TaskFactory::registerTaskRunner<Eval>( "Eval" );

	TaskEngine engine( argc, argv );

	return 0;
}
