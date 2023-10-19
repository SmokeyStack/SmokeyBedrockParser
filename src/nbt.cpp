#include "nbt.h"
#include "logger.h"

namespace smokey_bedrock_parser {
	int32_t ParseNbt(char header, char buffer, int32_t buffer_length, NbtTagList tag_list) {
		/*
		int32_t indent = 0;
		log::trace("{}NBT Decode Start", makeIndent(indent, header);
		// these help us look at dumped nbt data and match up LIST's and COMPOUND's
		globalNbtListNumber = 0;
		globalNbtCompoundNumber = 0;

		std::istringstream is(std::string(buf, bufLen));
		nbt::io::stream_reader reader(is, endian::little);

		// remove all elements from taglist
		tagList.clear();

		// read all tags
		MyNbtTag t;
		bool done = false;
		std::istream& pis = reader.get_istr();
		while (!done && (pis) && (!pis.eof())) {
			try {
				// todo emplace_back?
				tagList.push_back(reader.read_tag());
			}
			catch (std::exception& e) {
				// check for eof which means all is well
				if (!pis.eof()) {
					log::error("NBT exception: ({}) (eof={}) (is={}) (tc={}) (pos={}) (buflen={}) (parseNbt)"
						, e.what()
						, pis.eof() ? "true" : "false"
						, (pis) ? "true" : "false"
						, (int)tagList.size()
						, (int)pis.tellg()
						, bufLen
					);
					// todo - testing
					//dumpBuffer("nbt-buffer", buf, bufLen);
				}
				done = true;
			}
		}

		// iterate over the tags
		for (const auto& itt : tagList) {
			parseNbtTag(hdr, indent, itt);
		}
		log::trace("{}NBT Decode End ({} tags)", makeIndent(indent, hdr), tagList.size());
		*/
		return 0;
	}
}