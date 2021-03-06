#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>

#include <TObject.h>
#include <TSpline.h>

#include "GlobalConstants/Constants.h"
#include "GlobalConstants/MaterialConstants.h"
#include "GlobalConstants/RangeAndEnergyCalculations.h"

using namespace std;

Float_t getTLFromEnergy(Float_t energy, TSpline3 *spline) {
   return spline->Eval(energy);
}

Float_t getEnergyFromTL(Float_t range, TSpline3 *invSpline) {
   return invSpline->Eval(range);
}

Float_t getEnergyAtTL(Float_t E0, Float_t depth, TSpline3 *spline, TSpline3 *invSpline) {
   Float_t range = spline->Eval(E0);
   Float_t energy = invSpline->Eval(range - depth);
   return energy;
}

Float_t  getEnergyFromTL(Float_t range) {
   return getEnergyFromTL(range, splineMaterialInv);
}

Float_t getEnergyFromWEPL(Float_t wepl) {
   return getEnergyFromTL(wepl, splineWaterInv);
}

Float_t getEnergyAtTL(Float_t E0, Float_t depth) {
   return getEnergyAtTL(E0, depth, splineMaterial, splineMaterialInv);
}

Float_t getEnergyAtWEPL(Float_t E0, Float_t depth) {
   return getEnergyAtTL(E0, depth, splineWater, splineWaterInv);
}

Float_t getEnergyAtTLFromPureAluminum(Float_t E0, Float_t depth) {
   return getEnergyAtTL(E0, depth, splinePureAl, splinePureAlInv);
}

//////////////////////////
// Conversion to range  //
//////////////////////////

Float_t getTLFromEnergy(Float_t energy) {
   if (energy == 0) return 0;
   return getTLFromEnergy(energy, splineMaterial);
}

Float_t getWEPLFromEnergy(Float_t energy) {
   if (energy == 0) return 0;
   return getTLFromEnergy(energy, splineWater);
}

Float_t getTLFromWEPL(Float_t wepl) {
   if (wepl == 0) return 0;
   Float_t energy = getEnergyFromWEPL(wepl);
   Float_t tl = getTLFromEnergy(energy);

   return tl;
}

//////////////////////////
// Conversion to WEPL   //
//////////////////////////

Float_t getWEPLFactorFromEnergy(Float_t energy) {
   if (energy == 0) return 0;
   Float_t range = getTLFromEnergy(energy);
   Float_t wepl = getWEPLFromEnergy(energy);

//   return 2.15; // TEST
   return wepl / range;
}

Float_t getWEPLFromTL(Float_t tl) {
   if (tl == 0) return 0;
   Double_t energy = getEnergyFromTL(tl);
   Double_t wepl = getWEPLFromEnergy(energy);

   return wepl;
}


/////////////////////////////////////
// calculation of range straggling //
/////////////////////////////////////

Float_t getTLStragglingFromTL(Float_t tl, Float_t sigma_energy) {
   if (tl == 0) return 0;
   Float_t energy = getEnergyFromTL(tl);
   
   // The straggling is calculated empirically from fit from full MC data
   Float_t sigma_a = pow(straggling_a + tl * straggling_b, 2);
   Float_t sigma_b = pow(sigma_energy * alpha * p, 2) * pow(energy, 2*p-2);
   
   Float_t sigma = sqrt(sigma_a + sigma_b);
   
   return sigma;
}

Float_t getTLStragglingFromEnergy(Float_t energy, Float_t sigma_energy) {
   Float_t tl = getTLFromEnergy(energy);
   return getTLStragglingFromTL(tl, sigma_energy);
}

/////////////////////////////////////
// Conversion to WEPL straggling   //
/////////////////////////////////////

Float_t getWEPLStragglingFromWEPL(Float_t wepl, Float_t sigma_energy) {
   Float_t estimatedStraggling;

   Float_t tl = getTLFromWEPL(wepl);
   Float_t tl_straggling = getTLStragglingFromTL(tl, sigma_energy);

   Float_t tl_high = tl + tl_straggling / 2;
   Float_t tl_low = tl - tl_straggling / 2;

   Float_t wepl_high = getWEPLFromTL(tl_high);
   Float_t wepl_low = getWEPLFromTL(tl_low);

   estimatedStraggling = wepl_high - wepl_low;

   printf("getWEPLStragglingFromWEPL: TL = %.2f, WEPL = %.2f. TL straggling = %.2f (%.2f %%), WEPL straggling = %.2f (%.2f %%)\n", tl, wepl, tl_straggling, 100*tl_straggling/tl, estimatedStraggling, 100*estimatedStraggling/wepl);
   
   return estimatedStraggling;
}
   

Float_t getWEPLStragglingFromEnergy(Float_t energy, Float_t sigma_energy) {
   Float_t wepl = getWEPLFromEnergy(energy);
   return getWEPLStragglingFromWEPL(wepl, sigma_energy);
}  

///////////////////////////////////////
// Conversion to energy straggling   //
///////////////////////////////////////

Float_t getEnergyStragglingFromTL(Float_t tl, Float_t sigma_energy) {
   Float_t straggling = getTLStragglingFromTL (tl, sigma_energy);

   Float_t upper = tl + straggling / 2;
   Float_t lower = tl - straggling / 2;

   Float_t upper_energy = getEnergyFromTL(upper);
   Float_t lower_energy = getEnergyFromTL(lower);

   Float_t energy_straggling = (upper_energy - lower_energy);

   return energy_straggling;
}

