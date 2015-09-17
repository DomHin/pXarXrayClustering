#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdio>


#include "TApplication.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"
#include "TF1.h"
#include "TMath.h"

using namespace std;

typedef struct {
  UShort_t dac;
  UShort_t header;
  UShort_t trailer;
  UShort_t npix;
  UChar_t proc[200];
  UChar_t pcol[200];
  UChar_t prow[200];
  double pval[200];
  double pq[200];
} TreeEvent;


void searchCluster(TreeEvent, TH1D*);
void scanning(TreeEvent, int, int, vector<int>*);
void fillCluster(TreeEvent, TH1D*, vector<int>*);
void printHelp();
void printResults(int, int, int, int, int);


int main(int argc, char *argv[]){

  string filename = "";
  string mpvFile = "clustered/mpv.txt";
  string ext = ".root";
  string resultFolder = "";
  string target = "Sr90";
  int nRoc = 1;

  // Check inputs

  if(argc <2){
    cout << endl;
    cout << "You have to define a Input file!" << endl;
    printHelp();

    return 0;
  }

  if(!strcmp(argv[1],"-h")){
    printHelp();
  }

  for(int i=0; i<argc; i++){
    if (!strcmp(argv[i],"-f")) filename = argv[++i];
    if (!strcmp(argv[i],"-t")) target = argv[++i];    
    if (!strcmp(argv[i],"-m")) nRoc = 16; 
    if (!strcmp(argv[i],"-r")) resultFolder = argv[++i]; 
  }

  TApplication theApp("App", &argc, argv);
    
  // Load file

  TFile *fFile = new TFile(filename.c_str(), "READ");

  if (!fFile->IsOpen()){
    return 0;
  }

  // Get tree and set branch

  /*
  if (!fFile->GetListOfKeys()->Contains("Xray/events")){
    cout << "\nCannot find TTree! Did you enable the FillTree option for the Xray test? Do you have to change the target (default: Sr90)? \n" << endl;
    return 0;
  }
  */
  TTree *fTree = (TTree*)fFile->Get("Xray/events");
  
  if(!fTree){
    cout << "\nERROR: Cannot find TTree! Did you enable the FillTree option for the Xray test?\n" << endl;
    return 0;
  }

  TreeEvent fTreeEvent;

  fTree->SetBranchAddress("header",  &fTreeEvent.header);
  fTree->SetBranchAddress("trailer", &fTreeEvent.trailer);
  fTree->SetBranchAddress("npix",    &fTreeEvent.npix);
  fTree->SetBranchAddress("proc",    fTreeEvent.proc);
  fTree->SetBranchAddress("pcol",    fTreeEvent.pcol);
  fTree->SetBranchAddress("prow",    fTreeEvent.prow);
  fTree->SetBranchAddress("pval",    fTreeEvent.pval);
  fTree->SetBranchAddress("pq",      fTreeEvent.pq);

  // Create result file

  string resultFile = filename.erase(filename.size()-ext.size(),filename.size()) + "_clustered.root";
  
  if(resultFolder.length() != 0){
    resultFile = resultFolder + resultFile.erase(0,resultFile.find_last_of("/"));
  }

  TFile *fResults = new TFile(resultFile.c_str(), "RECREATE");
  fResults->cd();


  // Enter loop over all ROCs

  for(int iRoc=0; iRoc<nRoc; iRoc++){

    // Get the default histogram

    /* c++11
    string xrayString = "Xray/q_" + target + "_C" + to_string(iRoc) + "_V0";
    TH1D *fOrigHisto = (TH1D*)fFile->Get(xrayString.c_str());
    */

    // c++98
    char xrayString[50];
    sprintf(xrayString,"Xray/q_%s_C%i_V0",target.c_str(),iRoc);
    TH1D *fOrigHisto = (TH1D*)fFile->Get(xrayString);
        
    if(!fOrigHisto){
      cout << "\nWARNING: Cannot find default histogram. Did you set the target accordingly (default: Sr90)?\nWARNING: Create output file without default histogram!\n" << endl;
    }else{
      fOrigHisto->GetXaxis()->SetRangeUser(0,2000);
    }

    // Create histograms

    int nbins = 200;

    /* c++11
    string onePixel = "OnePixelEvents_" + to_string(iRoc);
    string twoPixel = "TwoPixelEvents_" + to_string(iRoc);
    string clustered = "Clustered_C" + to_string(iRoc);
    string oneSize = "OnePixelClustered_C" + to_string(iRoc);
    string twoSize = "TwoPixelClustered_C" + to_string(iRoc);
    string xSize = "xPixelClustered_C" + to_string(iRoc);    
    TH1D *fResultHisto1p = new TH1D(onePixel.c_str(), oneSize.c_str(), 2000, 0, 2000);
    TH1D *fResultHisto2p = new TH1D(twoPixel.c_str(), twoSize.c_str() , 2000, 0, 2000); 
    TH1D *fResultHisto = new TH1D(clustered.c_str() , clustered.c_str(), 2000, 0, 2000); 
    TH1D *fResultHisto1 = new TH1D(oneSize.c_str(), oneSize.c_str(), 2000, 0, 2000);
    TH1D *fResultHisto2 = new TH1D(twoSize.c_str(), twoSize.c_str() , 2000, 0, 2000); 
    TH1D *fResultHistoX = new TH1D(xSize.c_str(), xSize.c_str() , 2000, 0, 2000);    
    */

    char onePixel[50], twoPixel[50], clustered[50], twoPixelClustered[50], xPixelClustered[50];

    sprintf(onePixel, "OnePixelEvents_C%i", iRoc);
    sprintf(twoPixel, "TwoPixelEvents_C%i", iRoc);
    sprintf(clustered, "All_clustered_C%i", iRoc);
    sprintf(twoPixelClustered, "TwoPixelEvents_clustered_C%i", iRoc);
    sprintf(xPixelClustered, "WithoutOnePixelEvents_clustered_C%i", iRoc);


    TH1D *fResultHisto1p = new TH1D(onePixel, onePixel, nbins, 0, 2000);
    TH1D *fResultHisto2p = new TH1D(twoPixel, twoPixel, nbins, 0, 2000); 
    TH1D *fResultHisto = new TH1D(clustered , clustered, nbins, 0, 2000); 
    TH1D *fResultHisto2 = new TH1D(twoPixelClustered, twoPixelClustered, nbins, 0, 2000); 
    TH1D *fResultHistoX = new TH1D(xPixelClustered, xPixelClustered, nbins, 0, 2000); 
        
    int n0(0), n1(0), n2(0), n3(0);

    cout << endl;
    cout << "-------------- ROC " << iRoc << " -------------- \n";
    cout << "scanning..\n";

    // Loop over all entries

    for(int i=0; i<fTree->GetEntries(); i++){
      
      fTree->GetEntry(i);
      
      if(int(fTreeEvent.proc[0]) == iRoc){
	
	searchCluster(fTreeEvent, fResultHisto);

	if(fTreeEvent.npix == 0){
	  n0++;
	}else if(fTreeEvent.npix == 1){
	  n1++;
	  fResultHisto1p->Fill(fTreeEvent.pq[0]);
	}else if(fTreeEvent.npix == 2){
	  n2++;
	  fResultHisto2p->Fill(fTreeEvent.pq[0]+fTreeEvent.pq[1]);
	  searchCluster(fTreeEvent, fResultHisto2);
	}else{
	  n3++;
	  searchCluster(fTreeEvent, fResultHistoX);
	}	
      }
    }
   
    printResults(iRoc,n0,n1,n2,n3);
    
    if(fOrigHisto){
      fOrigHisto->Write();
    }
    fResultHisto1p->Write();
    fResultHisto2p->Write();    
    fResultHisto->Write();
    fResultHisto2->Write();
    fResultHistoX->Write();
    
  }
  
  fResults->Close();
  
  return 0;
}



