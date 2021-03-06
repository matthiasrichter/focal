#define getTracks_cxx

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <TRandom3.h>
#include <TStyle.h>
#include <TStopwatch.h>
#include <TView.h>
#include <TLeaf.h>

#include "GlobalConstants/Constants.h"
#include "GlobalConstants/MaterialConstants.h"
#include "GlobalConstants/RangeAndEnergyCalculations.h"
#include "GlobalConstants/Misalign.h"
// #include "Classes/Track/conversionFunctions.h"
#include "Classes/Track/Tracks.h"
#include "Classes/Hit/Hits.h"
#include "Classes/DataInterface/DataInterface.h"
#include "HelperFunctions/Tools.h"
#include "HelperFunctions/getTracks.h"

using namespace std;

void makeTracks(Int_t Runs, Int_t dataType, Bool_t recreate, Float_t energy) {
   Tracks * tracks = loadOrCreateTracks(recreate, Runs, dataType, energy);
   tracks->extrapolateToLayer0();
}

void saveTracks(Tracks *tracks, Int_t dataType, Float_t energy) {
   TString sDataType = (dataType == 0) ? "_MC_" : "_data_";

   tracks->CompressCWT();

   TString sEnergy = Form("_%.2fMeV", energy);
   TString fileName = "Data/Tracks/tracks";
   TString sAbsThick = Form("_%.0fmm", kAbsorbatorThickness);
   TString sMaterial = getMaterialChar();
   fileName.Append(sDataType);
   fileName.Append(sMaterial);
   fileName.Append(sAbsThick);
   fileName.Append(sEnergy);
   fileName.Append(".root");
   
   TFile f(fileName, "recreate");
   f.SetCompressionLevel(1);
   TTree T("T", "tracks");
   T.Branch("tracks", &tracks, 256000, 1);
   T.Fill();
   T.Write();
   f.Close();
}

Tracks * loadTracks(Int_t Runs, Int_t dataType, Float_t energy) {
   TString sDataType = (dataType == 0) ? "_MC_" : "_data_";
   TString sAbsThick = Form("_%.0fmm", kAbsorbatorThickness);
   TString sEnergy = Form("_%.2fMeV", energy);
   TString fileName = "Data/Tracks/tracks";
   TString sMaterial = getMaterialChar();
   fileName.Append(sDataType);
   fileName.Append(sMaterial);
   fileName.Append(sAbsThick);
   fileName.Append(sEnergy);
   fileName.Append(".root");
   
   TFile *f = new TFile(fileName);
   if (!f) return 0;
   TTree *T = (TTree*) f->Get("T");
   Tracks * tracks = new Tracks();

   T->GetBranch("tracks")->SetAutoDelete(kFALSE);
   T->SetBranchAddress("tracks",&tracks);


   T->GetEntry(0);
   
   cout << "There are " << tracks->GetEntriesFast() << " tracks in " << fileName << ".\n";
   
   return tracks;
}

Tracks * loadOrCreateTracks(Bool_t recreate, Int_t Runs, Int_t dataType, Float_t energy, Float_t *x, Float_t *y) {
   Tracks * tracks = nullptr;
   
   if (recreate) {
      printf("kUseAlpide = %d\n", kUseAlpide);
      if (!kUseAlpide) {
         tracks = getTracks(Runs, dataType, kCalorimeter, energy, x, y);
      }
      else {
         tracks = getTracksFromClusters(Runs, dataType, kCalorimeter, energy);
      }

      if (tracks->GetEntries()) {
         cout << "Saving " << tracks->GetEntries() << " tracks.\n";
         saveTracks(tracks, dataType, energy);
      }
   }

   else {
      tracks = loadTracks(Runs, dataType, energy);
   
      if (!tracks) {
         cout << "!tracks, creating new file\n";

         if (!kUseAlpide) {
            tracks = getTracks(Runs, dataType, kCalorimeter, energy, x, y);
         }
         else {
            tracks = getTracksFromClusters(Runs, dataType, kCalorimeter, energy);
         }

         saveTracks(tracks, dataType, energy);
      }
   }
   return tracks;
}

