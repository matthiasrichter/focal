#ifndef Track_h
#define Track_h

#include <TClonesArray.h>
#include <vector>

#include "Classes/Cluster/Cluster.h"
// #include "Classes/Track/conversionFunctions.h"

const Int_t MaxTrackLength = 40;
class TGraph;
class TGraphErrors;
class Clusters;

class Track : public TObject {

   private:
      TClonesArray track_;
      Float_t  fitEnergy_;
      Float_t  fitRange_;
      Float_t  fitScale_;
      Float_t  fitError_;

   public:
      Track();
      Track(Cluster *cluster);

      virtual ~Track(); 

      // ROOT & I/O
      virtual Cluster * At(Int_t i)                      { return ((Cluster*) track_.At(i)); }
      virtual Cluster * Last()                           { return ((Cluster*) track_.At(GetEntriesFast()-1)); }
      virtual Int_t     GetEntriesFast()                 { return track_.GetEntriesFast(); }
      virtual Int_t     GetEntries()                     { return track_.GetEntries(); }
      virtual void      Compress()                       { track_.Compress(); }
      virtual void      clearTrack()                     { track_.Clear("C"); }
      virtual void      Clear(Option_t * = "")           { track_.Clear("C"); }
   
      // Add and remove clusters
      virtual TObject * removeClusterAt(Int_t i)         { return track_.RemoveAt(i); }
      virtual void      removeCluster(Cluster *c)        { track_.Remove((TObject*) c); }
      virtual void      setTrack(Track *copyTrack, Int_t startOffset = 0); // copy whole track
      virtual void      appendCluster(Cluster *copyCluster, Int_t startOffset = 0); // copy cluster
      virtual void      appendPoint(Float_t x, Float_t y, Int_t layer, Int_t size = -1, Int_t eventID = -1);

      // Getters and setters
      virtual Float_t   getX(Int_t i)                    { return At(i)->getX(); }
      virtual Float_t   getY(Int_t i)                    { return At(i)->getY(); }
      virtual Int_t     getLayer(Int_t i)                { return At(i)->getLayer(); }
      Float_t           getXmm(Int_t i)                  { return At(i)->getXmm(); }
      Float_t           getYmm(Int_t i)                  { return At(i)->getYmm(); }
      Float_t           getLayermm(Int_t i)              { return At(i)->getLayermm(); }
      virtual Int_t     getSize(Int_t i)                 { return At(i)->getSize(); }
      virtual Int_t     getEventID(Int_t i)              { return At(i)->getEventID(); }
      virtual Int_t     getClusterSizeError(Int_t i)     { return At(i)->getError(); } 
      virtual Float_t   getDepositedEnergy(Int_t i, Bool_t c = false)      { return At(i)->getDepositedEnergy(c); }
      virtual Float_t   getDepositedEnergyError(Int_t i, Bool_t c = false) { return At(i)->getDepositedEnergyError(c); }
      Bool_t            isUsed(Int_t i)                  { return At(i)->isUsed(); }
      
      // TRACK PROPERTIES - Event IDs
      Int_t             getModeEventID(); // (vs. median)
      Bool_t            isOneEventID();
      Bool_t            isFirstAndLastEventIDEqual();

      // TRACK PROPERTIES - General track properties
      // trackProperties.C
      Int_t             getFirstLayer();
      Int_t             getLastLayer();
      Bool_t            hasLayer(Int_t layer);
      Int_t             getIdxFromLayer(Int_t i);
      Bool_t            doesTrackEndAbruptly();
      Float_t           getRiseFactor();
      Int_t             getNMissingLayers();

      // TRACK PROPERTIES - Ranges and energies
      // trackRangeCalculations.C
      Float_t           getTrackLengthmm();
      Float_t           getTrackLengthmmAt(Int_t i);  // TL between i-1 and i in mm
      Float_t           getTrackLengthWEPLmmAt(Int_t i);
      Float_t           getRangemm();
      Float_t           getRangemmAt(Int_t i);
      Float_t           getRangeWEPLAt(Int_t i);
      Float_t           getWEPL();
      Float_t           getEnergy();
      Float_t           getEnergyStraggling();

      // TRACK PROPERTIES - Angles
      // trackAngleCalculations.C
      Float_t           getSlopeAngleAtLayer(Int_t i);
      Float_t           getSlopeAngleBetweenLayers(Int_t i);
      Float_t           getSlopeAngle();
      Float_t           getSlopeAngleChangeBetweenLayers(Int_t i);
      Float_t           getSlopeAngleDifferenceSum();
      Float_t           getSlopeAngleDifferenceSumInTheta0();
      Float_t           getSinuosity();
      Float_t           getProjectedRatio();
      Float_t           getMaximumSlopeAngleChange();
      Float_t           getAbsorberLength(Int_t i);

      // TRACK PROPERTIES - Cluster properties
      // trackClusterProperties.C
      Int_t             getClusterIdx(Cluster * cluster);
      Int_t             getClusterIdx(Float_t x, Float_t y, Int_t layer);
      Bool_t            isClusterInTrack(Cluster * cluster);
      Int_t             getClusterFromLayer(Int_t layer);
      Clusters        * getConflictClusters();
      Int_t             getNumberOfConflictClusters();
      Bool_t            isUsedClustersInTrack();
      Float_t           getAverageCS();
      Float_t           getAverageCSLastN(Int_t i);
      Float_t           getMeanSizeToIdx(Int_t i);
      Float_t           getStdSizeToIdx(Int_t toIdx);


      // TRACK PROPERTIES - Pre-sensor material
      // trackPreSensorMaterial.C
      Bool_t            isHitOnScintillatorH();
      Bool_t            isHitOnScintillatorV();
      Bool_t            isHitOnScintillators();
      Int_t             getNScintillators();
      Float_t           getPreEnergyLoss();
      Float_t           getPreEnergyLossError();
      Float_t           getPreTL();
      Float_t           getPreWEPL();

      // TRACK PROPERTIES - Extrapolations
      // trackExtrapolations.C
      Cluster         * getInterpolatedClusterAt(Int_t layer);
      Cluster         * getExtrapolatedClusterAt(Float_t mmBeforeDetector);
      vector<Float_t>   getLateralDeflectionFromExtrapolatedPosition(Int_t layer);
      void              extrapolateToLayer0();

      // TRACK PROPERTIES - Track fitting, scoring
      // trackFitting.C
      TGraphErrors    * doFit();
      TGraphErrors    * doRangeFit(Bool_t isScaleVariable = false);
      Float_t           getFitParameterRange();
      Float_t           getFitParameterScale();
      Float_t           getFitParameterError();
      Float_t           getTrackScore();

      friend ostream& operator<<(ostream &os, Track& t);

      ClassDef(Track,5)
};
#endif
