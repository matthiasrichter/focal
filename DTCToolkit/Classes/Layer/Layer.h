#ifndef Layer_h
#define Layer_h

#include <TH2F.h>

#include "Classes/Cluster/Clusters.h"
#include "Classes/Hit/Hits.h"
#include "../../GlobalConstants/Constants.h"

class TRandom3;

class Layer : public TObject {
   private:
      TH2F frame2D_;
      Int_t layerNo_;
      Bool_t frameType_;
      Bool_t dataType_;

   public:
      // ROOT & I/O
      Layer(Int_t layerNo, Bool_t frameType, Bool_t dataType);
      virtual        ~Layer();
      virtual void   Reset() { frame2D_.Reset(); }
      virtual void   Fill(Float_t x, Float_t y, Float_t val = 1) { frame2D_.Fill(x,y,val); }
      virtual TH2F * getTH2F() { return (TH2F*) &frame2D_; }
      
      Int_t          diffuseLayer(TRandom3 *gRandom);
      virtual Bool_t findHits(Hits *hits);
      Float_t        getOccupancy();
      
      ClassDef(Layer,2)
};

#endif /* Layer_h */
