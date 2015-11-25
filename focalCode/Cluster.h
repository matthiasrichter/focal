#ifndef Cluster_h
#define Cluster_h

#include "TObject.h"
#include "Constants.h"
#include <iostream>

class Cluster : public TObject {
private:
	Float_t x_, y_;
	Int_t layerNo_;
	Int_t clusterSize_;

public:
	Cluster() { x_ = -1; y_ = -1; layerNo_ = -1; clusterSize_ = -1;}
	Cluster(Cluster *cluster);
	Cluster(Float_t x, Float_t y, Int_t layer = -1, Int_t size = -1);
	virtual ~Cluster();

	Float_t getX() { return x_; }
	Float_t getY() { return y_; }
	Int_t getLayer() { return layerNo_; }
	Int_t getSize() { return clusterSize_; }
	Float_t getRadiusmm();
	Float_t getDepositedEnergy();

	Double_t getXmm() { return x_ * dx; }
	Double_t getYmm() { return y_ * dy; }
	Double_t getLayermm(); // take into account tracker positions

	void set(Float_t x, Float_t y, Int_t layer = -1, Int_t size = -1);
	void set(Cluster *copyCluster); // copy properties, not pointer

	void setX(Float_t x) { x_ = x; }
	void setY(Float_t y) { y_ = y; }
	void setLayer(Int_t layer) { layerNo_ = layer; }
	void setSize(Int_t size) { clusterSize_ = size; }

	void setXmm(Float_t x) { x_ = x / dx; }
	void setYmm(Float_t y) { y_ = y / dy; }

	friend ostream& operator<<(ostream &os, Cluster &c);

	ClassDef(Cluster,1);
};

#endif
