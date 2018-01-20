#ifndef PAIR_TREE_MAKER_H
#define PAIR_TREE_MAKER_H

#include "HistoAnalyzer.h"
#include "HistoBins.h"
#include "XmlFunction.h"

#include "vendor/loguru.h"

// ROOT
#include "TNtuple.h"
#include "TTree.h"
#include "TRandom3.h"

// STL
#include <map>

class PairTreeMaker : public HistoAnalyzer
{

protected:
	TNtuple *ntuple;

	XmlFunction xfPt;

	TNtuple *ntuple_sig, *ntuple_bg, *ntuple_mixed;

public:

	PairTreeMaker() {}
	~PairTreeMaker() {}

	vector<TH1 * > hsig_pt;
	vector<TH1 * > hbg_pt;
	TH2 *hsig2d = nullptr, *hbg2d = nullptr;

	HistoBins pt_bins;


	virtual void initialize(){
		HistoAnalyzer::initialize();

		book->cd();
		pt_bins.load( config, "bins.pt" );

		TH2 * hsig2d = get<TH2>( "sig_pos_pT" );
		TH2 * hbg2d = get<TH2>( "bg_pos_pT" );

		LOG_F( INFO, "hsig2d = %p", hsig2d );
		LOG_F( INFO, "hbg2d = %p", hbg2d );

		assert( hsig2d != nullptr );
		assert( hbg2d != nullptr );

		for ( int i = 0; i < pt_bins.nBinEdges() - 1; i++ ){
			float pt1 = pt_bins[ i ];
			float pt2 = pt_bins[ i + 1 ];
			
			int b1 = hsig2d->GetXaxis()->FindBin( pt1 );
			int b2 = hsig2d->GetXaxis()->FindBin( pt2 );
			string n = "hsig_pt_" + ts(i);
			TH1 * h = hsig2d->ProjectionY( n.c_str(), b1, b2 );
			h->SetDirectory(0);
			// LOG_F( INFO, "%s = ProjectionY( %f -> %f, bins %d -> %d )", n.c_str(), pt1, pt2, b1, b2 );
			hsig_pt.push_back( h );

			n = "hbg_pt_" + ts(i);
			h = hbg2d->ProjectionY( n.c_str(), b1, b2 );
			h->SetDirectory(0);
			// LOG_F( INFO, "%s = ProjectionY( %f -> %f, bins %d -> %d )", n.c_str(), pt1, pt2, b1, b2 );
			hbg_pt.push_back( h );

		}


		gRandom = new TRandom3();
		unsigned long long int seed = get_seed();
		LOG_F( INFO, "seed=%llu", seed );
		gRandom->SetSeed( seed );

		LOG_F( INFO, "pt distribution @ %s", config.q( "dist.TF1{name==Pt}" ).c_str() );
		xfPt.set( config, config.q( "dist.TF1{name==Pt}" ) );


		ntuple_sig = new TNtuple( "sigPairs", "Signal Pairs", "pt1:pid1:pt2:pid2" );
		ntuple_bg = new TNtuple( "bgPairs", "Background Pairs", "pt1:pid1:pt2:pid2" );
		ntuple_mixed = new TNtuple( "mixedPairs", "Mixed Pairs", "pt1:pid1:pt2:pid2" );




	}

	unsigned long long int get_seed(){
		unsigned long long int random_value = 0; //Declare value to store data into
		size_t size = sizeof(random_value); //Declare size of data
		ifstream urandom("/dev/urandom", ios::in|ios::binary); //Open stream
		if(urandom) //Check if stream is open
		{
			urandom.read(reinterpret_cast<char*>(&random_value), size); //Read from urandom
			if(urandom) {
				return random_value;
			}
			else { //Read failed
				return 0;
			}
			urandom.close(); //close stream
		} else { //Open failed
			std::cerr << "Failed to open /dev/urandom" << std::endl;
		}
		return 0;
	}

	virtual void make(){
		LOG_SCOPE_FUNCTION( INFO );
		
		book->makeAll( config, nodePath + ".histograms" );

		size_t N = config.get<size_t>( "N", 1000 );
		LOG_F( INFO, "Generating %lu Pairs", N );

		for ( size_t i = 0; i < N; i++ ){
			float pt1 = xfPt.getTF1()->GetRandom();
			int b1 = pt_bins.findBin( pt1 );
			while ( b1 < 0 ){
				pt1 = xfPt.getTF1()->GetRandom();
				b1 = pt_bins.findBin( pt1 );
			}
			
			assert( hsig_pt[ b1 ] );
			assert( hbg_pt[ b1 ] );

			float sig_pid1 = hsig_pt[b1]->GetRandom();
			float bg_pid1 = hbg_pt[b1]->GetRandom();

			float pt2 = xfPt.getTF1()->GetRandom();
			int b2 = pt_bins.findBin( pt2 );

			while( b2 < 0 ){
				pt2 = xfPt.getTF1()->GetRandom();
				b2 = pt_bins.findBin( pt2 );
			}
			assert( hsig_pt[ b2 ] );
			assert( hbg_pt[ b2 ] );

			float sig_pid2 = hsig_pt[b2]->GetRandom();
			float bg_pid2 = hbg_pt[b2]->GetRandom();

			book->fill( "sig", sig_pid1, sig_pid2 );
			book->fill( "bg", bg_pid1, bg_pid2 );
			book->fill( "mixed", sig_pid1, bg_pid2 );
			book->fill( "mixed", bg_pid1, sig_pid2 );

			ntuple_sig->Fill( pt1, sig_pid1, pt2, sig_pid2 );
			ntuple_bg->Fill( pt1, bg_pid1, pt2, bg_pid2 );
			ntuple_mixed->Fill( pt1, sig_pid1, pt2, bg_pid2 );
			ntuple_mixed->Fill( pt1, bg_pid1, pt2, sig_pid2 );

		}
	}

	virtual void postMake(){
		
	}


};




#endif