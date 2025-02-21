#include <flexisim/devices/dummy_device.hpp>
#include <flexisim/device.hpp>
#include <flexisim/kernel.hpp>


int main() {
    auto dummy_device = flexisim::createDevice();
    auto kernel = flexisim::Kernel({dummy_device});

    flexisim::dummy_device::Action__dummy_no_arg({});

    flexisim::Action a1(dummy_device, flexisim::dummy_device::Action__dummy_no_arg, {});

     kernel.pushEvent({}, {&a1});
}
