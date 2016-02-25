/**************************************************************************
 * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
#include <TClonesArray.h>
#include <TObjArray.h>

#include <AliAODEvent.h>
#include <AliAODTrack.h>
#include <AliESDtrack.h>
#include <AliPicoTrack.h>
#include "AliEMCalPtTaskTrackSelectionAOD.h"

/// \cond CLASSIMP
ClassImp(EMCalTriggerPtAnalysis::AliEMCalPtTaskTrackSelectionAOD)
/// \endcond

namespace EMCalTriggerPtAnalysis {

/**
 * Main constructor, initialises fields with 0 (or NULL). For ROOT I/O, not intended
 * to be used by the users.
 */
AliEMCalPtTaskTrackSelectionAOD::AliEMCalPtTaskTrackSelectionAOD() :
	AliEMCalPtTaskVTrackSelection(),
	fFilterBits(0)
{
}

/**
 * Main Constructor, initalising also track cuts and filter bits. In case the initial cuts
 * is a nullpointer, only filter bits are used for the track selection. This constructor is
 * intended to be used by the users.
 *
 * \param cuts Inital track cut object (of type AliESDtrackCuts, can be a nullpointer)
 * \param filterbits Filter bits required
 */
AliEMCalPtTaskTrackSelectionAOD::AliEMCalPtTaskTrackSelectionAOD(AliESDtrackCuts* cuts, UInt_t filterbits):
	AliEMCalPtTaskVTrackSelection(),
	fFilterBits(filterbits)
{
  AddTrackCuts(cuts);
}

/**
 * Select tracks from a list (TClonesArray) of tracks. The actual selection process
 * is delegated to the function IsTrack selected. This function collects all the tracks
 * which are accepted and puts them into a TObjArray. Note that this class keeps ownership
 * over the resulting TObjArray.
 *
 * \param tracks TClonesArray of input tracks, under which we select the appropriate ones
 * \return TObjArray of selected tracks
 */
TObjArray* AliEMCalPtTaskTrackSelectionAOD::GetAcceptedTracks(const TClonesArray* const tracks) {
	if(!fListOfTracks) fListOfTracks = new TObjArray;
	else fListOfTracks->Clear();
	TIter trackIter(tracks);
	AliVTrack *track(NULL);
	while((track = dynamic_cast<AliVTrack *>(trackIter()))){
	  if(IsTrackAccepted(track)) fListOfTracks->AddLast(track);
	}
	return fListOfTracks;
}

	/**
	 * Select tracks from an input event. The actual selection process is delegated to
	 * the function IsTrack selected. This function collects all the tracks which are
	 * accepted and puts them into a TObjArray. Note that this class keeps ownership over
	 * the resulting TObjArray.
	 *
	 * \param event Input event under which we select the appropriate tracks
	 * \return TObjArray of selected tracks
	 */
	TObjArray* AliEMCalPtTaskTrackSelectionAOD::GetAcceptedTracks(const AliVEvent* const event) {
		if(!fListOfTracks) fListOfTracks = new TObjArray;
		else fListOfTracks->Clear();
		const AliAODEvent *aod = dynamic_cast<const AliAODEvent *>(event);
		if(!aod){
			AliError("Event not of type AliAODEvent");
			return fListOfTracks;
		}
		for(int itrk = 0; itrk < event->GetNumberOfTracks(); itrk++){
		  AliVTrack *trk = dynamic_cast<AliVTrack *>(event->GetTrack(itrk));
		  if(IsTrackAccepted(trk)) fListOfTracks->AddLast(trk);
		}
		return fListOfTracks;
	}

/**
 * Function checks whether track is accepted under the given track selection cuts.
 * The function can handle AliAODTrack and AliPicoTrack, while for AliPico track an
 * AliAODTrack is expected to be the underlying structure. If it is not possible to
 * access an AOD track from the input track, the object will not be selected. Otherwise
 * first the status bits are checked (if requested), and if further track cuts (of type
 * AliESDtrackCuts) are provided, the track is converted to an ESD track for further checks.
 *
 * \param trk: Track to check
 * \return true if selected, false otherwise
 */
bool AliEMCalPtTaskTrackSelectionAOD::IsTrackAccepted(AliVTrack * const trk){
  AliAODTrack *aodt = dynamic_cast<AliAODTrack *>(trk);
  if(!aodt){
    AliPicoTrack *picotrack = dynamic_cast<AliPicoTrack *>(trk);
    if(picotrack) aodt = dynamic_cast<AliAODTrack *>(picotrack->GetTrack());
    else{
      AliError("Track neither AOD track nor pico track");
      return kFALSE;
    }
  }
  if(!aodt){
    AliError("Failed getting AOD track");
    return kFALSE;
  }
  if(fFilterBits && !aodt->TestFilterBit(fFilterBits)) return kFALSE;
  Bool_t result = kTRUE;
  if(fListOfCuts){
    for(TIter cutIter = TIter(fListOfCuts).Begin(); cutIter != TIter::End(); ++cutIter){
      AliVCuts *trackCuts = static_cast<AliVCuts *>(*cutIter);
      if(trackCuts->IsA() == AliESDtrackCuts::Class()){
        // If track cuts are AliESDtrackCuts, the track needs to be converted to an AliESDtrack before
        AliESDtrack copyTrack(aodt);
        if(!trackCuts->IsSelected(&copyTrack)) result = kFALSE;
      } else{
        if(!trackCuts->IsSelected(aodt)) result = kFALSE;
      }
    }

  }
  return result;
}

} /* namespace EMCalTriggerPtAnalysis */
