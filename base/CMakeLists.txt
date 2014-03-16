
set (CRAWLERBASE_SOURCES
  utility.cpp
  BaseDecoder.cpp
  FSWalker.cpp 
  FSFileStream.cpp
  ZipDecoder.cpp 
  PlainDecoder.cpp 
)

set (CRAWLERBASE_HEADERS 
  ByteReader.h 
  Log.h
  utility.h
  Buffer.h
  BaseDecoder.h
  FSWalker.h 
  FSFileStream.h
  ZipDecoder.h 
  PlainDecoder.h 
)

add_library(crawlerbase  ${CRAWLERBASE_SOURCES} ${CRAWLERBASE_HEADERS})
