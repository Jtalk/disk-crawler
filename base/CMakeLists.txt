
set (CRAWLERBASE_SOURCES
  cmdopts.cpp
  fspick.cpp 
  Config.cpp
  Log.cpp
  utility.cpp
  Buffer.cpp
  OverlapBuffer.cpp 
  BaseDecoder.cpp
  ExtFileStream.cpp 
  FATFileStream.cpp 
  FATWalker.cpp 
  FSWalker.cpp 
  FSFileStream.cpp
  INode.cpp 
  ZipDecoder.cpp 
  PlainDecoder.cpp 
  PlainFileStream.cpp 
  PlainWalker.cpp
)

set (CRAWLERBASE_HEADERS
  cmdopts.h 
  fspick.h
  ByteReader.h 
  Config.h
  Log.h
  utility.h
  Buffer.h
  OverlapBuffer.h
  BaseDecoder.h
  ExtFileStream.h
  FATFileStream.h
  FATWalker.h
  FSWalker.h 
  FSFileStream.h
  INode.h
  ZipDecoder.h 
  PlainDecoder.h 
  PlainFileStream.h
  PlainWalker.h
)

add_library(crawlerbase  ${CRAWLERBASE_SOURCES} ${CRAWLERBASE_HEADERS})
