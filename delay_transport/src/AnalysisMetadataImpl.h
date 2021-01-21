#ifndef DELAY_TRANSPORT_ANALYSISMETADATAIMPL_H
#define DELAY_TRANSPORT_ANALYSISMETADATAIMPL_H

#include <delay_transport/AnalysisMetadata.h>

namespace delay_transport {

class AnalysisMetadataImpl : public AnalysisMetadata {
  public:
    ~AnalysisMetadataImpl() override = default;

  public:
    double message_creation_time;
    double message_expiration_time;
};

} // namespace delay_transport

#endif // DELAY_TRANSPORT_ANALYSISMETADATAIMPL_H
