#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <memory>
#include <cassert>
#include <cstring>
#include <clang-c/Index.h>
#pragma comment(lib, "libclang.lib")

const char INSERT_THIS[] = "\r\nfriend DEBUGXRAY::DEBUGCLASS;\r\n";
std::string searchForFile;

std::string unwrapCXString(const CXString& str)
{
	auto charptr = clang_getCString(str);
	std::string retval = charptr;
	clang_disposeString(str);
	return retval;
}

struct InterleaveBlock
{
	size_t	offset;				// where to insert to
	const char*	ptr;			// source address (interleave block ptrs are not owned, they can point to the same object)
	size_t	len;				// source length
};

typedef std::pair<std::unique_ptr<char[]>, size_t>	AUTOBUF;

inline AUTOBUF allocateAB(size_t bufSize) { return std::make_pair(std::make_unique<char[]>(bufSize), bufSize); }
inline void releaseAB(AUTOBUF& releaseThis) { releaseThis.first.reset(nullptr); releaseThis.second = 0; }

// inserts interleaves into a larger main block (source), each interleave has an offset member which tells the
// exact position where to insert the interleave block (exactly after taking offset bytes of source block
// -- this way offset = 0 means to put it before everything else and offset = sourceBlkLen means after everything else)
AUTOBUF InsertInterleaves(const char* sourceBlk, size_t sourceBlkLen, const std::vector<InterleaveBlock>& interleaves)
{
	const size_t interleavesSumLen = std::accumulate(interleaves.begin(), interleaves.end(), 0, [](size_t sofar, const InterleaveBlock& blk) {
		return sofar + blk.len;
	});
	const size_t destBlkLen = sourceBlkLen + interleavesSumLen;
	AUTOBUF destBlk = allocateAB(destBlkLen);
	memset(destBlk.first.get(), 0xCC, destBlkLen);
	char* destBlkPtr = destBlk.first.get();
	size_t sourceCursor = 0;
	size_t destCursor = 0;
	for (const InterleaveBlock& blk : interleaves)
	{
		const size_t bytesToCopyFromMainBlock = blk.offset - sourceCursor;										
		std::memcpy(&destBlkPtr[destCursor], &sourceBlk[sourceCursor], bytesToCopyFromMainBlock);				
		sourceCursor += bytesToCopyFromMainBlock;																
		destCursor += bytesToCopyFromMainBlock;																	

		std::memcpy(&destBlkPtr[destCursor], blk.ptr, blk.len);													
		destCursor += blk.len;																					
	}
	const size_t bytesToCopyFromMainBlock = sourceBlkLen - sourceCursor;												
	std::memcpy(&destBlkPtr[destCursor], &sourceBlk[sourceCursor], bytesToCopyFromMainBlock);							
	sourceCursor += bytesToCopyFromMainBlock;																			
	destCursor += bytesToCopyFromMainBlock;																				
	assert(sourceCursor == sourceBlkLen && destCursor == destBlkLen);
	return destBlk;
}

bool file_get_excerpt(const std::string& filename, int64_t startOff, int64_t endOff, AUTOBUF& excerpt, bool text = true)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::in);
	if (endOff == -1)
	{
		ifs.seekg(0, std::ios::end);
		endOff = ifs.tellg();
		endOff--;
		ifs.seekg(0, std::ios::beg);
	}
	const int64_t readLength = endOff - startOff + 1;
	const int64_t allocLength = readLength + (text ? 1 : 0);
	if (readLength < 0)
		return false;

	excerpt = allocateAB(allocLength);
	char* excerptPtr = excerpt.first.get();

	ifs.seekg(startOff);
	ifs.read(excerptPtr,readLength);
	if (!ifs.good())
	{
		releaseAB(excerpt);
		return false;
	}
	if (text)
	{
		excerptPtr[allocLength - 1] = '\0';
	}
	return true;
}

std::vector<InterleaveBlock> interleaves;

CXChildVisitResult visitor(CXCursor c, CXCursor parent, CXClientData client_data)
{
	if (clang_getCursorKind(c) != 4) return CXChildVisit_Recurse;					// only interested in class declarations
	auto location = clang_getCursorLocation(c);
	auto extent = clang_getCursorExtent(c);
	auto loc_from = clang_getRangeStart(extent);
	auto loc_to = clang_getRangeEnd(extent);
	CXFile cxfile;
	unsigned line, column, offset;
	clang_getSpellingLocation(location, &cxfile, &line, &column, &offset);
	clang_getSpellingLocation(loc_from, &cxfile, &line, &column, &offset);
	clang_getSpellingLocation(loc_to, &cxfile, &line, &column, &offset);
	clang_getExpansionLocation(location, &cxfile, &line, &column, &offset);
	clang_getExpansionLocation(loc_from, &cxfile, &line, &column, &offset);
	auto fromOffs = offset;
	clang_getExpansionLocation(loc_to, &cxfile, &line, &column, &offset);
	auto toOffs = offset;
	auto filename = unwrapCXString(clang_getFileName(cxfile));
	if (filename == searchForFile)
	{
		InterleaveBlock ivb{ toOffs - 1, INSERT_THIS, sizeof(INSERT_THIS)-1 };
		AUTOBUF classdecl;
		bool result = file_get_excerpt(filename, fromOffs, toOffs, classdecl);
		if (result)
		{
			if (strstr(classdecl.first.get(), INSERT_THIS) == nullptr)
			{
				interleaves.push_back(ivb);
			}
			else
				std::cout << "\n[[already contained]]\n";
		}
		else
			std::cout << "\n[[error]]\n";
	}

	return CXChildVisit_Recurse;
}

bool file_put_contents(const std::string& filename, const char* bufferptr, size_t buffersize)
{
	std::ofstream ofs(filename.c_str(), std::ios::binary | std::ios::trunc | std::ios::out);
	if (!ofs.good())
		return false;
	ofs.write((char*)bufferptr, buffersize);
	return ofs.good();
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <inputfile> <outputfile>\n";
		exit(-1);
	}
	searchForFile = argv[1];
	std::string saveFile = argv[2];
	CXIndex index = clang_createIndex(0, 0);
	CXTranslationUnit unit = clang_parseTranslationUnit(index, "try.cpp", nullptr, 0, nullptr, 0, CXTranslationUnit_None);
	CXCursor cursor = clang_getTranslationUnitCursor(unit);
	clang_visitChildren(cursor, &visitor, nullptr);
	if (!unit)
	{
		std::cout << "Unable to parse translation unit." << std::endl;
		exit(-2);
	}
	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);
	AUTOBUF contents;
	bool result = file_get_excerpt(searchForFile, 0, -1, contents, false);
	if (!result) 
	{ 
		std::cout << "File read error\n";
		exit(-3);
	}
	AUTOBUF mixed = InsertInterleaves(contents.first.get(), contents.second, interleaves);
	if (file_put_contents(saveFile, mixed.first.get(), mixed.second))
	{
		std::cout << "Saved successfully to " << saveFile << "\n";
	}
	else
	{
		std::cout << "Error saving " << saveFile << "\n";
	}
}