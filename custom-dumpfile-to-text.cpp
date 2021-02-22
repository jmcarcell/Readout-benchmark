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
#include <chrono>

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
    auto begin_main = std::chrono::steady_clock::now();
    // Vector with all the histograms

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

    std::chrono::duration<double> batch_time(0);
    std::chrono::duration<double> batch_time_no_save(0);
    int batches = nframes / 10000;

    for(int batch=0; batch<nframes / 10000; ++batch){
        auto begin = std::chrono::steady_clock::now();

        // Vector with the 256 histograms
        std::vector<Hist> v;
        v.reserve(256);
        for(int i=0; i<256; i++)
            v.push_back(Hist(100, 0, 5000));

        // File handle where the histos are saved for plotting later
        std::ofstream file( "hist_" + std::to_string(batch) + ".txt" );



        for(size_t i=batch * 10000; i<batch * 10001; ++i){
            const dune::FelixFrame* frame=frame_file.frame(i);
            // Print the header
            if(i==0){
                print_channel_numbers(fout, frame);
            }
            // Print the ADC value for each of the 256 channels in the frame
            uint64_t timestamp=frame->timestamp();
            // fprintf(fout, "%#" PRIx64 " ", frame->timestamp());
            for(int j=0; j<256; ++j){
                // fprintf(fout, "% 6d ", frame->channel(j));
                // std::cout << frame->channel(i) << " ";
                v[j].Fill(frame->channel(j));
            }
            // std::cout << std::endl;
            // fprintf(fout, "\n");

            // Check that the gap between timestamps is 25 ticks
            // if(prev_timestamp!=0 && (timestamp-prev_timestamp!=25)){
            //     std::cerr << "Inter-frame timestamp gap of " << (timestamp-prev_timestamp) << " ticks at ts 0x" << std::hex << timestamp << std::dec << ". index=" << i << std::endl;
            //     ++nbad;
            // }
            // prev_timestamp=timestamp;
        }
        auto t = std::chrono::steady_clock::now();
        batch_time_no_save += (t-begin);
        for(int i=0; i<256; i++)
            v[i].Save(file);
        auto end = std::chrono::steady_clock::now();
        batch_time += (end - begin);
        // std::cout << nbad << " bad of " << nframes << std::endl;

        file.close();
    }
    auto end_main = std::chrono::steady_clock::now();
    std::cout << "Total elapsed (sec, wall time): " <<
      (std::chrono::duration_cast<std::chrono::milliseconds>(end_main-begin_main)).count() / 1000.0
    << std::endl;
    std::cout << "Total elapsed (sec, processing batch time without saving): " <<
      batch_time_no_save.count() << std::endl;
    std::cout << "Total elapsed (sec, processing batch time with saving): " <<
      batch_time.count() << std::endl;
    // << std::endl;
}