void searchCluster(TreeEvent fTreeEvent, TH1D *fResultHisto){

  if((fTreeEvent.npix == 1) && (fTreeEvent.pq[0] != 0)){
    fResultHisto->Fill(fTreeEvent.pq[0]);
  }else if(fTreeEvent.npix > 1){
    
    vector<int> *clusterArray = new vector<int>(fTreeEvent.npix,0);
    int cluster(1);

    for(int i(0); i<fTreeEvent.npix; i++){

      if(clusterArray->at(i) != 0){
	continue;
      }

      scanning(fTreeEvent, i, cluster, clusterArray);
      cluster++;
    }
    
    fillCluster(fTreeEvent, fResultHisto, clusterArray);
     
  }
}

void scanning(TreeEvent fTreeEvent, int i, const int cluster, vector<int>* clusterArray){
  
  for(int j(0); j<fTreeEvent.npix; j++){

    if(i == j){
      continue;
    }

    if(clusterArray->at(j) != 0){
      continue;
    }

    if((abs(fTreeEvent.pcol[i]-fTreeEvent.pcol[j]) == 1 && fTreeEvent.prow[i]-fTreeEvent.prow[j] == 0) || (fTreeEvent.pcol[i]-fTreeEvent.pcol[j] == 0 && abs(fTreeEvent.prow[i]-fTreeEvent.prow[j]) == 1) || abs((fTreeEvent.pcol[i]-fTreeEvent.pcol[j]) == 1 && abs(fTreeEvent.prow[i]-fTreeEvent.prow[j]) == 1)){
    
      if(clusterArray->at(i)!=0 && clusterArray->at(j) == 0){
	clusterArray->at(j) = clusterArray->at(i);
      }else if(clusterArray->at(i) == 0 && clusterArray->at(j) == 0){
	clusterArray->at(j) = cluster;
	clusterArray->at(i) = cluster;
      }

      scanning(fTreeEvent,j,cluster,clusterArray);
    }
    if(clusterArray->at(i) == 0){
      clusterArray->at(i) = cluster;
    }
  } 
}

