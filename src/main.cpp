// SPDX-License-Identifier: MIT

// ultrasf2: convert the music of Nintendo 64 games to MIDI, SoundFont and SFZ.
//
//     ultrasf2 [options] <image.z64 | set.zip | set folder> [...]
//
// Output goes beside each image or set. Windows accepts files dropped on the executable. macOS uses Ultrasf2.app.

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "ntdlib/sequence_export/frame.h"
#include "seqlib/sfz/track_file.h"
#include "ultralib/sequence_export/n64/input.h"
#include "ultralib/sequence_export/n64/instruments.h"

namespace
{

namespace fs = std::filesystem;
namespace frame = ntdlib::sequence_export;
using ultralib::sequence_export::n64::Conversion;
using ultralib::sequence_export::n64::Input;
using ultralib::sequence_export::n64::PerformOptions;

constexpr char kProgram[] = "ultrasf2";

void Usage()
{
    std::string text =
        "ultrasf2: convert the music of Nintendo 64 games to MIDI, SoundFont 2 and SFZ.\n"
        "\n"
        "usage: ultrasf2 [options] <image or set> [...]\n"
        "\n"
        "Give it a game's image (.z64, .n64 or .v64), which may be in a ZIP archive, or\n"
        "an Aleck64 game's ROM set as a ZIP archive or a folder of its chips. Each\n"
        "sequence gets <name>.mid, <name>.sf2, and per-track SFZ files in sfz/<name>.\n"
        "Shared samples go in sfz/samples. Output is written to\n"
        "<image or set name>_ultrasf2 beside it. You can also drop files onto\n"
        "ultrasf2.exe on Windows or Ultrasf2.app on macOS.\n"
        "\n"
        "options:\n";
    text += frame::OutputOnlyHelp("image or set", "0x19");
    text += frame::kLoopsHelp;
    text += "  --seed N       initial game random state (default 0; Paper Mario only)\n";
    text +=
        "  --no-resampler-filter\n"
        "                 write the waves as the game decodes them, without the\n"
        "                 low-pass that matches the console's resampler\n";
    text += frame::kListQuietVersionHelp;

    std::puts(text.c_str());
}

// Reads the image or set at `input` (Input::Open) and prints the input's warnings. A set with the chips of more than
// one game gives one. Returns nothing when no file or folder is there. The frame then reports it.
std::optional<Input> Open(const fs::path& input, const std::string&)
{
    std::error_code error;
    if (!fs::exists(input, error))
    {
        return std::nullopt;
    }

    Input opened = Input::Open(input, kProgram);
    for (const std::string& warning : opened.Warnings())
    {
        std::fprintf(stderr, "warning: %s\n", warning.c_str());
    }

    return opened;
}

} // namespace

int main(int argc, char** argv)
{
    frame::Converter<PerformOptions, Input, Conversion> converter;
    converter.program = kProgram;
    converter.version_line = "ultrasf2 " ULTRASF2_VERSION;
    converter.input_word = "input";
    converter.reads_folders = true;
    converter.samples_folder = seqlib::sfz::kSamplesFolder;
    converter.usage = Usage;
    converter.option = [](const std::string& option, frame::OptionReader&, PerformOptions& perform)
    {
        if (option != "--no-resampler-filter")
        {
            return false;
        }

        perform.resampler_filter = false;

        return true;
    };
    converter.open = Open;
    converter.sequences = std::mem_fn(&Input::Sequences);
    converter.convert = [](Input& input, std::size_t index, const PerformOptions& options)
    {
        PerformOptions named = options;
        named.program = kProgram;
        return input.Convert(index, named);
    };

    const auto run = [&converter](const std::vector<std::string>& args)
    {
        return frame::Run(args, converter);
    };

    return frame::RunMain(argc, argv, run);
}
