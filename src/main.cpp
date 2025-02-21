#include <flexisim/kernel.hpp>
#include <flexisim/device.hpp>
#include <flexisim/devices/dummy_device.hpp>
#include <flexisim/devices/samsung_pim.hpp>


// int main() {
//     auto dummy_device = flexisim::createDevice();
//     auto kernel = flexisim::Kernel({dummy_device});

//     flexisim::dummy_device::Action__dummy_no_arg({});

//     flexisim::Action a1(dummy_device, flexisim::dummy_device::Action__dummy_no_arg, {});

//      kernel.pushEvent({}, {&a1});
// }


// another example

using namespace flexisim;
using namespace flexisim::samsung_pim;


int main() {

    // const auto dummy_device = createDevice();
    // Kernel kernel({dummy_device});
    //
    // Action a1(dummy_device, Action__dummy_no_arg, {});
    // kernel.pushEvent({}, {&a1});
    //
    // kernel.run();
    //
    // return 0;


    // --- Setup Simulation and Device ---
    // Create simulation object.

    // Create the HBM-PIM device.
    auto hbmPim = createDevice("./config.yaml");

    // Create the kernel with the device.
    Kernel kernel({ hbmPim });




    // --- Define Mappings for Vectors ---
    // Memory vector mapping.
    samsung_pim::Mapping memMapping({
        { MEMORY_ORG::CHANNEL,    Range(1) },
        { MEMORY_ORG::BANK_GROUP, Range(1) },
        { MEMORY_ORG::BANK,       Range(1) },
        { MEMORY_ORG::ROW,        Range(std::vector<int>{1}) },
        { MEMORY_ORG::COL,        Range(8) }
    });

    // PIM vector mapping.
    Mapping pimMapping({
        { PIM_ORG::CHANNEL, Range(1) },
        { PIM_ORG::REG,     Range(8) }
    });

    // --- Create Vectors ---
    // Construct the MemoryVector and PimVector
    auto  memVector = new MemoryVector(std::make_shared<Mapping>(memMapping));
    auto  pimVector = new PimVector(std::make_shared<Mapping>(pimMapping), PIM_REG_TYPE::GRF_A);

    // --- Define Memory Access Order ---
    std::vector<MEMORY_ORG> order = {
        MEMORY_ORG::ROW,
        MEMORY_ORG::COL,
        MEMORY_ORG::BANK_GROUP,
        MEMORY_ORG::BANK,
        MEMORY_ORG::CHANNEL
    };

    // --- Schedule Actions ---
    // Write the memory vector into the device memory.
    Action writeMemAction(hbmPim, Action__write_memory_vector, { memVector, &order });
    kernel.pushEvent({}, std::vector<BaseAction*>{ &writeMemAction });

    // Parameters for PIM actions.
    OPERAND_TYPE bankType = OPERAND_TYPE::EVEN_BANK;
    PIM_COMMAND_TYPE pimCmd = PIM_COMMAND_TYPE::RELU;

    // Iterate over slices of the memory vector.
    for (auto const& rowSlice : sliceMapping(memVector->m_mapping, MEMORY_ORG::ROW)) {
        for (auto const& colSlice : sliceMapping(rowSlice, MEMORY_ORG::COL, 8)) {
            // Create a new MemoryVector for this slice.
            MemoryVector* slicedMemVector = new MemoryVector(colSlice);

            // Create the fill/relu action.
            Action* reluAction = new Action(
                hbmPim,
                Action__fill_or_relu_pim_from_mem,
                std::vector<std::any>{ pimVector, slicedMemVector, &bankType, &pimCmd }
            );
            kernel.pushEvent({}, std::vector<BaseAction*>{ reluAction });

            // Create the move action (from PIM back to memory).
            Action* moveAction = new Action(
                hbmPim,
                Action__mov_pim_to_mem,
                std::vector<std::any>{ slicedMemVector, pimVector, &bankType }
            );
            kernel.pushEvent({}, std::vector<BaseAction*>{ moveAction });
        }
    }

    // Read the memory vector from the device memory.
    // Action readMemAction(hbmPim, Action__read_memory_vector, { memVector, &order });
    // kernel.pushEvent({}, std::vector<BaseAction*>{ &readMemAction });

    // --- Run the Simulation ---
    kernel.run();
    kernel.reportRun();

    // --- Cleanup ---
    delete memVector;
    delete pimVector;

    return 0;
}
