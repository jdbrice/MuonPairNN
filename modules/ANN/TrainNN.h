#ifndef TRAIN_NN_H
#define TRAIN_NN_H


#include "HistoAnalyzer.h"
#include "XmlRange.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"

#include "vendor/loguru.h"

#include "TNamed.h"


template <>
TString XmlConfig::get<TString>( string path ) const {
	TString r( getString( path ) );
	return r;
}

template <>
TString XmlConfig::get<TString>( string path, TString dv ) const {
	if ( !exists( path ) )
		return dv;
	TString r( getString( path ) );
	return r;
}

class TrainNN : public HistoAnalyzer
{
public:

	const int DEBUG = 1;
	TrainNN() {}
	~TrainNN() {}

	virtual void initialize(){
		HistoAnalyzer::initialize();
	}
protected:

	virtual void make() {

		TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", book->getOutputFile(), "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification" );

		TTree * signal_tree = get<TTree>( "sigPairs" );
		TTree * bg_tree = get<TTree>( "bgPairs" );
		TTree * mixed_tree = get<TTree>( "mixedPairs" );

		LOG_F( INFO, "signal tree = %p", signal_tree );
		LOG_F( INFO, "bg tree = %p", bg_tree );
		LOG_F( INFO, "mixed tree = %p", mixed_tree );


		// MLP
		factory->AddVariable( "pt1", "pT", "(GeV/c)", 'F' );
		factory->AddVariable( "pid1", "", "", 'F' );
		factory->AddVariable( "pt2", "pT", "(GeV/c)", 'F' );
		factory->AddVariable( "pid2", "", "", 'F' );



		Double_t signalWeight     = 1.0;
   		Double_t backgroundWeight = 1.0;
		
		factory->AddSignalTree    ( signal_tree,     signalWeight     );
   		factory->AddBackgroundTree( bg_tree, backgroundWeight );
   		factory->AddBackgroundTree( mixed_tree, backgroundWeight );

   		LOG_F( INFO, "CUTS: %s", config.getString( "Prepare:cuts" ).c_str()  );

		TCut precuts = TCut( config.get<TString>( "Prepare:cuts", "" ) );
		TString preopts = config.get<TString>( "Prepare.opts" );
		
		LOG_F( INFO, "Prepare(\"%s\")", preopts.Data() );
		LOG_F( INFO, "Pre Cuts(\"%s\")", config.get<TString>( "Prepare:cuts", "" ).Data() );
		factory->PrepareTrainingAndTestTree( precuts, preopts );

		LOG_F( INFO, "MLP(\"%s\") ", config.get<TString>( "Methods.MLP:opts" ).Data() );
		factory->BookMethod( TMVA::Types::kMLP, "MLP_" + config.getString( "mod" ), config.get<TString>( "Methods.MLP:opts" ) );

		// Train MVAs using the set of training events
		factory->TrainAllMethods();

		// ---- Evaluate all MVAs using the set of test events
		factory->TestAllMethods();

		// ----- Evaluate and compare performance of all configured MVAs
		factory->EvaluateAllMethods();



		TNamed config_str( "config", config.toXml() );
		config_str.Write();

	}

};


#endif