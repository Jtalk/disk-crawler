
set (CRAWLERBASE_SOURCES
  BaseDecoder.cpp
  FSWalker.cpp 
  FSFileStream.cpp
  ZipDecoder.cpp 
  PlainDecoder.cpp 
)

add_library(crawlerbase  ${CRAWLERBASE_SOURCES})
