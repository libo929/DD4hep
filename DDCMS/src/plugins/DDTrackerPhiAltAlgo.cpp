//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// Specialized generic detector constructor
// 
//==========================================================================

// Framework include files
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DDCMS/DDCMSPlugins.h"

// C/C++ include files
#include <sstream>

using namespace std;
using namespace dd4hep;
using namespace dd4hep::cms;

static long algorithm(Detector& /* description */,
                      ParsingContext& ctxt,
                      xml_h e,
                      SensitiveDetector& /* sens */)
{
  stringstream  str;
  Namespace     ns(ctxt, e, true);
  AlgoArguments args(ctxt, e);
  Volume        mother      = ns.volume(args.parentName());
  Volume        child       = ns.volume(args.childName());
  double        tilt        = args.value<double>("Tilt");        //Tilt of the module
  double        startAngle  = args.value<double>("StartAngle");  //offset in phi
  double        rangeAngle  = args.value<double>("RangeAngle");  //Maximum range in phi
  double        radiusIn    = args.value<double>("RadiusIn");    //Inner radius
  double        radiusOut   = args.value<double>("RadiusOut");   //Outer radius
  double        zpos        = args.value<double>("ZPosition");   //z position
  int           number      = args.value<int>("Number");         //Number of copies
  int           startCopyNo = args.find("StartCopyNo") ? args.value<int>("StartCopyNo") : 1; //Start copy number
  int           incrCopyNo  = args.find("IncrCopyNo")  ? args.value<int>("IncrCopyNo")  : 1; //Increment in copy number

  str << "Parameters for positioning-- Tilt " << tilt 
      << "\tStartAngle " << startAngle/CLHEP::deg 
      << "\tRangeAngle " << rangeAngle/CLHEP::deg 
      << "\tRin " << radiusIn << "\tRout " << radiusOut 
      << "\t ZPos " << zpos << "\tCopy Numbers " << number 
      << " Start/Increment " << startCopyNo << ", " 
      << incrCopyNo;
  printout(ctxt.debug_algorithms ? ALWAYS : DEBUG,"DDTrackerPhiAltAlgo",str);
  str << "Parent " << mother.name() << "\tChild " << child.name() << " NameSpace " << ns.name;
  printout(ctxt.debug_algorithms ? ALWAYS : DEBUG,"DDTrackerPhiAltAlgo",str);

  if (number > 0) {
    double theta  = 90.*CLHEP::deg;
    double dphi;
    if (number == 1 || fabs(rangeAngle-360.0*CLHEP::deg)<0.001*CLHEP::deg) 
      dphi = rangeAngle/number;
    else
      dphi = rangeAngle/(number-1);
    int copyNo = startCopyNo;

    for (int i=0; i<number; i++) {
      double phi  = startAngle + i*dphi;
      double phix = phi - tilt + 90.*CLHEP::deg;
      double phiy = phix + 90.*CLHEP::deg;
      double phideg = phix/CLHEP::deg;
  
      Rotation3D rotation;
      if (phideg != 0) {
        rotation = make_rotation3D(theta, phix, theta, phiy, 0., 0.);
      }
	
      double xpos, ypos;
      if (i%2 == 0) {
        xpos = radiusIn*cos(phi);
        ypos = radiusIn*sin(phi);
      } else {
        xpos = radiusOut*cos(phi);
        ypos = radiusOut*sin(phi);
      }
      Position tran(xpos, ypos, zpos);  
      /* PlacedVolume pv = */ mother.placeVolume(child,Transform3D(rotation,tran));
      str << "" << child.name() 
          << " number " << copyNo << " positioned in " 
          << mother.name() << " at " << tran << " with " 
          << rotation;
      printout(ctxt.debug_algorithms ? ALWAYS : DEBUG,"DDTrackerPhiAltAlgo",str);
      copyNo += incrCopyNo;
    }
  }
  return 1;
}

// first argument is the type from the xml file
DECLARE_DDCMS_DETELEMENT(track_DDTrackerPhiAltAlgo,algorithm)