void fillCluster(TreeEvent fTreeEvent, TH1D *fResultHisto, vector<int>* clusterArray){
  
  int nCluster(1);
  double pqCluster(0);

  for(unsigned int i(0); i<clusterArray->size(); i++){
    if(clusterArray->at(i) > nCluster){
      nCluster = clusterArray->at(i);
    }
  }

  for(int i(0); i<=nCluster; i++){
    for(unsigned int j(0); j<clusterArray->size(); j++){
      if(clusterArray->at(j) == i){
	pqCluster += fTreeEvent.pq[j];
      }
    }

    if(pqCluster != 0){
      fResultHisto->Fill(pqCluster);
    }
    pqCluster = 0;
  }
}

void printHelp(){
  
  cout << endl;
  cout << "Simple clustering alogrithm for pXar output files of the xray test (FillTree required) \n" << endl;
  cout << "-f \t Input file " << endl;
  cout << "-t \t Target/source (Default: Sr90) Note: string is case sensitive!" << endl;
  cout << "-m \t Add -m to analyze full modules (default: nPix = 1)" << endl;
  cout << "-r \t Parse a folder for the result files" << endl;
  cout << endl;

}

void printResults(int iRoc, int n0, int n1, int n2, int n3){

  cout << endl;
  cout << "Empty events:\t\t" << n0 << endl;
  cout << "One pixel events:\t" << n1 << endl;
  cout << "Two pixel events:\t" << n2 << endl;
  cout << " >2 pixel events:\t" << n3 << endl;
  cout << "-----------------------------------" << endl;
  cout << endl;

}
