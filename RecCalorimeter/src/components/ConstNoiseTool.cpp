#include "ConstNoiseTool.h"

// FCCSW
#include "DetCommon/DetUtils.h"
#include "DetInterface/IGeoSvc.h"

// DD4hep
#include "DD4hep/Detector.h"

// Root
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"

DECLARE_TOOL_FACTORY(ConstNoiseTool)

ConstNoiseTool::ConstNoiseTool(const std::string& type, const std::string& name, const IInterface* parent)
    : GaudiTool(type, name, parent) {
  declareInterface<INoiseConstTool>(this);
  declareProperty("noiseInHCalBarrel", m_addedHcalBarrelNoise, "Electronic noise constant in HCalBarrel");
}

StatusCode ConstNoiseTool::initialize() {
  /// Noise estimates from Atlas [GeV]
  // set for ECalBarrel system:5
  // set for HCalBarrel system:6
  // set for HCalExtBarrel system:7
  m_systemNoiseConstMap.emplace(5, 0.0075 / 4.);
  m_systemNoiseConstMap.emplace(8, 0.0115 / 4.);
  m_systemNoiseConstMap.emplace(9, 0.0115 / 4.);
  // if noise in HCal is added following Gaussian distribution with sigma = m_addedHcalBarrelNoise [GeV]
  if (m_addedHcalBarrelNoise > 0){
    m_systemNoiseConstMap.erase(8);
    m_systemNoiseConstMap.emplace(8, m_addedHcalBarrelNoise);
  }
  
  // Get GeoSvc
  m_geoSvc = service("GeoSvc");
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }
 
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;

  return sc;
}

StatusCode ConstNoiseTool::finalize() {
  StatusCode sc = GaudiTool::finalize();
  return sc;
}

double ConstNoiseTool::getNoiseConstantPerCell(uint64_t aCellId) {

  double Noise = 0.;

  // Get cells global coordinate "system"
  m_decoder->setValue(aCellId);
  unsigned cellSystem = (*m_decoder)["system"];
  // cell noise in system
  if (m_systemNoiseConstMap[cellSystem])
    Noise = m_systemNoiseConstMap[cellSystem];
  else
    warning() << "No noise constants set for this subsystem! Noise of cell set to 0. " << endmsg;
  return Noise;
}
