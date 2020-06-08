#include "common/Typedefs.h"
#include "Common/timing/Timer.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include <iostream>

int main()
{
    string inputPath = "G:/RFG Unpack/data/misc.vpp_pc";
    string outputPath = "G:/RFG Unpack 2/CppToolOutput/Unpack/";

    std::cout << "inputPath: \"" << inputPath << "\"\n";
    std::cout << "outputPath: \"" << outputPath << "\"\n";

    u32 numRuns = 100;
    std::vector<u64> times;
    times.reserve(numRuns);

    printf("Parsing packfile metadata...\n");
    Timer timer(true);

    for (u32 i = 0; i < numRuns; i++)
    {
        Packfile3 packfile(inputPath);
        packfile.ReadMetadata();
        times.push_back(timer.ElapsedMicroseconds());
        timer.Reset();
    }

    f32 sum = 0.0f;
    for (auto& val : times)
        sum += (f32)val;

    std::cout << "Elapsed time (average over " << numRuns << " runs): " << sum / (f32)numRuns << "us\n";

    return 0;
}