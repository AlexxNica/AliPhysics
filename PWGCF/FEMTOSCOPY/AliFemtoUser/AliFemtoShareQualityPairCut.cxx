/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// AliFemtoShareQualityPairCut - a pair cut which checks for some pair     //
// qualities that attempt to identify slit/doubly reconstructed tracks     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
/***************************************************************************
 *
 * $Id: AliFemtoShareQualityPairCut.cxx 53713 2011-12-20 12:31:21Z akisiel $
 *
 * Author: Adam Kisiel, Ohio State, kisiel@mps.ohio-state.edu
 ***************************************************************************
 *
 * Description: part of STAR HBT Framework: AliFemtoMaker package
 *   a cut to remove "shared" and "split" pairs
 *
 ***************************************************************************
 *
 *
 **************************************************************************/

#include "AliFemtoShareQualityPairCut.h"
#include <string>
#include <cstdio>

#ifdef __ROOT__
ClassImp(AliFemtoShareQualityPairCut)
#endif

//__________________
AliFemtoShareQualityPairCut::AliFemtoShareQualityPairCut():
  fNPairsPassed(0),
  fNPairsFailed(0),
  fShareQualityMax(1.0),
  fShareFractionMax(1.0),
  fRemoveSameLabel(0)
{
  // Default constructor
  // Nothing to do
}
//__________________
AliFemtoShareQualityPairCut::~AliFemtoShareQualityPairCut(){
  /* no-op */
}
AliFemtoShareQualityPairCut& AliFemtoShareQualityPairCut::operator=(const AliFemtoShareQualityPairCut& cut)
{
  if (this != &cut) {
    AliFemtoPairCut::operator=(cut);
    fNPairsPassed = 0;
    fNPairsFailed = 0;
    fShareQualityMax = cut.fShareQualityMax;
    fShareFractionMax = cut.fShareFractionMax;
    fRemoveSameLabel = cut.fRemoveSameLabel;
  }

  return *this;
}
//__________________
bool AliFemtoShareQualityPairCut::Pass(const AliFemtoPair* pair){
  // Check for pairs that are possibly shared/double reconstruction
  bool temp;

  Int_t nh = 0;
  Int_t an = 0;
  Int_t ns = 0;

  if ((fShareFractionMax >= 1.0) && ( fShareQualityMax >= 1.0)) {
     temp = true;
  }
  else {
    const unsigned int n_bits = pair->Track1()->Track()->TPCclusters().GetNbits();

    const auto tpc_clusters_1 = pair->Track1()->Track()->TPCclusters(),
               tpc_clusters_2 = pair->Track2()->Track()->TPCclusters();

    const auto tpc_sharing_1 = pair->Track1()->Track()->TPCsharing(),
               tpc_sharing_2 = pair->Track2()->Track()->TPCsharing();

     for (unsigned int imap = 0; imap < n_bits; imap++) {
       const bool cluster_bit_1 = tpc_clusters_1.TestBitNumber(imap),
                  cluster_bit_2 = tpc_clusters_1.TestBitNumber(imap);
        // If both have clusters in the same row
        if (cluster_bit_1 && cluster_bit_2) {
           // Do they share it ?
           if (tpc_sharing_1.TestBitNumber(imap) && tpc_sharing_2.TestBitNumber(imap))
           {
              //	  cout << "A shared cluster !!!" << endl;
              //	cout << "imap idx1 idx2 "
              //	     << imap << " "
              //	     << tP1idx[imap] << " " << tP2idx[imap] << endl;
              an++;
              nh+=2;
              ns+=2;
           }
           // Different hits on the same padrow
           else {
              an--;
              nh+=2;
           }
        }
        else if (cluster_bit_1 || cluster_bit_2) {
           // One track has a hit, the other does not
           an++;
           nh++;
        }
     }
     Float_t hsmval = 0.0;
     Float_t hsfval = 0.0;

     if (nh >0) {
        hsmval = an*1.0/nh;
        hsfval = ns*1.0/nh;
     }
    //  if (hsmval > -0.4) {
    //   cout << "Pair quality: " << hsmval << " " << an << " " << nh << " "
    //        << (pair->Track1()->Track()) << " "
    //        << (pair->Track2()->Track()) << endl;
    //   cout << "Bits: " << pair->Track1()->Track()->TPCclusters().GetNbits() << endl;
    //  }
    //   if (hsfval > 0.0) {
    //     cout << "Pair sharity: " << hsfval << " " << ns << " " << nh << "    " << hsmval << " " << an << " " << nh << endl;
    //   }
     if( (fShareQualityMax < 1) && (fShareFractionMax < 1)  )
        temp = (hsmval < fShareQualityMax) && (hsfval < fShareFractionMax);
     else if (fShareQualityMax < 1)
        temp = (hsmval < fShareQualityMax);
     else if (fShareFractionMax < 1)
        temp = (hsmval < fShareFractionMax);
     else temp = false;
  }

  if (fRemoveSameLabel) {
    if (abs(pair->Track1()->Track()->Label()) == abs(pair->Track2()->Track()->Label())) {
//       cout << "Found a pair with same label " << pair->Track1()->Track()->Label() << endl;
//       cout << "Quality Sharity Passed " << hsmval << " " << hsfval << " " << pair->QInv() << " " << temp << endl;
      temp = kFALSE;
    }
  }

  temp ? fNPairsPassed++ : fNPairsFailed++;
  return temp;
}
//__________________
AliFemtoString AliFemtoShareQualityPairCut::Report()
{
  // Prepare the report from the execution
  string stemp = "AliFemtoShareQuality Pair Cut - remove shared and split pairs\n";  char ctemp[100];
  snprintf(ctemp , 100, "Number of pairs which passed:\t%ld  Number which failed:\t%ld\n",fNPairsPassed,fNPairsFailed);
  stemp += ctemp;
  AliFemtoString returnThis = stemp;
  return returnThis;}
//__________________

void AliFemtoShareQualityPairCut::SetShareQualityMax(Double_t aShareQualityMax) {
  fShareQualityMax = aShareQualityMax;
}

Double_t AliFemtoShareQualityPairCut::GetAliFemtoShareQualityMax() const {
  return fShareQualityMax;
}

void AliFemtoShareQualityPairCut::SetShareFractionMax(Double_t aShareFractionMax) {
  fShareFractionMax = aShareFractionMax;
}
Double_t AliFemtoShareQualityPairCut::GetAliFemtoShareFractionMax() const {
  return fShareFractionMax;
}

TList *AliFemtoShareQualityPairCut::ListSettings()
{
  // return a list of settings in a writable form
  TList *tListSetttings = new TList();
  char buf[200];
  snprintf(buf, 200, "AliFemtoShareQualityPairCut.sharequalitymax=%f", fShareQualityMax);
  snprintf(buf, 200, "AliFemtoShareQualityPairCut.sharefractionmax=%f", fShareFractionMax);
  tListSetttings->AddLast(new TObjString(buf));

  return tListSetttings;
}

void     AliFemtoShareQualityPairCut::SetRemoveSameLabel(Bool_t aRemove)
{
  fRemoveSameLabel = aRemove;
}
