#include <vector>
#include <algorithm>

#include <TStopwatch.h>

#include "Classes/Hit/Hits.h"
#include "Classes/Cluster/Clusters.h"
#include "GlobalConstants/Constants.h"
#include "GlobalConstants/MaterialConstants.h"
#include "HelperFunctions/Tools.h"

Clusters * Hits::findClustersFromHits() {
   Clusters        * clusters = new Clusters(kEventsPerRun * 20);
   vector<Int_t>   * expandedCluster = nullptr;
   vector<Int_t>   * checkedIndices = nullptr; // Optimization: Make this array
   vector<Int_t>   * firstHits = nullptr;
   Int_t             layerIdxFrom, layerIdxTo;

   for (Int_t layer=0; layer<nLayers; layer++) {
      layerIdxFrom = getFirstIndexOfLayer(layer);
      layerIdxTo = getLastIndexOfLayer(layer);

      if (layerIdxFrom<0) continue;

      makeVerticalIndexOnLayer(layer); // optimization

      checkedIndices = new vector<Int_t>;
      checkedIndices->reserve(layerIdxTo - layerIdxFrom);

      for (Int_t i = layerIdxFrom; i < layerIdxTo; i++) {
         if (isItemInVector(i, checkedIndices)) continue;

         firstHits = findNeighbours(i);

         if (firstHits->size()) {
            // Find expanded clusters is the time demanding function
            expandedCluster = getAllNeighboursFromCluster(i, checkedIndices); 

            appendNeighboursToClusters(expandedCluster, clusters);
            delete expandedCluster;
            delete firstHits;
         }
      }
      delete checkedIndices;
   }

   if (kEventsPerRun == 1) {
      for (Int_t i=0; i<clusters->GetEntriesFast(); i++) {
         clusters->At(i)->setEventID(getEventID(0));
      }
   }

   return clusters;
}

vector<Int_t> * Hits::findNeighbours(Int_t index) {
   vector<Int_t>   * neighbours = new vector<Int_t>;
   Int_t             xGoal = getX(index);
   Int_t             yGoal = getY(index);
   Int_t             layer = getLayer(index);
   Int_t             idxFrom = getFirstIndexBeforeY(yGoal);
   Int_t             idxTo = getLastIndexAfterY(yGoal);
   
   neighbours->reserve(8);

   for (Int_t j=idxFrom; j < idxTo; j++) {
      if (index == j) continue;
      if (neighbours->size() == 8) break;

      if (abs(xGoal - getX(j)) <= 1 && abs(yGoal - getY(j)) <= 1) {
         neighbours->push_back(j);
      }
   }
   return neighbours;
}

vector<Int_t> * Hits::getAllNeighboursFromCluster(Int_t i, vector<Int_t> *checkedIndices) {
   // TODO: Optimize this function
   // Use static arrays, and a counter instead of toCheck->empty();

   vector<Int_t>   * expandedCluster = new vector<Int_t>;
   vector<Int_t>   * toCheck = new vector<Int_t>;
   vector<Int_t>   * nextCandidates = nullptr;
   Int_t             currentCandidate = 0;
   
   expandedCluster->reserve(40);
   toCheck->reserve(40);

   expandedCluster->push_back(i);
   toCheck->push_back(i);
   
   while (!toCheck->empty()) {
      currentCandidate = toCheck->back();    
      toCheck->pop_back();
      
      checkedIndices->push_back(currentCandidate);
      
      nextCandidates = findNeighbours(currentCandidate);
      checkAndAppendAllNextCandidates(nextCandidates, checkedIndices, toCheck, expandedCluster);
      delete nextCandidates;
   }

   delete toCheck;

   return expandedCluster;
}

void Hits::appendNeighboursToClusters(vector<Int_t> *expandedCluster, Clusters *clusters) {
   Float_t  sumX = 0;
   Float_t  sumY = 0;
   Int_t    idx = 0;
   Int_t    cSize = expandedCluster->size();
   Int_t    layerNo = getLayer(expandedCluster->at(0));

   for (Int_t j=0; j<cSize; j++) {
      idx = expandedCluster->at(j);
      sumX += getX(idx) - 0.5; // -0.5  to get
      sumY += getY(idx) - 0.5; // pixel center
   }
   clusters->appendCluster(sumX / cSize, sumY / cSize, layerNo, cSize);
}

void Hits::checkAndAppendAllNextCandidates(vector<Int_t> * nextCandidates, vector<Int_t> *checkedIndices,
         vector<Int_t> *toCheck, vector<Int_t> *expandedCluster) {

   Bool_t   inChecked;
   Bool_t   inToCheck;
   Int_t    nextCandidate;
   
   while (!nextCandidates->empty()) {
      nextCandidate = nextCandidates->back();
      nextCandidates->pop_back();

      inChecked = isItemInVector(nextCandidate, checkedIndices);
      inToCheck = isItemInVector(nextCandidate, toCheck);

      if (!inChecked && !inToCheck) {
         expandedCluster->push_back(nextCandidate);
         toCheck->push_back(nextCandidate);
      }
   }
}

vector<Hits*> * Hits::findClustersHitMap() {
   vector<Hits*> *clusterHitMap = new vector<Hits*>;
   vector<Int_t> *checkedIndices = new vector<Int_t>;
   vector<Int_t> *expandedCluster = 0;
   checkedIndices->reserve(GetEntriesFast());

   for (Int_t i = 0; i < GetEntriesFast(); i++) {
      if (isItemInVector(i, checkedIndices)) continue;

      vector<Int_t> * firstHits = findNeighbours(i);
      if (firstHits->size()) {
         expandedCluster = getAllNeighboursFromCluster(i, checkedIndices);
         appendExpandedClusterToClusterHitMap(expandedCluster, clusterHitMap);
         delete expandedCluster;
         delete firstHits;
      }
   }

   delete checkedIndices;
   return clusterHitMap;
}

void Hits::appendExpandedClusterToClusterHitMap(vector<Int_t> *expandedCluster, vector<Hits*> *clusterHitMap) {
   Float_t sumX = 0; Float_t sumY = 0;
   Float_t x; Float_t y;
   Int_t idx;
   Int_t cSize = expandedCluster->size();
   
   for (Int_t j=0; j<cSize; j++) {
      Int_t idx = expandedCluster->at(j);
      sumX += getX(idx) - 0.5;
      sumY += getY(idx) - 0.5;
   }

   // center cluster hitmap
   Int_t offsetForHistogramX = 5 - sumX/cSize;
   Int_t offsetForHistogramY = 5 - sumY/cSize;

   clusterHitMap->push_back(new Hits(cSize));

   Int_t cidx = clusterHitMap->size() - 1;   
   for (Int_t j=0; j<cSize; j++) {
      idx = expandedCluster->at(j);
      x = offsetForHistogramX + getX(idx);
      y = offsetForHistogramY + getY(idx);

      clusterHitMap->at(cidx)->appendPoint(x, y);
   }
}
