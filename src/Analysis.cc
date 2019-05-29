//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Author:
// Areg Danagoulian, 2015
// MIT, NSE
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
///////////////////////////////////////////////////////////////////////////////

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "Analysis.hh"

#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include "G4RunManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4Track.hh"
#include "G4ClassificationOfNewTrack.hh"
#include "G4TrackStatus.hh"
#include "G4Step.hh"
#include "G4Types.hh"
#include "G4ThreeVector.hh"

#include "G4GDMLParser.hh"

#include <time.h>
#include <vector>

static const G4double LambdaE = 2.0 * 3.14159265358979323846 * 1.973269602e-16 * m * GeV;

#if defined (G4ANALYSIS_USE_ROOT)

#include "TROOT.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TH1.h"
#include "TPad.h"
//  #include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMath.h"
#endif /* defined (G4ANALYSIS_USE_ROOT) */

#include <iostream>
#include <fstream>

extern G4String RootOutputFile;
extern G4GDMLParser parser;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Analysis::Analysis()
{
  LastDoItTime = (time_t)0;

  file_even = 100000;

  nextinline=1;

  counter=1;

  LightProducingParticle=parser.GetConstant("LightProducingParticle");
  EventGeneratorParticle=parser.GetConstant("ParticleNumber");
  LowEnergyCutoff		=parser.GetConstant("LowEnergyCutoff");
  if(parser.GetConstant("KeepOnlyMainParticle")==0)
    KeepOnlyMainParticle=false;
  else
    KeepOnlyMainParticle=true;

  SaveSurfaceHitTrack=false;
  SaveTrackInfo=false;
  SaveEdepositedTotalEntry=false;
  if(parser.GetConstant("SaveSurfaceHitTrack")==1) SaveSurfaceHitTrack=true;
  if(parser.GetConstant("SaveTrackInfo")==1) SaveTrackInfo=true;
  if(parser.GetConstant("SaveEdepositedTotalEntry")==1) SaveEdepositedTotalEntry=true;

  // file for the text output
  
#if defined (G4ANALYSIS_USE_ROOT)

  if (gSystem) gSystem->ProcessEvents();

  // open the output data file

  TString outfile_name = RootOutputFile;

  f = new TFile(outfile_name.Data(),"RECREATE");

  // create a "tree" for the data to be stored into

  tree = new TTree("tree","main tree");

  // defines "branches" of the tree

  //  tree->Branch("npart",&npart,"npart/I");
  tree->Branch("E_incident",&E,"E_incident/D");
  tree->Branch("E_deposited",&Edep,"E_deposited/D");
  tree->Branch("x_incident",&x,"x_incident/D");
  tree->Branch("y_incident",&y,"y_incident/D");
  tree->Branch("z_incident",&z,"z_incident/D");
  tree->Branch("theta",&theta,"theta/D");
  tree->Branch("Time",&Time,"Time/F");
  tree->Branch("EventID",&EventID,"EventID/I");
  tree->Branch("TrackID",&TrackID,"TrackID/I");
  tree->Branch("ParticleID",&ID,"ParticleID/I");
//  char *tmp=(char*)ParticleName_placeholder.c_str(); //same as below -- set the pointer to the placeholder for particle name
//  tree->Branch("ParticleName",tmp,"ParticleName/C");
  tree->Branch("ParticleName",&ParticleName);
  tree->Branch("CreatorProcessID",&ProcID,"CreatorProcessID/I"); //obsolete
//  char *tmp2=(char*)CreatorProcessName.c_str(); //a tmp pointer at the creatorprocessname, which will be updated at the end of every track
  tree->Branch("CreatorProcessName",&CreatorProcessName);
  tree->Branch("IsEdepositedTotalEntry",&IsSummaryEntry,"IsEdepositedTotalEntry/O");
  tree->Branch("IsSurfaceHitTrack",&IsSurfaceHitTrack,"IsSurfaceHitTrack/O");
 
 

#endif /* defined (G4ANALYSIS_USE_ROOT) */
  textoutput=parser.GetConstant("TextOutputOn");
  briefoutput=parser.GetConstant("BriefOutputOn");
  
  if(textoutput){
	  std::string data_file_name;
	  std::string rootoutputfile=(std::string)RootOutputFile;
	  if(rootoutputfile.find(".root")<rootoutputfile.length()){
		  data_file_name=(std::string)rootoutputfile.substr(0,rootoutputfile.find(".root"));
		  data_file_name+=".dat";
	  }
	  else
		  data_file_name=(std::string)RootOutputFile+".dat";

	  data_file.open(data_file_name.c_str(), std::ofstream::out | std::ofstream::trunc); //open the text file
	  if(briefoutput)
	    data_file << "E(MeV)\tEventID\tParticleName\tCreatorProcessName\tTime(ns)" << std::endl;	    
	  else
	    data_file << "E_incident(MeV)\tE_deposited(MeV)\tx_incident\ty_incident\tz_incident\ttheta\tTime\tEventID\tTrackID\tParticleID\tParticleName\tCreatorProcessName\tIsEdepositedTotalEntry\tIsSurfaceHitTrack" << std::endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Analysis::~Analysis()
{
  //OnceAWhileDoIt();
  
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  f->Write();
  f->Close();
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  if(textoutput)
	  data_file.close();
  

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::Construct(const G4VPhysicalVolume *theWorldWolume)
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::ConstructParticle()
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::ConstructProcess()
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::SetCuts()
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::GeneratePrimaries(const G4Event *anEvent)
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::BeginOfEventAction(const G4Event *anEvent)
{

	EventID = anEvent->GetEventID();

	ResetEverything(); //reset all the class variables

#if defined (G4ANALYSIS_USE_ROOT)

	if (gSystem) gSystem->ProcessEvents();


#endif /* defined (G4ANALYSIS_USE_ROOT) */

	OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::EndOfEventAction(const G4Event *anEvent)
{
  
	for(int i=0;i<Ev.size();++i){ //looping over all the tracks in the detector
		if(IsSurfaceHit.at(i))
			E=Ev.at(i);	//fill the incident energy variable ONLY for incident hit entries
		Edep=Edepv.at(i);
		x=xv.at(i);
		y=yv.at(i);
		z=zv.at(i);
		theta=thetav.at(i);
		ID=IDv.at(i);
		TrackID=TrackIDv.at(i);
		//    EventID=EventIDv.at(i); //defined in the beginning
		ProcID=ProcIDv.at(i);
		CreatorProcessName=ProcessNamev.at(i); //copy over the name to the placeholder
		ParticleName=ParticleNamev.at(i); //same here
		Time=Timev.at(i);
		IsSurfaceHitTrack=IsSurfaceHit.at(i);

		if(detector_hit.at(i) &&
				(
						(IsSurfaceHitTrack && SaveSurfaceHitTrack )
						|| SaveTrackInfo
				)
		){//filter out the empty stuff

			if(textoutput){
				if(briefoutput){
					data_file << std::setprecision(5);
					data_file << E
							<< "\t" << EventID
							<< "\t" << ParticleName.c_str()
							<< "\t" << CreatorProcessName.c_str()
							<< "\t" << Time
							<<std::endl;
				}
				else{
					data_file << std::setprecision(5);
					data_file << E
							<< "\t" << Edep
							<< "\t" << x
							<< "\t" << y
							<< "\t" << z
							<< "\t" << theta
							<< "\t" << Time
							<< "\t" << EventID
							<< "\t" << TrackID
							<< "\t" << ID
							<< "\t" << ParticleName.c_str()
							<< "\t" << CreatorProcessName.c_str()
							<< "\t" << 0
							<< "\t" << IsSurfaceHitTrack
							<<std::endl;
				}
			}
#if defined (G4ANALYSIS_USE_ROOT)
			else
				tree->Fill();
#endif


		}
	}
  //Now, fill the summary entry
  if(SaveEdepositedTotalEntry){
	  IsSummaryEntry=true;
	  IsSurfaceHitTrack=false;
	  Edep=0; //set it to zero
	  CreatorProcessName="";
	  for(int i=0;i<Ev.size();++i){ //add ALL the deposited energies from ALL the tracks that hit the detector
		  if(detector_hit.at(i) &&  //make sure we are inside the detector
				  ( IDv.at(i)==LightProducingParticle ||   //this is the designated light producing particle, OR
						  LightProducingParticle==0 ||           //the light producing particle has been set to 0 (i.e. everything), OR
						  ( LightProducingParticle==2212 && ProcessNamev.at(i)=="hIoni") ) //this is an hadron ionization track from a proton
		  ){
			  Edep+=Edepv.at(i);
			  CreatorProcessName+=ProcessNamev.at(i)+"/";
		  }
	  }
#if defined (G4ANALYSIS_USE_ROOT)
	  if (gSystem) gSystem->ProcessEvents();
#endif /* defined (G4ANALYSIS_USE_ROOT) */
	  if(Edep>0){

		  if(textoutput){
			  if(briefoutput){
				  data_file << std::setprecision(5);
				  data_file << "\t" << Edep
						  << "\t" << EventID
						  << "\t" << -1
						  << "\t" << CreatorProcessName.c_str()
						  << std::endl;

			  }
			  else{
				  data_file << std::setprecision(5);
				  data_file << -1
						  << "\t" << Edep
						  << "\t" << -1
						  << "\t" << -1
						  << "\t" << -1
						  << "\t" << -1
						  << "\t" << Time
						  << "\t" << EventID
						  << "\t" << -1
						  << "\t" << -1
						  << "\t" << -1
						  << "\t" << CreatorProcessName.c_str()
						  << "\t" << 1
						  << "\t" << 0
						  << std::endl;
			  }
		  }
#if defined (G4ANALYSIS_USE_ROOT)
		  else
			  tree->Fill();
#endif
	  }
  }
#if defined (G4ANALYSIS_USE_ROOT)
  if (tree->GetEntries()%500000 == 1) tree->AutoSave("SaveSelf");
#endif

  if(anEvent->GetEventID() % 100 == 0)
	  std::cout << "\r\tEvent, and tracks:\t" << anEvent->GetEventID() << "\t" << Ev.size() << std::flush;


  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::ClassifyNewTrack(
			     const G4Track *aTrack,
			     G4ClassificationOfNewTrack *classification_ptr)
{
  // G4ClassificationOfNewTrack &classification = (*classification_ptr);
  
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  G4String particleName = aTrack->GetDefinition()->GetParticleName();
  
  if (particleName == "opticalphoton")
    {
      Double_t aWaveLength = 0.0; // will be in [nanometer]
      aWaveLength = (LambdaE / aTrack->GetTotalEnergy()) / nanometer; // in [nanometer]
      // aWaveLength = (LambdaE / aTrack->GetKineticEnergy()) / nanometer; // in [nanometer]
      //      if (hOPWaveLength) hOPWaveLength->Fill(aWaveLength, aTrack->GetWeight()); // Wavelength of the produced optical photon
    }
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::NewStage()
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::PrepareNewEvent()
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::PreUserTrackingAction(const G4Track *aTrack)
{
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::PostUserTrackingAction(const G4Track *aTrack,
						G4TrackStatus *status_ptr)
{
  // G4TrackStatus &status = (*status_ptr);
  
#if defined (G4ANALYSIS_USE_ROOT)
  
  if (gSystem) gSystem->ProcessEvents();
  
  
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
  
  OnceAWhileDoIt();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::UserSteppingAction(const G4Step *aStep)
{


	if(aStep->GetPostStepPoint()==NULL) {
		return;} //skip this if we are at the end of the world
	else if(aStep->GetPostStepPoint()->GetPhysicalVolume()==NULL){
		return;
	}

	int trackid =  aStep->GetTrack()->GetTrackID();

	if(TrackMustDie(aStep)){ //apply the cuts here
		aStep->GetTrack()->SetTrackStatus(fStopAndKill);
//		if(aStep->GetTrack()->GetDefinition()->GetParticleName()=="gamma")
//			std::cout << "killing a photon: " << EventID << "\t" << trackid << "\t" << aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName() << "\t" << aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() <<std::endl;

		return;
	}



	if(Ev.size()<trackid){ //a new track
		Ev.resize(trackid,-1e+6);
		Edepv.resize(trackid,0);
		xv.resize(trackid,-1e+6);
		yv.resize(trackid,-1e+6);
		zv.resize(trackid,-1e+6);
		thetav.resize(trackid,-1e+6);
		IDv.resize(trackid,-1e+6);
		ParticleNamev.resize(trackid,"");
		ProcessNamev.resize(trackid,"");
		TrackIDv.resize(trackid,-1e+6);
		EventIDv.resize(trackid,-1e+6);
		ProcIDv.resize(trackid,-1e+6);
		Timev.resize(trackid,-1e+6);
		detector_hit.resize(trackid,false);
		IsSurfaceHit.resize(trackid,false);

	}


	if(aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName()=="det_phys" &&
			aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()=="world_log_PV"
					&& IsSurfaceHit[trackid-1]==false //check that this is truly the first time
	) //stepping into det for the first time
	{


		IsSurfaceHit[trackid-1]=true;

		Ev[trackid-1]= aStep->GetPreStepPoint()->GetKineticEnergy()/(MeV);

		X= aStep->GetPostStepPoint()->GetPosition(); //take the position from the post step position
		xv[trackid-1]=X.x()/(mm);
		yv[trackid-1]=X.y()/(mm);
		zv[trackid-1]=X.z()/(mm);
		Timev[trackid-1]= aStep->GetTrack()->GetGlobalTime();
		p = aStep->GetPreStepPoint()->GetMomentum();
		thetav[trackid-1]=asin(sqrt(pow(p.x(),2)+pow(p.y(),2))/p.mag()); //the angle of the particle relative to the Z axis
		if(aStep->GetTrack()->GetDefinition()->GetParticleName()=="gamma"){
			if(theta>3.14159/4.){
				std::cout << "Potentially dangerous behavior:  killing gammas >45deg" << std::endl;
				aStep->GetTrack()->SetTrackStatus(fStopAndKill); // kill all gammas that are >45deg
			}
		}


		ParticleNamev[trackid-1]= aStep->GetTrack()->GetDefinition()->GetParticleName();
		if(aStep->GetTrack()->GetTrackID()>1)
			ProcessNamev[trackid-1]= aStep->GetTrack()->GetCreatorProcess()->GetProcessName();
		else
			ProcessNamev[trackid-1]= "EventGenerator";

		npart++;

	}


	if(aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()=="det_phys"){ //in the det

		detector_hit[trackid-1]=true; //this track interacted with the detector
		IDv[trackid-1]= aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
		ParticleNamev[trackid-1]= aStep->GetTrack()->GetDefinition()->GetParticleName();
		if(aStep->GetTrack()->GetTrackID()>1)
			ProcessNamev[trackid-1]= aStep->GetTrack()->GetCreatorProcess()->GetProcessName();
		else
			ProcessNamev[trackid-1]= "EventGenerator";
		TrackIDv[trackid-1]= aStep->GetTrack()->GetTrackID();
		Edepv[trackid-1] += aStep->GetTotalEnergyDeposit()/(MeV);

	}

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::OnceAWhileDoIt(const G4bool DoItNow)
{
  time_t Now = time(0); // get the current time (measured in seconds)
  if ( (!DoItNow) && (LastDoItTime > (Now - 10)) ) return; // every 10 seconds
  LastDoItTime = Now;
  
#if defined (G4ANALYSIS_USE_ROOT)
  if (gSystem) gSystem->ProcessEvents();
  
#endif /* defined (G4ANALYSIS_USE_ROOT) */
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
bool Analysis::TrackMustDie(const G4Step *aStep){

  int particleid = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
  std::string creatorprocessname;
  if(aStep->GetTrack()->GetCreatorProcess()==NULL)
    creatorprocessname="EventGenerator";
  else
    creatorprocessname=aStep->GetTrack()->GetCreatorProcess()->GetProcessName();
  std::string volumename=aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName(); //changed this from PostStep to PreStep, that way non-light producing particles can still be recorded at the surface, in SurfaceHit mode
  if(volumename!="det_phys"){ //outside the detector
    if(particleid!=EventGeneratorParticle && KeepOnlyMainParticle){ //only keep the main track
      return true;
    }
  }
/*
  else{ //if inside the detector
    if(particleid!=EventGeneratorParticle &&
       particleid!=LightProducingParticle && 
       creatorprocessname!="hIoni" && 
       LightProducingParticle!=0){// checks if inside the detector, and if the "unneeded" particles (no light production).  This could cause problems in SurfaceHit mode, as the particle gets killed _before_ entering the detector
//      return true;  //this improves the efficiency, but causes various problems
    }
  }
*/
  //Energy cuts
  if(aStep->GetTrack()->GetDefinition()->GetPDGEncoding()==EventGeneratorParticle &&
     aStep->GetTrack()->GetKineticEnergy()/MeV < LowEnergyCutoff){ //for the main track, kill it off below some limit
    return true;
  }
	
  return false;
}
void Analysis::ResetEverything()
{
	  npart=0;
	  E=-1;
	  x=-1e5;
	  y=-1e5;
	  z=-1e5;
	  theta=-1e5;
	  ID=-100000;
	  TrackID=-10;
	  ProcID=-1;
	  Time=-1;
	  IsSummaryEntry=false;
	  IsSurfaceHitTrack=false;
	  Ev.clear();
	  Edepv.clear();
	  xv.clear();
	  yv.clear();
	  zv.clear();
	  thetav.clear();
	  IDv.clear();
	  ParticleNamev.clear();
	  ParticleName="n/a";
	  ProcessNamev.clear();
	  CreatorProcessName="n/a";
	  TrackIDv.clear();
	  EventIDv.clear();
	  ProcIDv.clear();
	  Timev.clear();
	  detector_hit.clear();
	  IsSurfaceHit.clear();

}
