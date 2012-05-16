#ifndef _OSTREAM_FILE_STDOUT_H_
#define _OSTREAM_FILE_STDOUT_H_

#include <stdexcept>
#include <string>
#include <ostream>
#include <fstream>

typedef std::basic_ostream< char, std::char_traits< char > > OstreamFileStdout_Base;

class OstreamFileStdout : public OstreamFileStdout_Base
{
	public:
		OstreamFileStdout(const std::string& outputFilename)
			: OstreamFileStdout_Base(std::cout.rdbuf())
		{
			if (outputFilename != "-") {
				mOssFile.open(outputFilename, std::ios_base::out|std::ios_base::trunc);
				this->rdbuf(&mOssFile);
				if (!mOssFile.is_open()) {
					throw std::runtime_error("ERROR: Can not open file '"+outputFilename+"' for writing");
				}
			}
		}

		virtual ~OstreamFileStdout() {
			if (mOssFile.is_open()) {
				mOssFile.close();
			}
		}
	protected:
		std::filebuf mOssFile;
};

#endif