Clusters * getClusters(Int_t Runs, Int_t dataType, Int_t frameType, Float_t energy) {
   DataInterface   * di = new DataInterface();
   Int_t             nClusters = kEventsPerRun * 5 * nLayers;
   Int_t             nHits = kEventsPerRun * 50;
   Int_t             nTracks = kEventsPerRun * 2;
   Bool_t            breakSignal = false;
   CalorimeterFrame *cf = new CalorimeterFrame();
   Clusters        * clusters = new Clusters(nClusters);
   Clusters        * trackerClusters = new Clusters(nClusters);
   Clusters        * allClusters = new Clusters(nClusters * Runs);
   Hits            * hits = new Hits(nHits);
   Hits            * eventIDs = new Hits(kEventsPerRun * sizeOfEventID);
   Int_t             eventID = -1;
   Hits            * trackerHits = new Hits(nHits);
   TRandom3        * gRandom = new TRandom3(0);

   for (Int_t i=0; i<Runs; i++) {

      cout << "Finding clusters " << i*kEventsPerRun << "->" << (i+1)*kEventsPerRun << " of " << Runs * kEventsPerRun << endl;

      if (dataType == kMC) {
         eventID = di->getMCFrame(i, cf);
         di->getEventIDs(i, eventIDs);
         cf->diffuseFrame(gRandom);
         hits = cf->findHits(eventID);
         clusters = hits->findClustersFromHits(); // badly optimized
         clusters->removeSmallClusters(2);

         clusters->matchWithEventIDs(eventIDs);
         eventIDs->Clear();
      }
      
      else if (dataType == kData) {
         di->getDataFrame(i, cf, energy);
         hits = cf->findHits();
         clusters = hits->findClustersFromHits();
         clusters->removeSmallClusters(2);
         clusters->removeAllClustersAfterLayer(8); // bad data in layer 10 and 11
      }
      
      clusters->Compress();
      
      if (clusters->GetEntriesFast() == 0) breakSignal = kTRUE; // to stop running

      for (Int_t j=0; j<clusters->GetEntriesFast(); j++) {
         allClusters->appendCluster(clusters->At(j));
      }

      cf->Reset();
      hits->clearHits();
      trackerHits->clearHits();
      clusters->clearClusters();
      trackerClusters->clearClusters();
      
      if (breakSignal) break;
   }


   delete cf;
   delete clusters;
   delete trackerClusters;
   delete hits;
   delete trackerHits;
   delete di;

   return allClusters;
}

Tracks * getTracksFromClusters(Int_t Runs, Int_t dataType, Int_t frameType, Float_t energy) {
   DataInterface   * di = new DataInterface();
   Int_t             nClusters = kEventsPerRun * 5 * nLayers;
   Int_t             nTracks = kEventsPerRun * 2;
   Bool_t            breakSignal = false;
   Clusters        * clusters = new Clusters(nClusters);
   Tracks          * tracks = new Tracks(nTracks);
   Tracks          * allTracks = new Tracks(nTracks * Runs);

   for (Int_t i=0; i<Runs; i++) {
      showDebug("Start getMCClusters\n");
      di->getMCClusters(i, clusters);

      showDebug("Finding calorimeter tracks\n");
      tracks = clusters->findCalorimeterTracksWithMCTruth();

      if (tracks->GetEntriesFast() == 0) breakSignal = kTRUE; // to stop running

      // Track improvements
      Int_t nTracksBefore = 0, nTracksAfter = 0;
      Int_t nIsInelastic = 0, nIsNotInelastic = 0;
      
      tracks->extrapolateToLayer0();
      nTracksBefore = tracks->GetEntries();
      tracks->removeTracksLeavingDetector();
      nTracksAfter = tracks->GetEntries();
      
      cout << "Of " << nTracksBefore << " tracks, " << nTracksBefore - nTracksAfter << " (" << 100* ( nTracksBefore - nTracksAfter) / ( (float) nTracksBefore ) << "%) were lost when leaving the detector.\n";
      
      // tracks->removeTrackCollisions();
      // tracks->retrogradeTrackImprovement(clusters);

      tracks->Compress();
      tracks->CompressClusters();
      
      for (Int_t j=0; j<tracks->GetEntriesFast(); j++) {
         if (!tracks->At(j)) continue;

         allTracks->appendTrack(tracks->At(j));
      }

      allTracks->appendClustersWithoutTrack(clusters->getClustersWithoutTrack());

      clusters->clearClusters();
      tracks->Clear();

      if (breakSignal) break;
   }

   delete clusters;
   delete tracks;
   delete di;

   return allTracks;
}

