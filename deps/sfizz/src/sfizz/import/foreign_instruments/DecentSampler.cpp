// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "DecentSampler.h"
#include "sfizz/Opcode.h"
#include <absl/strings/match.h>
#include <absl/strings/string_view.h>
#include <absl/memory/memory.h>
#include <algorithm>
#include <locale>
#include <sstream>
#include <iostream>

namespace sfz {

DecentSamplerInstrumentFormat& DecentSamplerInstrumentFormat::getInstance()
{
    static DecentSamplerInstrumentFormat format;
    return format;
}

const char* DecentSamplerInstrumentFormat::name() const noexcept
{
    return "DecentSampler instrument";
}

bool DecentSamplerInstrumentFormat::matchesFilePath(const fs::path& path) const
{
    const std::string ext = path.extension().u8string();
    return absl::EqualsIgnoreCase(ext, ".dspreset");
}

std::unique_ptr<InstrumentImporter> DecentSamplerInstrumentFormat::createImporter() const
{
    return absl::make_unique<DecentSamplerInstrumentImporter>();
}

///
std::string DecentSamplerInstrumentImporter::convertToSfz(const fs::path& path) const
{
    std::ostringstream os;
    os.imbue(std::locale::classic());

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    if (!result) {
        std::cerr << "[sfizz] dspreset: cannot load xml\n";
        return {};
    }

    pugi::xml_node rootNode(doc.child("DecentSampler"));
    if (!rootNode) {
        std::cerr << "[sfizz] dspreset: missing <DecentSampler> element\n";
        return {};
    }

    pugi::xml_node uiNode(rootNode.child("ui"));
    const char* image = uiNode.attribute("bgImage").as_string();
    if (image[0] != '\0')
        os << "<control> image=" << image << '\n';

    pugi::xml_node globalNode(rootNode.child("groups"));
    os << "<global>\n";
    emitRegionalOpcodes(os, globalNode);

    for (pugi::xml_node groupNode: globalNode.children("group"))
    {
        os << "<group>\n";
        emitRegionalOpcodes(os, groupNode);

        for (pugi::xml_node sampleNode: groupNode.children("sample"))
        {
            os << "<region>\n";
            emitRegionalOpcodes(os, sampleNode);
        }
    }

    // TODO effects

    return os.str();
}

void DecentSamplerInstrumentImporter::emitRegionalOpcodes(std::ostream& os, pugi::xml_node node) const
{
    std::vector<Opcode> xmlOpcodes;
    xmlOpcodes.reserve(64);

    for (pugi::xml_attribute attr : node.attributes())
        xmlOpcodes.emplace_back(attr.name(), attr.value());

    // a number of opcodes must respect a particular processing order
    // sort opcodes by computing a score to each (lower is first)

    std::sort(
        xmlOpcodes.begin(), xmlOpcodes.end(),
        [](const Opcode& a, const Opcode& b) {
            auto score = [](const Opcode& x) -> int {
                switch (x.lettersOnlyHash) {
                case hash("rootNote"):
                    // rootNote before loNote/hiNote
                    return -1;
                default:
                    return 0;
                }
            };
            return score(a) < score(b);
        });


    for (const Opcode& xmlOpcode : xmlOpcodes) {
        auto convertToInt = [&os, &xmlOpcode](absl::string_view name) {
            int64_t value;
            if (readLeadingInt(xmlOpcode.value, &value))
                os << name << '=' << value << '\n';
        };
        auto convertToReal = [&os, &xmlOpcode](absl::string_view name) {
            double value;
            if (readLeadingFloat(xmlOpcode.value, &value))
                os << name << '=' << value << '\n';
        };
        auto convertToRealEx = [&os, &xmlOpcode](absl::string_view name, double(*conv)(double)) {
            double value;
            if (readLeadingFloat(xmlOpcode.value, &value))
                os << name << '=' << conv(value) << '\n';
        };

        ///
        switch (xmlOpcode.lettersOnlyHash) {
        case hash("volume"):
            {
                absl::string_view unit;
                double value;
                if (readLeadingFloat(xmlOpcode.value, &value, &unit)) {
                    os << ((unit == "dB") ? "volume" : "amplitude")
                       << '=' << value << "\n";
                }
            }
            break;
        case hash("ampVeltrack"):
            convertToReal("amp_veltrack");
            break;
        case hash("path"):
            os << "sample=" << xmlOpcode.value << '\n';
            break;
        case hash("rootNote"):
            convertToInt("key");
            break;
        case hash("loNote"):
            convertToInt("lokey");
            break;
        case hash("hiNote"):
            convertToInt("hikey");
            break;
        case hash("loVel"):
            convertToInt("lovel");
            break;
        case hash("hiVel"):
            convertToInt("hivel");
            break;
        case hash("start"):
            convertToInt("offset");
            break;
        case hash("end"):
            convertToInt("end");
            break;
        case hash("tuning"):
            convertToReal("transpose");
            break;
        case hash("pan"):
            convertToReal("pan");
            break;
        case hash("trigger"):
            os << "trigger=" << xmlOpcode.value << '\n';
            break;
        case hash("onLoCC&"):
            convertToInt("on_locc" + std::to_string(xmlOpcode.parameters[0]));
            break;
        case hash("onHiCC&"):
            convertToInt("on_hicc" + std::to_string(xmlOpcode.parameters[0]));
            break;
        case hash("loopStart"):
            convertToInt("loop_start");
            break;
        case hash("loopEnd"):
            convertToInt("loop_end");
            break;
        case hash("loopCrossfade"):
            convertToRealEx(
                "loop_crossfade",
                [](double xfadeInFrames) -> double {
                    // transform this value to seconds
                    // TODO: probably should get sample rate from the audio file?
                    return xfadeInFrames / 44100.0;
                });
            break;
        case hash("loopCrossfadeMode"):
            // TODO
            break;
        case hash("loopEnabled"):
            os << "loop_mode="
               << ((xmlOpcode.value == "true") ? "loop_continuous" : "no_loop") << "\n";
            break;
        case hash("attack"):
            convertToReal("ampeg_attack");
            break;
        case hash("decay"):
            convertToReal("ampeg_decay");
            break;
        case hash("sustain"):
            convertToRealEx("ampeg_sustain", [](double x) -> double { return 100.0 * x; });
            break;
        case hash("release"):
            convertToReal("ampeg_release");
            break;
        case hash("seqMode"):
            // TODO
            break;
        case hash("seqPosition"):
            convertToInt("seq_position");
            break;
        }
    }
}

const InstrumentFormat* DecentSamplerInstrumentImporter::getFormat() const noexcept
{
    return &DecentSamplerInstrumentFormat::getInstance();
}

} // namespace sfz
