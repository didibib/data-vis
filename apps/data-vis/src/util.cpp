#include "precomp.h"

//--------------------------------------------------------------
namespace Random {

	static uint seed = 0x12345678;
	uint UInt()
	{
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 5;
		return seed;
	}

	uint UInt(uint& _seed)
	{
		_seed ^= _seed << 13;
		_seed ^= _seed >> 17;
		_seed ^= _seed << 5;
		return _seed;
	}

	uint Range(uint min, uint max) {
		std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
		return dist(Random::MT19937);
	}

	uint Range(uint max) {
		return Range(0, max);
	}

	float Float() { return UInt() * 2.3283064365387e-10f; }
	float RangeF(float _range) { return Float() * _range; }

}
//--------------------------------------------------------------
void ParseCmdline(const boost::program_options::options_description& _options, std::string _cmdline_input) {
	namespace po = boost::program_options;
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(po::split_unix(_cmdline_input)).options(_options).run(), vm);
	}
	catch (std::exception& e) {
		std::cout << e.what();
	}
	po::notify(vm);
}