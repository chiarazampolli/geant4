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
// *                                                                  *
// * Parts of this code which have been  developed by QinetiQ Ltd     *
// * under contract to the European Space Agency (ESA) are the	      *
// * intellectual property of ESA. Rights to use, copy, modify and    *
// * redistribute this software for general public use are granted    *
// * in compliance with any licensing, distribution and development   *
// * policy adopted by the Geant4 Collaboration. This code has been   *
// * written by QinetiQ Ltd for the European Space Agency, under ESA  *
// * contract 17191/03/NL/LvH (Aurora Programme). 		      *
// *                                                                  *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// MODULE:              G4WilsonAbrasionModel.cc
//
// Version:		B.1
// Date:		15/04/04
// Author:		P R Truscott
// Organisation:	QinetiQ Ltd, UK
// Customer:		ESA/ESTEC, NOORDWIJK
// Contract:		17191/03/NL/LvH
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// CHANGE HISTORY
// --------------
//
// 6 October 2003, P R Truscott, QinetiQ Ltd, UK
// Created.
//
// 15 March 2004, P R Truscott, QinetiQ Ltd, UK
// Beta release
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
////////////////////////////////////////////////////////////////////////////////
//
#include "G4WilsonAbrasionModel.hh"
#include "G4WilsonRadius.hh"
#include "G4NuclearAbrasionGeometry.hh"
#include "G4WilsonAblationModel.hh"

