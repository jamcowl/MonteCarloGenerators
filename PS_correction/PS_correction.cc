// C++
// by James Cowley, UCL
// plots pT distributions of hadron and parton jets

#include "Rivet/Analyses/MC_JetAnalysis.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Tools/ParticleIdUtils.hh"
#include "HepMC/GenParticle.h"

namespace Rivet {
  
  using namespace Cuts;
  
  class PS_correction : public Analysis {
    
  public:
    int evNum;       // counter for tracking # of events
    double maxR;     // jet clustering R parameter
    double pTmin;    // minimum pT for all analysed jets
    double pTmax;    // maximum pT for analysed jets
    double maxAbsY;  // maximum rapidity cut
    string evType;   // type of event to analyse

    PS_correction() : Analysis("PS_correction"){}
    
    // *******************************************************************************************************
    //  INITIALIZE ANALYSIS
    // *******************************************************************************************************
    void init() {
      
      evNum   = 0;
      maxR    = 0.7;
      pTmin   = 25*GeV;
      pTmax   = 400;
      maxAbsY = 3.0;
      evType  = "HAS_PARTONS";
      
      // histogram properties
      string xl = "pT [GeV]";                    // x axis label
      string yp = "# of hard partons";           // y axis label for hard partons
      string yh = "# of parton shower jets";     // y axis label for parton shower jets
      int nb = 25;                               // number of bins (25*15 = 375)
      double binMin = 25;                        // minimum pT bin edge
      double binMax = 400;                       // maximum pT bin edge
      
      std::ostringstream rtos;
      rtos << std::setprecision(1) << maxR;
      string rt = rtos.str();                      // rparameter string for titles
      string rtp = "Hard Parton pT (R = "+rt+")";  // histogram title for parton jet pT
      string rth = "Parton Jet pT (R = "+rt+")";   // histogram title for hadron jet pT
      string nmh = "END_SHOWER_PARTON_JET_pT";     // histogram name for parton shower jets
      string nmp = "HARD_PARTON_pT";               // histogram name for hard parton jets
      
      // hard parton histos:
      _h_pjetpt       = bookHisto1D(nmp,        nb, binMin, binMax, rtp+" (all y)",       xl, yp);
      _h_pjetpt_00_05 = bookHisto1D(nmp+"_0.5", nb, binMin, binMax, rtp+" (0.0<|y|<0.5)", xl, yp);
      _h_pjetpt_05_10 = bookHisto1D(nmp+"_1.0", nb, binMin, binMax, rtp+" (0.5<|y|<1.0)", xl, yp);
      _h_pjetpt_10_15 = bookHisto1D(nmp+"_1.5", nb, binMin, binMax, rtp+" (1.0<|y|<1.5)", xl, yp);
      _h_pjetpt_15_20 = bookHisto1D(nmp+"_2.0", nb, binMin, binMax, rtp+" (1.5<|y|<2.0)", xl, yp);
      _h_pjetpt_20_25 = bookHisto1D(nmp+"_2.5", nb, binMin, binMax, rtp+" (2.0<|y|<2.5)", xl, yp);
      _h_pjetpt_25_30 = bookHisto1D(nmp+"_3.0", nb, binMin, binMax, rtp+" (2.5<|y|<3.0)", xl, yp);
      
      // parton shower histos:
      _h_hjetpt       = bookHisto1D(nmh,        nb, binMin, binMax, rth+" (all y)",       xl, yh);
      _h_hjetpt_00_05 = bookHisto1D(nmh+"_0.5", nb, binMin, binMax, rth+" (0.0<|y|<0.5)", xl, yh);
      _h_hjetpt_05_10 = bookHisto1D(nmh+"_1.0", nb, binMin, binMax, rth+" (0.5<|y|<1.0)", xl, yh);
      _h_hjetpt_10_15 = bookHisto1D(nmh+"_1.5", nb, binMin, binMax, rth+" (1.0<|y|<1.5)", xl, yh);
      _h_hjetpt_15_20 = bookHisto1D(nmh+"_2.0", nb, binMin, binMax, rth+" (1.5<|y|<2.0)", xl, yh);
      _h_hjetpt_20_25 = bookHisto1D(nmh+"_2.5", nb, binMin, binMax, rth+" (2.0<|y|<2.5)", xl, yh);
      _h_hjetpt_25_30 = bookHisto1D(nmh+"_3.0", nb, binMin, binMax, rth+" (2.5<|y|<3.0)", xl, yh);
    }

    
    // *******************************************************************************************************
    //  ANALYSE EVENT
    // *******************************************************************************************************
    void analyze(const Event& event) {
      double weight = 1; // alternatively = event.weight();
      evNum++;
      if(evNum > 250){ cout<<"'"<<evType<<"' analysis, R = "<<maxR<<", pTmin = "<<pTmin<<", event #"<<evNum<<endl; }      
      
      
      // *******************************************************************************************************
      //  COLLECT PARTONS
      // *******************************************************************************************************
      std::vector<HepMC::GenParticle*> allParticles = particles(event.genEvent());
      Particles hardPartons, endPartons;
      
      // get the partons
      foreach(GenParticle* gp, allParticles){
        int thisStatus = gp->status();	
	
	// hard partons
	if(thisStatus == 23){
	  Particle p(gp);
	  hardPartons.push_back(p);
	}
	
	// parton shower partons
	if(60 < thisStatus && thisStatus < 70){
	  Particle p(gp);
	  bool isEndParton = true;
	  Particles pcs = p.children();
	  foreach(Particle c, pcs){
	    double cStat = c.genParticle()->status();
	    if(60 < cStat && cStat < 70){
	      isEndParton = false;
	      break;
	    }
	  }
	  if(isEndParton){ endPartons.push_back(p); }
	}
      }
      
      
      // *******************************************************************************************************
      //  CLUSTER PARTON JETS WITH ANTI-KT
      // *******************************************************************************************************
      
      // apply anti-kT to partons
      FastJets hardPartonJets(FastJets::ANTIKT, maxR);
      FastJets softPartonJets(FastJets::ANTIKT, maxR);
      hardPartonJets.calc(hardPartons);  // jets from hard partons
      softPartonJets.calc(endPartons);   // jets after parton shower
      
      // apply cuts to jets
      Jets cutHardPartonJets = hardPartonJets.jetsByPt(pTmin, pTmax, -maxAbsY, maxAbsY);
      Jets cutSoftPartonJets = softPartonJets.jetsByPt(pTmin, pTmax, -maxAbsY, maxAbsY);
      
      
      // *******************************************************************************************************
      //  PLOT HARD PARTON JET PROPERTIES
      // *******************************************************************************************************      
      foreach(Jet hj, cutHardPartonJets){
	double thispT = hj.pT();
	double absY = hj.absrapidity();
	
	// populate hard parton jet pT histograms:
	_h_pjetpt->fill(thispT, weight);
	     if (absY < 0.5){ _h_pjetpt_00_05->fill(thispT, weight); }
	else if (absY < 1.0){ _h_pjetpt_05_10->fill(thispT, weight); }
	else if (absY < 1.5){ _h_pjetpt_10_15->fill(thispT, weight); }
	else if (absY < 2.0){ _h_pjetpt_15_20->fill(thispT, weight); }
	else if (absY < 2.5){ _h_pjetpt_20_25->fill(thispT, weight); }
	else if (absY < 3.0){ _h_pjetpt_25_30->fill(thispT, weight); }
	     
      } // end of parton jet loop
      
      
      // *******************************************************************************************************
      //  PLOT PARTON SHOWER JET PROPERTIES
      // *******************************************************************************************************
      foreach(Jet sj, cutSoftPartonJets){
	double thispT = sj.pT();
	double absY = sj.absrapidity();
	
	// populate parton shower jet pT histograms:
	_h_hjetpt->fill(thispT, weight);
	     if (absY < 0.5){ _h_hjetpt_00_05->fill(thispT, weight); }
	else if (absY < 1.0){ _h_hjetpt_05_10->fill(thispT, weight); }
	else if (absY < 1.5){ _h_hjetpt_10_15->fill(thispT, weight); }
	else if (absY < 2.0){ _h_hjetpt_15_20->fill(thispT, weight); }
	else if (absY < 2.5){ _h_hjetpt_20_25->fill(thispT, weight); }
	else if (absY < 3.0){ _h_hjetpt_25_30->fill(thispT, weight); }
      } // end of hadron jet loop
      
    } // end of analyze() function
    
    
    // *******************************************************************************************************
    //  FINALISE EVENT
    // *******************************************************************************************************
    void finalize() {
      
      // normalize using:
      // normalize(IHistogram1D* histo, const double norm)
      
    }
    
  private:
    // parton jet histos:
    Histo1DPtr _h_pjetpt;
    Histo1DPtr _h_pjetpt_00_05; Histo1DPtr _h_pjetpt_05_10; Histo1DPtr _h_pjetpt_10_15;
    Histo1DPtr _h_pjetpt_15_20; Histo1DPtr _h_pjetpt_20_25; Histo1DPtr _h_pjetpt_25_30;
    
    // hadron jet histos:
    Histo1DPtr _h_hjetpt;
    Histo1DPtr _h_hjetpt_00_05; Histo1DPtr _h_hjetpt_05_10; Histo1DPtr _h_hjetpt_10_15;
    Histo1DPtr _h_hjetpt_15_20; Histo1DPtr _h_hjetpt_20_25; Histo1DPtr _h_hjetpt_25_30;
  };
  
  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(PS_correction);
}
