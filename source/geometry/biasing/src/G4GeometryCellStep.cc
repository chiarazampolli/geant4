//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: G4GeometryCellStep.cc,v 1.1 2002/10/22 13:18:46 dressel Exp $
// GEANT4 tag $Name: geant4-05-02-patch-01 $
//
// ----------------------------------------------------------------------
// GEANT 4 class source file
//
// G4GeometryCellStep.cc
//
// ----------------------------------------------------------------------

#include "G4GeometryCellStep.hh"

G4GeometryCellStep::G4GeometryCellStep(const G4GeometryCell &preCell, 
		 const G4GeometryCell &postCell)
  : 
  fPreGeometryCell(preCell), 
  fPostGeometryCell(postCell), 
  fCrossBoundary(false) 
{}

G4GeometryCellStep::~G4GeometryCellStep()
{}