#include "G4ExcitationHandler.hh"
#include "G4Evaporation.hh"
#include "G4FermiBreakUp.hh"
#include "G4StatMF.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
#include "Randomize.hh"
#include "G4Fragment.hh"
#include "G4VNuclearDensity.hh"
#include "G4NuclearShellModelDensity.hh"
#include "G4NuclearFermiDensity.hh"
#include "G4FermiMomentum.hh"
#include "G4ReactionProductVector.hh"
#include "G4LorentzVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4Poisson.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "globals.hh"
////////////////////////////////////////////////////////////////////////////////
//
G4WilsonAbrasionModel::G4WilsonAbrasionModel (G4bool useAblation1)
{
//
//
// Send message to stdout to advise that the G4Abrasion model is being used.
//
  PrintWelcomeMessage();
//
//
// Set the default verbose level to 0 - no output.
//
  verboseLevel = 0;
  useAblation  = useAblation1;
//
//
// No de-excitation handler has been supplied - define the default handler.
//
  theExcitationHandler  = new G4ExcitationHandler;
  theExcitationHandlerx = new G4ExcitationHandler;
  if (useAblation)
  {
    theAblation = new G4WilsonAblationModel;
    theAblation->SetVerboseLevel(verboseLevel);
    theExcitationHandler->SetEvaporation(theAblation);
    theExcitationHandlerx->SetEvaporation(theAblation);
  }
  else
  {
    theAblation                      = NULL;
    G4Evaporation * theEvaporation   = new G4Evaporation;
    G4FermiBreakUp * theFermiBreakUp = new G4FermiBreakUp;
    G4StatMF * theMF                 = new G4StatMF;
    theExcitationHandler->SetEvaporation(theEvaporation);
    theExcitationHandler->SetFermiModel(theFermiBreakUp);
    theExcitationHandler->SetMultiFragmentation(theMF);
    theExcitationHandler->SetMaxAandZForFermiBreakUp(12, 6);
    theExcitationHandler->SetMinEForMultiFrag(5.0*MeV);

    theEvaporation  = new G4Evaporation;
    theFermiBreakUp = new G4FermiBreakUp;
    theExcitationHandlerx->SetEvaporation(theEvaporation);
    theExcitationHandlerx->SetFermiModel(theFermiBreakUp);
    theExcitationHandlerx->SetMaxAandZForFermiBreakUp(12, 6);
  }
//
//
// Set the minimum and maximum range for the model (despite nomanclature, this
// is in energy per nucleon number).  
//
  SetMinEnergy(70.0*MeV);
  SetMaxEnergy(10.1*GeV);
  isBlocked = false;
//
//
// npK, when mutiplied by the nuclear Fermi momentum, determines the range of
// momentum over which the secondary nucleon momentum is sampled.
//
  npK              = 5.0;
  B                = 10.0 * MeV;
  third            = 1.0 / 3.0;
  conserveEnergy   = false;
  conserveMomentum = true;
}
////////////////////////////////////////////////////////////////////////////////
//
G4WilsonAbrasionModel::G4WilsonAbrasionModel (G4ExcitationHandler *aExcitationHandler)
{
//
//
// Send message to stdout to advise that the G4Abrasion model is being used.
//
  PrintWelcomeMessage();
//
//
// Set the default verbose level to 0 - no output.
//
  verboseLevel = 0;
//                      
//
// The user is able to provide the excitation handler as well as an argument
// which is provided in this instantiation is used to determine
// whether the spectators of the interaction are free following the abrasion.
//
  theExcitationHandler             = aExcitationHandler;
  theExcitationHandlerx            = new G4ExcitationHandler;
  G4Evaporation * theEvaporation   = new G4Evaporation;
  G4FermiBreakUp * theFermiBreakUp = new G4FermiBreakUp;
  theExcitationHandlerx->SetEvaporation(theEvaporation);
  theExcitationHandlerx->SetFermiModel(theFermiBreakUp);
  theExcitationHandlerx->SetMaxAandZForFermiBreakUp(12, 6);
//
//
// Set the minimum and maximum range for the model (despite nomanclature, this
// is in energy per nucleon number).  
//
  SetMinEnergy(70.0*MeV);
  SetMaxEnergy(10.1*GeV);
  isBlocked = false;
//
//
// npK, when mutiplied by the nuclear Fermi momentum, determines the range of
// momentum over which the secondary nucleon momentum is sampled.
//
  npK              = 5.0;
  B                = 10.0 * MeV;
  third            = 1.0 / 3.0;
  conserveEnergy   = false;
  conserveMomentum = true;
}
////////////////////////////////////////////////////////////////////////////////
//
G4WilsonAbrasionModel::~G4WilsonAbrasionModel ()
{
//
//
// The destructor doesn't have to do a great deal!
//
  delete theExcitationHandler;
  delete theExcitationHandlerx;
}
////////////////////////////////////////////////////////////////////////////////
//
G4HadFinalState *G4WilsonAbrasionModel::ApplyYourself (
  const G4HadProjectile &theTrack, G4Nucleus &theTarget)
{
//
//
// The secondaries will be returned in G4HadFinalState &theParticleChange -
// initialise this.  The original track will always be discontinued and
// secondaries followed.
//
  theParticleChange.Clear();
  theParticleChange.SetStatusChange(stopAndKill);
//
//
// Get relevant information about the projectile and target (A, Z, energy/nuc,
// momentum, etc).
//
  const G4ParticleDefinition *definitionP = theTrack.GetDefinition();
  const G4double AP   = definitionP->GetBaryonNumber();
  const G4double ZP   = definitionP->GetPDGCharge();
  G4LorentzVector pP  = theTrack.Get4Momentum();
  G4double E          = theTrack.GetKineticEnergy()/AP;
  G4double AT         = theTarget.GetN();
  G4double ZT         = theTarget.GetZ();
  G4double TotalEPre  = theTrack.GetTotalEnergy() +
    theTarget.AtomicMass(AT, ZT) + theTarget.GetEnergyDeposit();
  G4double TotalEPost = 0.0;
//
//
// Determine the radii of the projectile and target nuclei.
//
  G4WilsonRadius aR;
  G4double rP   = aR.GetWilsonRadius(AP);
  G4double rT   = aR.GetWilsonRadius(AT);
  G4double rPsq = rP * rP;
  G4double rTsq = rT * rT;
  if (verboseLevel >= 2)
  {
    G4cout <<"########################################"
           <<"########################################"
           <<G4endl;
    G4cout.precision(6);
    G4cout <<"IN G4WilsonAbrasionModel" <<G4endl;
    G4cout <<"Initial projectile A=" <<AP 
           <<", Z=" <<ZP
           <<", radius = " <<rP/fermi <<" fm"
           <<G4endl; 
    G4cout <<"Initial target     A=" <<AT
           <<", Z=" <<ZT
           <<", radius = " <<rT/fermi <<" fm"
           <<G4endl;
    G4cout <<"Projectile momentum and Energy/nuc = " <<pP <<" ," <<E <<G4endl;
  }
//
//
// The following variables are used to determine the impact parameter in the
// near-field (i.e. taking into consideration the electrostatic repulsion).
//
  G4double rm   = ZP * ZT * elm_coupling / (E * AP);
  G4double r    = 0.0;
  G4double rsq  = 0.0;
//
//
// Initialise some of the variables which wll be used to calculate the chord-
// length for nucleons in the projectile and target, and hence calculate the
// number of abraded nucleons and the excitation energy.
//
  G4NuclearAbrasionGeometry *theAbrasionGeometry = NULL;
  G4double CT   = 0.0;
  G4double F    = 0.0;
  G4int Dabr    = 0;
//
//
// The following loop is performed until the number of nucleons which are
// abraded by the process is >1, i.e. an interaction MUST occur.
//
  while (Dabr == 0)
  {
//
//
// Sample the impact parameter.  For the moment, this class takes account of
// electrostatic effects on the impact parameter, but (like HZETRN AND NUCFRG2)
// does not make any correction for the effects of nuclear-nuclear repulsion.
//
    G4double rPT   = rP + rT;
    G4double rPTsq = rPT * rPT;
    r              = 1.1 * rPT;
    while (r > rPT)
    {
      G4double bsq = rPTsq * G4UniformRand();
      r            = (rm + sqrt(rm*rm + 4.0*bsq)) / 2.0;
    }
    rsq = r * r;
//
//
// Now determine the chord-length through the target nucleus.
//
    if (rT > rP)
    {
      G4double x = (rPsq + rsq - rTsq) / 2.0 / r;
      if (x > 0.0) CT = 2.0 * sqrt(rTsq - x*x);
      else         CT = 2.0 * sqrt(rTsq - rsq);
    }
    else
    {
      G4double x = (rTsq + rsq - rPsq) / 2.0 / r;
      if (x > 0.0) CT = 2.0 * sqrt(rTsq - x*x);
      else         CT = 2.0 * rT;
    }
//
//
// Determine the number of abraded nucleons.  Note that the mean number of
// abraded nucleons is used to sample the Poisson distribution.  The Poisson
// distribution is sampled only ten times with the current impact parameter,
// and if it fails after this to find a case for which the number of abraded
// nucleons >1, the impact parameter is re-sampled.
//
    theAbrasionGeometry = new G4NuclearAbrasionGeometry(AP,AT,r);
    F                   = theAbrasionGeometry->F();
    G4double lambda     = 16.6*fermi / pow(E/MeV,0.26);
    G4double Mabr       = F * AP * (1.0 - exp(-CT/lambda));
    G4long n            = 0;
    for (G4int i = 0; i<10; i++)
    {
      n = G4Poisson(Mabr);
      if (n > 0)
      {
        if (n>AP) Dabr = (G4int) AP;
        else      Dabr = (G4int) n;
        break;
      }
    }
  }
  if (verboseLevel >= 2)
  {
    G4cout <<G4endl;
    G4cout <<"Impact parameter    = " <<r/fermi <<" fm" <<G4endl;
    G4cout <<"# Abraded nucleons  = " <<Dabr <<G4endl;
  }
//
//
// The number of abraded nucleons must be no greater than the number of
// nucleons in either the projectile or the target.  If AP - Dabr < 2 or 
// AT - Dabr < 2 then either we have only a nucleon left behind in the
// projectile/target or we've tried to abrade too many nucleons - and Dabr
// should be limited.
//
  if (AP - (G4double) Dabr < 2.0) Dabr = (G4int) AP;
  if (AT - (G4double) Dabr < 2.0) Dabr = (G4int) AT;
//
//
// Determine the abraded secondary nucleons from the projectile.  *fragmentP
// is a pointer to the prefragment from the projectile and nSecP is the number
// of nucleons in theParticleChange which have been abraded.  The total energy
// from these is determined.
//
  G4ThreeVector boost   = pP.findBoostToCM();
  G4Fragment *fragmentP = GetAbradedNucleons (Dabr, AP, ZP, rP); 
  G4int nSecP           = theParticleChange.GetNumberOfSecondaries();
  G4int i               = 0;
  for (i=0; i<nSecP; i++)
  {
    TotalEPost += theParticleChange.GetSecondary(i)->
      GetParticle()->GetTotalEnergy();
  }
//
//
// Determine the number of spectators in the interaction region for the
// projectile.
//
  G4int DspcP = (G4int) (AP*F) - Dabr;
  if (DspcP <= 0)           DspcP = 0;
  else if (DspcP > AP-Dabr) DspcP = ((G4int) AP) - Dabr;
//
//
// Determine excitation energy associated with excess surface area of the
// projectile (EsP) and the excitation due to scattering of nucleons which are
// retained within the projectile (ExP).  Add the total energy from the excited
// nucleus to the total energy of the secondaries.
//
  G4bool excitationAbsorbedByProjectile = false;
  if (fragmentP != NULL)
  {
    G4double EsP = theAbrasionGeometry->GetExcitationEnergyOfProjectile();
    G4double ExP = 0.0;
    if (Dabr < AT)
      excitationAbsorbedByProjectile = G4UniformRand() < 0.5;
    if (excitationAbsorbedByProjectile)
      ExP = GetNucleonInducedExcitation(rP, rT, r);
    G4double xP = EsP + ExP;
    if (xP > B*(AP-Dabr)) xP = B*(AP-Dabr);
    G4LorentzVector lorentzVector = fragmentP->GetMomentum();
    lorentzVector.setE(lorentzVector.e()+xP);
    fragmentP->SetMomentum(lorentzVector);
    TotalEPost += lorentzVector.e();
  }
  G4double EMassP = TotalEPost;
//
//
// Determine the abraded secondary nucleons from the target.  Note that it's
// assumed that the same number of nucleons are abraded from the target as for
// the projectile, and obviously no boost is applied to the products. *fragmentT
// is a pointer to the prefragment from the target and nSec is the total number
// of nucleons in theParticleChange which have been abraded.  The total energy
// from these is determined.
//
  G4Fragment *fragmentT = GetAbradedNucleons (Dabr, AT, ZT, rT); 
  G4int nSec = theParticleChange.GetNumberOfSecondaries();
  for (i=nSecP; i<nSec; i++)
  {
    TotalEPost += theParticleChange.GetSecondary(i)->
      GetParticle()->GetTotalEnergy();
  }
//
//
// Determine the number of spectators in the interaction region for the
// target.
//
  G4int DspcT = (G4int) (AT*F) - Dabr;
  if (DspcT <= 0)           DspcT = 0;
  else if (DspcT > AP-Dabr) DspcT = ((G4int) AT) - Dabr;
//
//
// Determine excitation energy associated with excess surface area of the
// target (EsT) and the excitation due to scattering of nucleons which are
// retained within the target (ExT).  Add the total energy from the excited
// nucleus to the total energy of the secondaries.
//
  if (fragmentT != NULL)
  {
    G4double EsT = theAbrasionGeometry->GetExcitationEnergyOfTarget();
    G4double ExT = 0.0;
    if (!excitationAbsorbedByProjectile)
      ExT = GetNucleonInducedExcitation(rT, rP, r);
    G4double xT = EsT + ExT;
    if (xT > B*(AT-Dabr)) xT = B*(AT-Dabr);
    G4LorentzVector lorentzVector = fragmentT->GetMomentum();
    lorentzVector.setE(lorentzVector.e()+xT);
    fragmentT->SetMomentum(lorentzVector);
    TotalEPost += lorentzVector.e();
  }
//
//
// Now determine the difference between the pre and post interaction
// energy - this will be used to determine the Lorentz boost if conservation
// of energy is to be imposed/attempted.
//
  G4double deltaE = TotalEPre - TotalEPost;
  if (deltaE > 0.0 && conserveEnergy)
  {
    G4double beta = sqrt(1.0 - EMassP*EMassP/pow(deltaE+EMassP,2.0));
    boost = boost / boost.mag() * beta;
  }
//
//
// Now boost the secondaries from the projectile.
//
  G4ThreeVector pBalance = pP.vect();
  for (i=0; i<nSecP; i++)
  {
    G4DynamicParticle *dynamicP = theParticleChange.GetSecondary(i)->
      GetParticle();
    G4LorentzVector lorentzVector = dynamicP->Get4Momentum();
    lorentzVector.boost(-boost);
    dynamicP->Set4Momentum(lorentzVector);
    pBalance -= lorentzVector.vect();
  }
//
//
// Set the boost for the projectile prefragment.  This is now based on the
// conservation of momentum.  However, if the user selected momentum of the
// prefragment is not to be conserved this simply boosted to the velocity of the
// original projectile times the ratio of the unexcited to the excited mass
// of the prefragment (the excitation increases the effective mass of the
// prefragment, and therefore modifying the boost is an attempt to prevent
// the momentum of the prefragment being excessive).
//
  if (fragmentP != NULL)
  {
    G4LorentzVector lorentzVector = fragmentP->GetMomentum();
    G4double m                    = lorentzVector.m();
    if (conserveMomentum)
      fragmentP->SetMomentum
        (G4LorentzVector(pBalance,sqrt(pBalance.mag2()+m*m+1.0*eV*eV)));
    else
    {
      G4double mg = fragmentP->GetGroundStateMass();
      fragmentP->SetMomentum(lorentzVector.boost(-boost * mg/m));
    }
  }
//
//
// Output information to user if verbose information requested.
//
  if (verboseLevel >= 2)
  {
    G4cout <<G4endl;
    G4cout <<"-----------------------------------" <<G4endl;
    G4cout <<"Secondary nucleons from projectile:" <<G4endl;
    G4cout <<"-----------------------------------" <<G4endl;
    G4cout.precision(7);
    for (i=0; i<nSecP; i++)
    {
      G4cout <<"Particle # " <<i <<G4endl;
      theParticleChange.GetSecondary(i)->GetParticle()->DumpInfo();
      G4DynamicParticle *dyn = theParticleChange.GetSecondary(i)->GetParticle();
      G4cout <<"New nucleon (P) " <<dyn->GetDefinition()->GetParticleName()
             <<" : "              <<dyn->Get4Momentum()
             <<G4endl;
    }
    G4cout <<"---------------------------" <<G4endl;
    G4cout <<"The projectile prefragment:" <<G4endl;
    G4cout <<"---------------------------" <<G4endl;
    if (fragmentP != NULL)
      G4cout <<*fragmentP <<G4endl;
    else
      G4cout <<"(No residual prefragment)" <<G4endl;
    G4cout <<G4endl;
    G4cout <<"-------------------------------" <<G4endl;
    G4cout <<"Secondary nucleons from target:" <<G4endl;
    G4cout <<"-------------------------------" <<G4endl;
    G4cout.precision(7);
    for (i=nSecP; i<nSec; i++)
    {
      G4cout <<"Particle # " <<i <<G4endl;
      theParticleChange.GetSecondary(i)->GetParticle()->DumpInfo();
      G4DynamicParticle *dyn = theParticleChange.GetSecondary(i)->GetParticle();
      G4cout <<"New nucleon (T) " <<dyn->GetDefinition()->GetParticleName()
             <<" : "              <<dyn->Get4Momentum()
             <<G4endl;
    }
    G4cout <<"-----------------------" <<G4endl;
    G4cout <<"The target prefragment:" <<G4endl;
    G4cout <<"-----------------------" <<G4endl;
    if (fragmentT != NULL)
      G4cout <<*fragmentT <<G4endl;
    else
      G4cout <<"(No residual prefragment)" <<G4endl;
  }
//
//
// Now we can decay the nuclear fragments if present.  The secondaries are
// collected and boosted as well.  This is performed first for the projectile...
//
  if (fragmentP !=NULL)
  {
    G4ReactionProductVector *products = NULL;
    if (fragmentP->GetZ() != fragmentP->GetA())
      products = theExcitationHandler->BreakItUp(*fragmentP);
    else
      products = theExcitationHandlerx->BreakItUp(*fragmentP);      
    delete fragmentP;
    fragmentP = NULL;
  
    G4ReactionProductVector::iterator iter;
    for (iter = products->begin(); iter != products->end(); ++iter)
    {
      G4DynamicParticle *secondary =
        new G4DynamicParticle((*iter)->GetDefinition(),
        (*iter)->GetTotalEnergy(), (*iter)->GetMomentum());
      G4String particleName = (*iter)->GetDefinition()->GetParticleName();
      if (verboseLevel >= 2 && particleName.find("[",0) < particleName.size())
      {
        G4cout <<"------------------------" <<G4endl;
        G4cout <<"The projectile fragment:" <<G4endl;
        G4cout <<"------------------------" <<G4endl;
        G4cout <<" fragmentP = " <<particleName
               <<" Energy    = " <<secondary->GetKineticEnergy()
               <<G4endl;
      }
    }
  }
//
//
// Now decay the target nucleus - no boost is applied since in this
// approximation it is assumed that there is negligible momentum transfer from
// the projectile.
//
  if (fragmentT != NULL)
  {
    G4ReactionProductVector *products = NULL;
    if (fragmentT->GetZ() != fragmentT->GetA())
      products = theExcitationHandler->BreakItUp(*fragmentT);
    else
      products = theExcitationHandlerx->BreakItUp(*fragmentT);      
    delete fragmentT;
    fragmentT = NULL;
  
    G4ReactionProductVector::iterator iter;
    for (iter = products->begin(); iter != products->end(); ++iter)
    {
      G4DynamicParticle *secondary =
        new G4DynamicParticle((*iter)->GetDefinition(),
        (*iter)->GetTotalEnergy(), (*iter)->GetMomentum());
      theParticleChange.AddSecondary (secondary);
      G4String particleName = (*iter)->GetDefinition()->GetParticleName();
      if (verboseLevel >= 2 && particleName.find("[",0) < particleName.size())
      {
        G4cout <<"--------------------" <<G4endl;
        G4cout <<"The target fragment:" <<G4endl;
        G4cout <<"--------------------" <<G4endl;
        G4cout <<" fragmentT = " <<particleName
               <<" Energy    = " <<secondary->GetKineticEnergy()
               <<G4endl;
      }
    }
  }

  if (verboseLevel >= 2)
     G4cout <<"########################################"
            <<"########################################"
            <<G4endl;
  
  delete theAbrasionGeometry;

  return &theParticleChange;
}
////////////////////////////////////////////////////////////////////////////////
//
G4Fragment *G4WilsonAbrasionModel::GetAbradedNucleons (G4int Dabr, G4double A,
  G4double Z, G4double r)
{
//
//
// Initialise variables.  tau is the Fermi radius of the nucleus.  The variables
// p..., C... and g(amma) are used to help sample the secondary nucleon
// spectrum.
//
  
  G4double pK   = hbarc * pow(9.0 * pi / 4.0 * A, third) / (1.29 * r);
  if (A <= 24.0) pK *= -0.229*pow(A,third) + 1.62; 
  G4double pKsq = pK * pK;
  G4double p1sq = 2.0/5.0 * pKsq;
  G4double p2sq = 6.0/5.0 * pKsq;
  G4double p3sq = 500.0 * 500.0;
  G4double C1   = 1.0;
  G4double C2   = 0.03;
  G4double C3   = 0.0002;
  G4double g    = 90.0 * MeV;
  G4double maxn = C1 + C2 + C3;
//
//
// initialise the number of secondary nucleons abraded to zero, and initially set
// the type of nucleon abraded to proton ... just for now.
//  
  G4double Aabr                     = 0.0;
  G4double Zabr                     = 0.0; 
  G4ParticleDefinition *typeNucleon = G4Proton::ProtonDefinition();
  G4DynamicParticle *dynamicNucleon = NULL;
  G4ParticleMomentum pabr(0.0, 0.0, 0.0);
//
//
// Now go through each abraded nucleon and sample type, spectrum and angle.
//
  for (G4int i=0; i<Dabr; i++)
  {
//
//
// Sample the nucleon momentum distribution by simple rejection techniques.  We
// reject values of p == 0.0 since this causes bad behaviour in the sinh term.
//
    G4double p   = 0.0;
    G4bool found = false;
    while (!found)
    {
      while (p <= 0.0) p = npK * pK * G4UniformRand();
      G4double psq = p * p;
      found = maxn * G4UniformRand() < C1*exp(-psq/p1sq/2.0) +
        C2*exp(-psq/p2sq/2.0) + C3*exp(-psq/p3sq/2.0) + p/g/sinh(p/g);
    }
//
//
// Determine the type of particle abraded.  Can only be proton or neutron,
// and the probability is determine to be proportional to the ratio as found
// in the nucleus at each stage.
//
    G4double prob = (Z-Zabr)/(A-Aabr);
    if (G4UniformRand()<prob)
    {
      Zabr++;
      typeNucleon = G4Proton::ProtonDefinition();
    }
    else
      typeNucleon = G4Neutron::NeutronDefinition();
    Aabr++;
//
//
// The angular distribution of the secondary nucleons is approximated to an
// isotropic distribution in the rest frame of the nucleus (this will be Lorentz
// boosted later.
//
    G4double costheta = 2.*G4UniformRand()-1.0;
    G4double sintheta = sqrt((1.0 - costheta)*(1.0 + costheta));
    G4double phi      = 2.0*pi*G4UniformRand()*rad;
    G4ThreeVector direction(sintheta*cos(phi),sintheta*sin(phi),costheta);
    G4double nucleonMass = typeNucleon->GetPDGMass();
    G4double E           = sqrt(p*p + nucleonMass*nucleonMass)-nucleonMass;
    dynamicNucleon = new G4DynamicParticle(typeNucleon,direction,E);
    theParticleChange.AddSecondary (dynamicNucleon);
    pabr += p*direction;
  }
//
//
// Next determine the details of the nuclear prefragment .. that is if there
// is one or more protons in the residue.  (Note that the 1 eV in the total
// energy is a safety factor to avoid any possibility of negative rest mass
// energy.)
//
  G4Fragment *fragment = NULL;
  if (Z-Zabr>=1.0)
  {
    G4double ionMass = G4ParticleTable::GetParticleTable()->GetIonTable()->
                       GetIonMass(G4lrint(Z-Zabr),G4lrint(A-Aabr));
    G4double E       = sqrt(pabr.mag2() + ionMass*ionMass);
    G4LorentzVector lorentzVector = G4LorentzVector(-pabr, E + 1.0*eV);
    fragment =
      new G4Fragment((G4int) (A-Aabr), (G4int) (Z-Zabr), lorentzVector);
  }

  return fragment;
}
////////////////////////////////////////////////////////////////////////////////
//
G4double G4WilsonAbrasionModel::GetNucleonInducedExcitation
  (G4double rP, G4double rT, G4double r)
{
//
//
// Initialise variables.
//
  G4double Cl   = 0.0;
  G4double rPsq = rP * rP;
  G4double rTsq = rT * rT;
  G4double rsq  = r * r;
//
//
// Depending upon the impact parameter, a different form of the chord length is
// is used.
//  
  if (r > rT) Cl = 2.0*sqrt(rPsq + 2.0*r*rT - rsq - rTsq);
  else        Cl = 2.0*rP;
  
  G4double bP = (rPsq+rsq-rTsq)/2.0/r;
  G4double Ct = 2.0*sqrt(rPsq - bP*bP);
  
  G4double Ex = 13.0 * Cl / fermi;
  if (Ct > 1.5*fermi)
    Ex += 13.0 * Cl / fermi /3.0 * (Ct/fermi - 1.5);

  return Ex;
}
////////////////////////////////////////////////////////////////////////////////
//
void G4WilsonAbrasionModel::SetUseAblation (G4bool useAblation1)
{
  if (useAblation != useAblation1)
  {
    useAblation = useAblation1;
    delete theExcitationHandler;
    delete theExcitationHandlerx;
    theExcitationHandler  = new G4ExcitationHandler;
    theExcitationHandlerx = new G4ExcitationHandler;
    if (useAblation)
    {
      theAblation = new G4WilsonAblationModel;
      theAblation->SetVerboseLevel(verboseLevel);
      theExcitationHandler->SetEvaporation(theAblation);
      theExcitationHandlerx->SetEvaporation(theAblation);
    }
    else
    {
      theAblation                      = NULL;
      G4Evaporation * theEvaporation   = new G4Evaporation;
      G4FermiBreakUp * theFermiBreakUp = new G4FermiBreakUp;
      G4StatMF * theMF                 = new G4StatMF;
      theExcitationHandler->SetEvaporation(theEvaporation);
      theExcitationHandler->SetFermiModel(theFermiBreakUp);
      theExcitationHandler->SetMultiFragmentation(theMF);
      theExcitationHandler->SetMaxAandZForFermiBreakUp(12, 6);
      theExcitationHandler->SetMinEForMultiFrag(5.0*MeV);

      theEvaporation  = new G4Evaporation;
      theFermiBreakUp = new G4FermiBreakUp;
      theExcitationHandlerx->SetEvaporation(theEvaporation);
      theExcitationHandlerx->SetFermiModel(theFermiBreakUp);
      theExcitationHandlerx->SetMaxAandZForFermiBreakUp(12, 6);
    }
  }
  return; 
}
////////////////////////////////////////////////////////////////////////////////
//
void G4WilsonAbrasionModel::PrintWelcomeMessage ()
{
  G4cout <<G4endl;
  G4cout <<" *****************************************************************"
         <<G4endl;
  G4cout <<" Nuclear abrasion model for nuclear-nuclear interactions activated"
         <<G4endl;
  G4cout <<" (Written by QinetiQ Ltd for the European Space Agency)"
         <<G4endl;
  G4cout <<" *****************************************************************"
         <<G4endl;
  G4cout << G4endl;

  return;
}
////////////////////////////////////////////////////////////////////////////////
//