Float_t getEnergyStragglingFromEnergy(Float_t energy, Float_t sigma_energy) {
   Float_t tl = getTLFromEnergy(energy);
   return getEnergyStragglingFromTL(tl, sigma_energy);
}

Float_t getEnergyStragglingFromTLStraggling(Float_t tl, Float_t TLStraggling) {
   Float_t upper = tl + TLStraggling / 2;
   Float_t lower = tl - TLStraggling / 2;

   Float_t upper_energy = getEnergyFromTL(upper);
   Float_t lower_energy = getEnergyFromTL(lower);

   Float_t energy_straggling = (upper_energy - lower_energy);

   return energy_straggling;
}

Float_t getEnergyStragglingFromWEPLStraggling(Float_t wepl, Float_t WEPLStraggling) {
   Float_t upper = wepl + WEPLStraggling / 2;
   Float_t lower = wepl - WEPLStraggling / 2;

   Float_t upper_energy = getEnergyFromWEPL(upper);
   Float_t lower_energy = getEnergyFromWEPL(lower);

   Float_t energy_straggling = (upper_energy - lower_energy);

   return energy_straggling;
}

/////////////////////////////////////////////
// Conversion to/from defined kOutputUnit  //
/////////////////////////////////////////////

Float_t getUnitFromEnergy(Float_t energy) {
   Float_t res = 0;
   if       (kOutputUnit == kEnergy)    res = energy;
   else if  (kOutputUnit == kPhysical)  res = getTLFromEnergy(energy);
   else if  (kOutputUnit == kWEPL)      res = getWEPLFromEnergy(energy);

   return res;
}

Float_t getUnitFromTL(Float_t tl) {
   Float_t res = 0;
   if       (kOutputUnit == kEnergy)   res = getEnergyFromTL(tl);
   else if  (kOutputUnit == kPhysical) res = tl;
   else if  (kOutputUnit == kWEPL)     res = getWEPLFromTL(tl);

   return res;
}

Float_t getUnitFromWEPL(Float_t wepl) {
   Float_t res = 0;
   if       (kOutputUnit == kEnergy)   res = getEnergyFromWEPL(wepl);
   else if  (kOutputUnit == kPhysical) res = getTLFromWEPL(wepl);
   else if  (kOutputUnit == kWEPL)     res = wepl;
   
   return res;
}

Float_t getEnergyFromUnit(Float_t unit) {
   Float_t res = 0;

   if       (kOutputUnit == kEnergy)   res = unit;
   else if  (kOutputUnit == kPhysical) res = getEnergyFromTL(unit);
   else if  (kOutputUnit == kWEPL)     res = getEnergyFromWEPL(unit);
   
   return res;
}

Float_t getEnergyAtUnit(Float_t unit, Float_t degraderthickness) {
   Float_t res = 0;

   if       (kOutputUnit == kEnergy)      res = getEnergyAtWEPL(getWEPLFromEnergy(unit), degraderthickness); // Eh?
   else if  (kOutputUnit == kWEPL)        res = getEnergyAtWEPL(unit, degraderthickness);
   else if  (kOutputUnit == kPhysical)    res = getEnergyAtTL(unit, degraderthickness);

   return res;
}

Float_t getUnitStragglingFromEnergy(Float_t energy, Float_t sigma_energy) {
   Float_t res = 0;
   if      (kOutputUnit == kEnergy)    res = getEnergyStragglingFromEnergy(energy, sigma_energy);
   else if (kOutputUnit == kWEPL)      res = getWEPLStragglingFromEnergy(energy, sigma_energy);
   else if (kOutputUnit == kPhysical)  res = getTLStragglingFromEnergy(energy, sigma_energy);

   return res;
}

Float_t getEnergyLossFromScintillators(Float_t energy, Int_t nScintillators) {
   Float_t energyLoss = 0;
   
   if      (nScintillators == 0) energyLoss = 0;
   else if (nScintillators == 1) energyLoss = 214.88444 * pow(energy, -0.7264956);
   else if (nScintillators == 2) energyLoss = 341.79255 * pow(energy, -0.7357198);
   else if (nScintillators == 3) energyLoss = 482.63531 * pow(energy, -0.7453624);
   
   else {
      cout << "ASSERTION ERROR ON NUMBER OF SCINTILLATORS!\n";
      energyLoss = 0;
   }
   
   return energyLoss;
}

Float_t getEnergyLossErrorFromScintillators(Int_t nScintillators) {
   Float_t energyLossError = 0;

   if       (nScintillators == 0) energyLossError = 0;
   else if (nScintillators == 1) energyLossError = 0.30;
   else if (nScintillators == 2) energyLossError = 0.38;
   else if (nScintillators == 3) energyLossError = 0.44;
   
   return energyLossError;
}

Float_t getEnergyLossFromAluminumAbsorber(Float_t energy) {
   // We assume that the energy loss function is constant in the high energy range
   // (somewhat true)
   // and calculate the energy loss of kAbsorbatorThickness mm Aluminum by scaling the
   // measured MC energy loss in a 1.5 mm Al absorber

   return  62.00441 * pow(energy, -0.7158403) * (kAbsorbatorThickness/1.5);
}

Float_t getEnergyLossErrorFromAluminumAbsorber() {
   return 0.135;
}
