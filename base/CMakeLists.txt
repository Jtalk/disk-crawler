
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
  FSFileStream.cpp
  INode.cpp 
  SignatureWalker.cpp 
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
  FSWalker.h
  FSFileStream.h
  INode.h
  SignatureWalker.h 
  ZipDecoder.h 
  PlainDecoder.h 
  PlainFileStream.h
  PlainWalker.h
)

add_library(crawlerbase  ${CRAWLERBASE_SOURCES} ${CRAWLERBASE_HEADERS})
