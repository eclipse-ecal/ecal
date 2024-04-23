#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

namespace eCAL
{
    class CNBSubscriber: public CSubscriber
    {
    public:
        CNBSubscriber() : CSubscriber() { }
        CNBSubscriber(const std::string& s) : CSubscriber(s) { }
        CNBSubscriber(const std::string& s, const SDataTypeInformation& datainfo) : CSubscriber(s,datainfo) { }

        std::string Receive(int nb_timeout)
        {
            std::string Rec_Data = "No Reception";
            auto success = eCAL::CSubscriber::ReceiveBuffer(Rec_Data, nullptr, nb_timeout);
            return(Rec_Data);
        }
    };
}
