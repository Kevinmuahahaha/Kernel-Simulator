#ifndef INCLUDE_UTILS
#define INCLUDE_UTILS
#include <string>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <vector>
class utils{
	public:
		static bool loadSystemwideConfig( std::string filepath );
		static void log(std::string status, std::string class_name, std::string msg, bool enabled=true);
		static void csleep(int sleep_mili_seconds);// cross-platform sleep function

		static inline void ltrim(std::string &s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
						return !std::isspace(ch);
						})); }

		static inline void rtrim(std::string &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
						return !std::isspace(ch);
						}).base(), s.end());
		}

		static inline void trim(std::string &s) {
			ltrim(s);
			rtrim(s);
		}

		static inline std::vector<std::string> split(const std::string needs_split, std::string delimiter = " "){
			std::vector<std::string> ret_vec;
			// std::string delimiter = " ";
			size_t pos = 0;
			std::string s = needs_split;
			std::string token;
			while ((pos = s.find(delimiter)) != std::string::npos) {
				token = s.substr(0, pos);
				ret_vec.push_back( token );
				s.erase(0, pos + delimiter.length());
			}
			ret_vec.push_back(s);
			return ret_vec;
		}

		static inline std::vector<int> makeIntVector(){
			std::vector<int> ret;
			return ret;
		}

	private:
		// returns true on sucess, returns false on failure.
		static inline bool stoi( std::string string_input, int* store_int ){
			std::string::size_type sz;
			int ret = -1;
			try{
				ret = std::stoi (string_input,&sz);
			}
			catch( ... ){
				return false;
			}
			*store_int = ret;
			return true;
		}
};
#endif