Tracks * getTracks(Int_t Runs, Int_t dataType, Int_t frameType, Float_t energy, Float_t *x, Float_t *y) {
   DataInterface   * di = new DataInterface();
   Misalign        * m = new Misalign();
   Int_t             nClusters = kEventsPerRun * 5 * nLayers;
   Int_t             nHits = kEventsPerRun * 50;
   Int_t             nTracks = kEventsPerRun * 2;
   Bool_t            breakSignal = false;
   CalorimeterFrame *cf = new CalorimeterFrame();
   Clusters        * clusters = nullptr; // new Clusters(nClusters);
   Clusters        * trackerClusters = new Clusters(nClusters);
   Hits            * hits = nullptr; // new Hits(nHits);
   Hits            * eventIDs = new Hits(kEventsPerRun * sizeOfEventID);
   Int_t             eventID = -1;
   Hits            * trackerHits = new Hits(nHits);
   Tracks          * calorimeterTracks = nullptr;
   Tracks          * trackerTracks = new Tracks(nTracks);
   Tracks          * allTracks = new Tracks(nTracks * Runs);
   TRandom3        * gRandom = new TRandom3(0);
   TStopwatch        t1, t2, t3, t4, t5, t6;
   ofstream          file("OutputFiles/efficiency.csv", ofstream::out | ofstream::app);
   Int_t             totalNumberOfFrames = 0;
   Int_t             tracksTotalNumberAfterRecon = 0;
   Int_t             tracksRemovedDueToBadChannels = 0;
   Int_t             tracksGivenToReconstruction = 0;
   Int_t             tracksRemovedDueToLeavingDetector = 0;
   Int_t             tracksRemovedDueToNuclearInteractions = 0;
   Int_t             clustersInFirstLayer = 0;
   Int_t             tracksRemovedDueToCollisions = 0;

   // file: np; number of reconstructed tracks; tracks after removeTracksLeavingDetector; tracks after removeTrackCollisions
   
   for (Int_t i=0; i<Runs; i++) {

      cout << "Finding track " << (i+1)*kEventsPerRun << " of " << Runs*kEventsPerRun << "... ";
      
      if (dataType == kMC) {
         t1.Start();

         eventID = di->getMCFrame(i, cf, x, y);
         di->getEventIDs(i, eventIDs);

         t1.Stop(); t2.Start();
         showDebug("Start diffuseFrame\n");

         cf->diffuseFrame(gRandom);

         showDebug("End diffuseFrame, start findHits\n");
         t2.Stop(); t3.Start();

         hits = cf->findHits(eventID);

         showDebug("Number of hits in frame: " << hits->GetEntriesFast() << endl);
         t3.Stop(); t4.Start();

         clusters = hits->findClustersFromHits(); // badly optimized
         
         cout << "Found " << clusters->GetEntriesInLayer(0) << " clusters in the first layer.\n";
         cout << "Found " << clusters->GetEntriesInLayer(1) << " clusters in the second layer.\n";

         clusters->removeSmallClusters(2);
         cout << "Found " << clusters->GetEntriesInLayer(0) << " clusters in the first layer after removeSmallClusters.\n";

         t4.Stop();

         clusters->matchWithEventIDs(eventIDs);
         eventIDs->Clear();
      }
      
      else if (dataType == kData) {
         t1.Start(); di->getDataFrame(i, cf, energy); t1.Stop();
         t3.Start(); hits = cf->findHits(); t3.Stop();
         t4.Start(); clusters = hits->findClustersFromHits(); t4.Stop();
         clusters->removeSmallClusters(2);
         clusters->removeAllClustersAfterLayer(8); // bad data in layer 10 and 11
         cout << "Found " << clusters->GetEntriesInLayer(0) << " clusters in the first layer.\n";
         cout << "Found " << clusters->GetEntriesInLayer(1) << " clusters in the second layer.\n";

         m->correctClusters(clusters);
      }
      
      t5.Start();
      calorimeterTracks = clusters->findCalorimeterTracks();
      t5.Stop();
      
      tracksTotalNumberAfterRecon += calorimeterTracks->GetEntries();

      if (calorimeterTracks->GetEntriesFast() == 0) breakSignal = kTRUE; // to stop running

      // Track improvements
      Int_t nTracksBefore = 0, nTracksAfter = 0;
      Int_t nIsInelastic = 0, nIsNotInelastic = 0;
      
      calorimeterTracks->extrapolateToLayer0();
      calorimeterTracks->splitSharedClusters();
      nTracksBefore = calorimeterTracks->GetEntries();
      calorimeterTracks->removeTracksLeavingDetector();
      nTracksAfter = calorimeterTracks->GetEntries();
      
      tracksRemovedDueToLeavingDetector += nTracksBefore - nTracksAfter;
      
      cout << "Of " << nTracksBefore << " tracks, " << nTracksBefore - nTracksAfter << " (" << 100* ( nTracksBefore - nTracksAfter) / ( (float) nTracksBefore ) << "%) were lost when leaving the detector.\n";
      
      nTracksBefore = calorimeterTracks->GetEntries();
      calorimeterTracks->removeTrackCollisions();
      nTracksAfter = calorimeterTracks->GetEntries();
      
      tracksRemovedDueToCollisions += nTracksBefore - nTracksAfter;

      if (kDataType == kData) {
         nTracksBefore = calorimeterTracks->GetEntries();
         calorimeterTracks->removeTracksEndingInBadChannels();
         nTracksAfter = calorimeterTracks->GetEntries();
         cout << "Of " << nTracksBefore << " tracks, " << nTracksBefore - nTracksAfter << " (" << 100* ( nTracksBefore - nTracksAfter) / ( (float) nTracksBefore ) << "%) were removed due to ending just before a bad channel.\n";
         tracksRemovedDueToBadChannels += nTracksBefore - nTracksAfter;

      }
      
      for (Int_t k=0; k<calorimeterTracks->GetEntriesFast(); k++) {
         if (calorimeterTracks->At(k)) {
            if (calorimeterTracks->At(k)->doesTrackEndAbruptly()) {
               nIsInelastic++;
            }
            else nIsNotInelastic++;
         }
      }
      
      tracksRemovedDueToNuclearInteractions += nIsInelastic;
      cout << "Of these, " << nIsInelastic << " end abruptly and " << nIsNotInelastic << " does not.\n";

      file << energy << " " << kEventsPerRun << " " << nTracksBefore << " " << nTracksAfter << " " << calorimeterTracks->GetEntries() << " " <<  nIsInelastic << " " << nIsNotInelastic << endl;

      // calorimeterTracks->retrogradeTrackImprovement(clusters);

      calorimeterTracks->Compress();
      calorimeterTracks->CompressClusters();
      
      for (Int_t j=0; j<calorimeterTracks->GetEntriesFast(); j++) {
         if (!calorimeterTracks->At(j)) continue;

         allTracks->appendTrack(calorimeterTracks->At(j));
         tracksGivenToReconstruction++;
      }

      allTracks->appendClustersWithoutTrack(clusters->getClustersWithoutTrack());

      cout << Form("Timing: getMCframe (%.2f sec), diffuseFrame (%.2f sec), findHits (%.2f sec), findClustersFromHits (%.2f sec), findTracks (%.2f sec)\n",
              t1.RealTime(), t2.RealTime(), t3.RealTime(), t4.RealTime(), t5.RealTime());

      cf->Reset();
      hits->clearHits();
      trackerHits->clearHits();
      clusters->clearClusters();
      trackerClusters->clearClusters();
      calorimeterTracks->Clear();
      trackerTracks->Clear();

      if (breakSignal) break;
   }
   printf("\033[1mTrack statics for article. Clusters found in first layer (= N protons) = %d. Total number of tracks found = %d. Total number of tracks given to reconstruction = %d. Tracks removed due to bad channels = %d. Tracks removed due to nuclear interactions = %d. Tracks removed due to leaving the detector laterally = %d. Tracks removed due to collisions = %d. Sum = %d.\033[0m\n", clustersInFirstLayer, tracksTotalNumberAfterRecon, tracksGivenToReconstruction, tracksRemovedDueToBadChannels, tracksRemovedDueToNuclearInteractions, tracksRemovedDueToLeavingDetector, tracksRemovedDueToCollisions, tracksGivenToReconstruction + tracksRemovedDueToBadChannels + tracksRemovedDueToLeavingDetector + tracksRemovedDueToCollisions); 

   file.close();

   delete cf;
   delete clusters;
   delete trackerClusters;
   delete hits;
   delete trackerHits;
   delete calorimeterTracks;
   delete trackerTracks;
   delete di;

   return allTracks;
}
