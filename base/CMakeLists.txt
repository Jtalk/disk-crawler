
set (CRAWLERBASE_SOURCES
  cmdopts.cpp 
  Config.cpp
  Log.cpp
  utility.cpp
  Buffer.cpp
  OverlapBuffer.cpp 
  BaseDecoder.cpp
  FSWalker.cpp 
  FSFileStream.cpp
  ZipDecoder.cpp 
  PlainDecoder.cpp 
)

set (CRAWLERBASE_HEADERS
  cmdopts.h 
  ByteReader.h 
  Config.h
  Log.h
  utility.h
  Buffer.h
  OverlapBuffer.h
  BaseDecoder.h
  FSWalker.h 
  FSFileStream.h
  ZipDecoder.h 
  PlainDecoder.h 
)

add_library(crawlerbase  ${CRAWLERBASE_SOURCES} ${CRAWLERBASE_HEADERS})
