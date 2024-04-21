#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

namespace eCAL
{
    class CNBSubscriber:CSubscriber
    {
    public:
        CNBSubscriber(const std::string& s) : CSubscriber(s) { }

        std::string Receive()
        {
            std::string Rec_Data = "No Reception";
            long long rcv_time = 12345;
            auto success = eCAL::CSubscriber::ReceiveBuffer(Rec_Data, &rcv_time, 500);
            return(Rec_Data);
        }
    };
}
