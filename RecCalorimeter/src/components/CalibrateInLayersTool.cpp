#include "CalibrateInLayersTool.h"

// FCCSW
#include "DetInterface/IGeoSvc.h"

// DD4hep
#include "DD4hep/LCDD.h"
#include "DD4hep/Readout.h"

DECLARE_TOOL_FACTORY(CalibrateInLayersTool)

CalibrateInLayersTool::CalibrateInLayersTool(const std::string& type, const std::string& name, const IInterface* parent)
    : GaudiTool(type, name, parent) {
  declareInterface<ICalibrateCaloHitsTool>(this);
}

StatusCode CalibrateInLayersTool::initialize() {
  if (GaudiTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  m_geoSvc = service("GeoSvc");
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }
  // check if readouts exist
  if (m_geoSvc->lcdd()->readouts().find(m_readoutName) == m_geoSvc->lcdd()->readouts().end()) {
    error() << "Readout <<" << m_readoutName << ">> does not exist." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

void CalibrateInLayersTool::calibrate(std::unordered_map<uint64_t, double>& aHits) {
  auto decoder = m_geoSvc->lcdd()->readout(m_readoutName).idSpec().decoder();
  // Loop through energy deposits, multiply energy to get cell energy at electromagnetic scale
  std::for_each(aHits.begin(), aHits.end(),
    [this, decoder](std::pair<const uint64_t, double>& p) {
      decoder->setValue(p.first);
      info() << m_samplingFraction[(*decoder)[m_layerFieldName].value()] << endmsg;
      p.second /= m_samplingFraction[(*decoder)[m_layerFieldName].value()];
    });
}

StatusCode CalibrateInLayersTool::finalize() { return GaudiTool::finalize(); }
