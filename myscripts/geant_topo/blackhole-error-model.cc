#include "blackhole-error-model.h"
 
NS_OBJECT_ENSURE_REGISTERED(BlackholeErrorModel);
 
TypeId BlackholeErrorModel::GetTypeId(void) {
  static TypeId tid = TypeId("BlackholeErrorModel")
    .SetParent<ErrorModel>()
    .AddConstructor<BlackholeErrorModel>()
    ;
  return tid;
}
 
BlackholeErrorModel::BlackholeErrorModel() : enabled_(true) { }
 
bool BlackholeErrorModel::DoCorrupt(Ptr<Packet> p) {
  return enabled_;
}

void BlackholeErrorModel::Enable() {
  enabled_ = true;
}

void BlackholeErrorModel::Disable() {
  enabled_ = false;
}

void BlackholeErrorModel::DoReset(void) { }

void Enable(Ptr<BlackholeErrorModel> em, const Time next, const int repeat) {
    static int counter = 0;
    counter++;
    std::cout << Simulator::Now().GetSeconds() << "s: Enabling blackhole" << std::endl;
    em->Enable();
    if(counter < repeat) {
        Simulator::Schedule(next, &Enable, em, next, repeat);
    }
}

void Disable(Ptr<BlackholeErrorModel> em, const Time next, const int repeat) {
    static int counter = 0;
    std::cout << Simulator::Now().GetSeconds() << "s: Disabling blackhole" << std::endl;
    counter++;
    em->Disable();
    if(counter < repeat) {
        Simulator::Schedule(next, &Disable, em, next, repeat);
    }
}