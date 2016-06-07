// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: ExN02RunAction.hh,v 1.3 1998/10/09 14:31:06 japost Exp $
// GEANT4 tag $Name: geant4-00 $
//
// 

#ifndef ExN02RunAction_h
#define ExN02RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

class ExN02RunAction : public G4UserRunAction
{
  public:
    ExN02RunAction();
    ~ExN02RunAction();

  public:
    void BeginOfRunAction(G4Run* aRun);
    void EndOfRunAction(G4Run* aRun);

  private:
    G4int runIDcounter;
};

#endif
