#ifndef EVAL_H
#define EVAL_H

#include "TreeAnalyzer.h"

#include "TMVA/Reader.h"
#include "TMVA/Config.h"
#include "TMVA/Tools.h"
#include "TMVA/MethodCuts.h"

#include "TNtuple.h"
#include "TTree.h"


#include <memory>

class Eval : public TreeAnalyzer
{
public:
	Eval() {}
	~Eval() {}

protected:

	shared_ptr<TMVA::Reader> reader;
	float pt1, pid1, pt2, pid2;
	float pairPid, classId;

	TNtuple *ntuple;
	// MVA name
	string name;

	void loadNN( string _weight_file, string _name = "MLP" ){
		LOG_SCOPE_FUNCTION(INFO);
		this->name=_name;
		LOG_F( INFO, "Loading MVA weights from : %s", _weight_file.c_str() );

		if ( nullptr == reader ){
			reader = shared_ptr<TMVA::Reader>(new TMVA::Reader( "!Color:!Silent" ) ); 

			reader->AddVariable( "pt1", &pt1 );
			reader->AddVariable( "pid1", &pid1 );
			reader->AddVariable( "pt2", &pt2 );
			reader->AddVariable( "pid2", &pid2 );
		}
		reader->BookMVA( this->name.c_str(), _weight_file.c_str() ); 
	} // loadNN

	virtual void initialize(){
		LOG_SCOPE_FUNCTION(INFO);
		TreeAnalyzer::initialize();

		LOG_F( INFO, "book: %p", book.get() );
		book->cd();
		LOG_F( INFO, "creating output ntuple" );
		ntuple = new TNtuple( "PairNN", "Pair with NN response", "pt1:pid1:pt2:pid2:pairPid:classId" );

		chain->SetBranchAddress( "pt1", &pt1 );
		chain->SetBranchAddress( "pid1", &pid1 );
		chain->SetBranchAddress( "pt2", &pt2 );
		chain->SetBranchAddress( "pid2", &pid2 );

		classId = config.get<float>( nodePath + ":classId" );
		LOG_F( INFO, "ClassId = %f", classId );


		loadNN( "/home/jdb12/work/dimuonAna/PairNN/bin/weights/TMVAClassification_MLP_.weights.xml" );
	}


	virtual void analyzeEvent(){

		pairPid = reader->EvaluateMVA( this->name.c_str() );

		ntuple->Fill( pt1, pid1, pt2, pid2, pairPid, classId );
	}
	
};


#endif