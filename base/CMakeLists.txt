
set (CRAWLERBASE_SOURCES
  BaseDecoder.cpp
  FSWalker.cpp 
  FSFileStream.cpp
  ZipDecoder.cpp 
  PlainDecoder.cpp 
)

set (CRAWLERBASE_HEADERS 
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
