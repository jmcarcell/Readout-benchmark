#include "FelixFormat.hh"
#include "PdspChannelMapService.h"
#include "FrameFile.h"

#include <fstream>
#include <iostream>
#include <map>
#include <cstdio>

#include <inttypes.h> // For PRIx64 format specifier

#include "CLI11.hpp"

#include "hist.h"

//======================================================================
void print_channel_numbers(FILE* f, const dune::FelixFrame* frame)
{
    PdspChannelMapService channelMapService;
    fprintf(f, "0x0               ");
    for(int i=0; i<256; ++i){
        fprintf(f, "% 6d ", channelMapService.getOfflineChannel(frame, i));
    }
    fprintf(f, "\n");
            
}
//======================================================================
//
// dumpfile-to-text: converts raw binary long-readout FELIX dumps to a
// text format. See --help for options
//
// The output format is:
//
// 0x0            ch0     ch1     ch2 ...
// timestamp0    adc0    adc1    adc2 ...
// timestamp1    adc0    adc1    adc2 ...
// ...

// where ch0, ch1 etc are the offline channel numbers (in
// "electronics" order) and timestampX is the timestamp in 50 MHz
// clock ticks. In this scheme, each column is the waveform from one
// channel
int main(int argc, char** argv)
{
    Hist hist = Hist(100, 0, 5000);
    // -----------------------------------------------------------------
    // Parse the command-line arguments
    CLI::App app{"Print dumped hits"};

    std::string input_file_name;
    app.add_option("-i", input_file_name, "Input file", true)->required();

    std::string output_file_name;
    app.add_option("-o", output_file_name, "Output file", true)->required();

    int nticks=-1;
    app.add_option("-n", nticks, "Number of ticks to output (-1 for no limit)", true);

    CLI11_PARSE(app, argc, argv);

    // -----------------------------------------------------------------

    // The size in bytes of a frame
    constexpr size_t frame_size=sizeof(dune::FelixFrame);

    FrameFile frame_file(input_file_name.c_str());
    // We use FILE* and fprintf, instead of std::ofstream, for output so we can get nice columns
    FILE* fout=fopen(output_file_name.c_str(), "w");

    uint64_t prev_timestamp=0;
    size_t nbad=0;
    // The number of frames we'll actually loop over: might have been limited by the -n option
    size_t nframes=(nticks==-1) ? frame_file.num_frames() : std::min((size_t)nticks, frame_file.num_frames());

    std::cout << nframes << std::endl;

    for(size_t i=0; i<1000; ++i){
        const dune::FelixFrame* frame=frame_file.frame(i);
        // Print the header
        if(i==0){
            print_channel_numbers(fout, frame);
        }
        // Print the ADC value for each of the 256 channels in the frame
        uint64_t timestamp=frame->timestamp();
        fprintf(fout, "%#" PRIx64 " ", frame->timestamp());
        for(int i=0; i<256; ++i){
            fprintf(fout, "% 6d ", frame->channel(i));
            // std::cout << frame->channel(i) << " ";
            hist.Fill(frame->channel(i));
        }
        // std::cout << std::endl;
        fprintf(fout, "\n");

        // Check that the gap between timestamps is 25 ticks
        if(prev_timestamp!=0 && (timestamp-prev_timestamp!=25)){
            std::cerr << "Inter-frame timestamp gap of " << (timestamp-prev_timestamp) << " ticks at ts 0x" << std::hex << timestamp << std::dec << ". index=" << i << std::endl;
            ++nbad;
        }
        prev_timestamp=timestamp;
    }
    std::cout << nbad << " bad of " << nframes << std::endl;
    hist.Save("hist.txt");
}
