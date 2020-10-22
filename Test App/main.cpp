#include "common/Typedefs.h"
#include "Common/timing/Timer.h"
#include "Common/filesystem/Path.h"
#include "Common/filesystem/File.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include <iostream>

int main()
{
    /*Unpacking test*/

    //string inputPath = "G:\\GOG\\Games\\Red Faction Guerrilla Re-Mars-tered\\data\\anims.vpp_pc";
    ////string inputPath = "G:\\RFG Unpack 2\\CppToolOutput\\Unpack\\vehicles_r\\mar_raider1.ccar_pc.str2_pc";
    //string outputPath = "G:/RFG Unpack 2/CppToolOutput/Unpack/";

    //std::cout << "inputPath: \"" << inputPath << "\"\n";
    //std::cout << "outputPath: \"" << outputPath << "\"\n";

    //u32 numRuns = 1; //Should use higher number for more precise results. Need speed for testing though
    //std::vector<u64> times;
    //times.reserve(numRuns);

    //printf("Extracting packfile %d times...\n", numRuns);
    //Timer timer(true);

    //for (u32 i = 0; i < numRuns; i++)
    //{
    //    Packfile3 packfile(inputPath);
    //    packfile.ReadMetadata();
    //    packfile.ExtractSubfiles(outputPath + Path::GetFileNameNoExtension(inputPath) + "/");
    //    //auto weaponsXtbl = packfile.ExtractSingleFile("weapons.xtbl");
    //    //File::WriteToFile("G:\\RFG Unpack\\boop.xml", weaponsXtbl.value());
    //    //delete[] weaponsXtbl.value().data();
    //    times.push_back(timer.ElapsedMicroseconds());
    //    timer.Reset();
    //}

    //f32 sum = 0.0f;
    //for (auto& val : times)
    //    sum += (f32)val;

    //std::cout << "Elapsed time (average over " << numRuns << " runs): " << sum / (f32)numRuns << "us | " << sum / (f32)numRuns / 1000.0f << "ms | " << sum / (f32)numRuns / 1000000.0f << "s\n";


    /*Packing test*/

    std::cout << "Starting packing tests...\n";

    string packInputPath = "G:\\RFG Unpack 2\\CppToolOutput\\Unpack\\anims\\";
    string packOutputPath = "G:/RFG Unpack 2/CppToolOutput/Repack/anims.vpp_pc";
    //string packInputPath = "G:\\RFG Unpack 2\\CppToolOutput\\Unpack\\mar_raider1.ccar_pc\\";
    //string packOutputPath = "G:/RFG Unpack 2/CppToolOutput/Repack/mar_raider1.ccar_pc.str2_pc";

    std::cout << "inputPath: \"" << packInputPath << "\"\n";
    std::cout << "outputPath: \"" << packOutputPath << "\"\n";

    //Todo: Add support for consuming @streams file for config
    Packfile3::Pack(packInputPath, packOutputPath, true, true);

    return 0;
}