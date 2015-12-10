#ifndef Track_h
#define Track_h
#include "TClonesArray.h"
#include "Cluster.h"

const Int_t MaxTrackLength = 40;
struct Fit {
	Float_t energy;
	Float_t scale;
};

class Track : public TObject {

   private:
      TClonesArray track_;
      Fit fitValues_;

   public:

      Track() : track_("Cluster", MaxTrackLength) {
		  fitValues_.energy = 0;
		  fitValues_.scale = 0;
	  };

      Track(Cluster *cluster) : track_("Cluster", MaxTrackLength) { 
      	appendCluster(cluster);
      	fitValues_.energy = 0;
		fitValues_.scale = 0;
      }

      virtual ~Track(); 

      virtual void setTrack(Track *copyTrack, Int_t startOffset = 0); // copy whole track
      virtual void appendCluster(Cluster *copyCluster, Int_t startOffset = 0); // copy cluster
      virtual void appendPoint(Float_t x, Float_t y, Int_t layer, Int_t size = -1);

      virtual void clearTrack() { track_.Clear("C"); }
      virtual void Clear(Option_t * = "");

      virtual Int_t GetEntriesFast() { return track_.GetEntriesFast(); }
      virtual Int_t GetEntries() { return track_.GetEntries(); }
      
      virtual Float_t getX(Int_t i) { return At(i)->getX(); }
      virtual Float_t getY(Int_t i) { return At(i)->getY(); }
      virtual Int_t getLayer(Int_t i) { return At(i)->getLayer(); }
      virtual Int_t getSize(Int_t i) { return At(i)->getSize(); }
      virtual Float_t getDepositedEnergy(Int_t i) {return At(i)->getDepositedEnergy(); }

		// with dimensions
		Float_t getXmm(Int_t i) { return At(i)->getXmm(); }
		Float_t getYmm(Int_t i) { return At(i)->getYmm(); }
		Float_t getLayermm(Int_t i) { return At(i)->getLayermm(); }
		Float_t getTrackLengthmm();
		Float_t getTrackLengthmmAt(Int_t i);	// return length between two points in mm
		Float_t getTrackLengthWEPLmmAt(Int_t i);
		Float_t getSinuosity();
		Float_t getSlopeAngle();
		Float_t getSlopeAngleAtLayer(Int_t i);
		Float_t getSnakeness();
		Float_t getTrackScore();
		Float_t getMeanSizeToIdx(Int_t i);
		Float_t getStdSizeToIdx(Int_t toIdx);
		Float_t getEnergyFromWEPL(Float_t wepl);
		Float_t getEnergyFromTL(Float_t tl);
		Float_t getEnergy();
		Float_t getFitParameterEnergy();
		Float_t getFitParameterScale();
		Float_t getWEPLFromTL(Float_t tl);
		Float_t getWEPL();
		Float_t getWEPLFactorFromEnergy ( Float_t energy );

      Int_t getNMissingLayers();
      Int_t getLastLayer();
      Int_t getFirstLayer();
      Bool_t hasLayer(Int_t layer);
      Float_t diffmm(Cluster *p1, Cluster *p2);

      Float_t getAverageCS();
      Float_t getAverageCSLastN(Int_t i);

      Bool_t doFit();
	  
      virtual Cluster* At(Int_t i) { return ((Cluster*) track_.At(i)); }
      Int_t getClusterFromLayer( Int_t layer );
      Cluster * getInterpolatedClusterAt(Int_t layer);
		virtual Cluster* Last() { return ((Cluster*) track_.At(GetEntriesFast()-1)); }

      virtual TObject* removeClusterAt(Int_t i) { return track_.RemoveAt(i); }
      virtual void removeCluster(Cluster *c) { track_.Remove((TObject*) c); }

      virtual void extrapolateToLayer0();

   ClassDef(Track,2);
};
#endif
