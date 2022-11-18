#include "hexa_engine/tools/Comp.h"

#include <base_lib/BinaryStream.h>
#include <base_lib/Compound.h>
#include <base_lib/File.h>
#include <base_lib/Path.h>

void Tools::Comp::execute(const List<String>& args) {
    if (args.length() >= 1) {
        const String converter_name = args[2];
        if (args.length() >= 2) {
            const Path src = Path(args[1]);
            const Path out = Path(args.length() >= 2 ? Path(args[2]) : src.with_extension("comp"));

            if (src.exists()) {
                Shared<Compound::Convert::IParser> parser = nullptr;

                if (converter_name == "json") parser = MakeShared<Compound::Convert::JSON>();
                // else if (converter_name == "yaml") converter =
                // MakeShared<Compound::Converters::YAML>(); else if (converter_name
                // == "xml") converter = MakeShared<Compound::Converters::XML>();

                if (parser) {
                    Compound::Object json;
                    if (parser->try_parse_object(File::read_file(src), json)) {
                        if (auto writer = BinaryWriter::open(out)) {
                            writer->write(json);
                            writer->close();
                        }

                        verbose("comp", "Conversion done!");
                    } else
                        print_error("comp", "Provided file contains broken compound value");
                } else
                    print_error("comp", "Converter %s is not a valid converter", converter_name.c());
            } else
                print_error("comp", "Unable to read from provided source file");
        } else
            print_error("comp", "Source file path is required");
    } else
        print_error("comp", "Source format is required");
}
