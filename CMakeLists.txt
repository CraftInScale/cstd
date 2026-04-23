cmake_minimum_required (VERSION 3.12)

project ("cstd")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable (cstd
  collections/BinaryChunkVec.h
  collections/BinaryChunkVec.cpp
  collections/ChunkVec.h
  collections/ChunkVec.cpp
  collections/ConcurrentHashMap.h
  collections/ConcurrentHashMap.cpp
  collections/ConcurrentLinkedList.h
  collections/ConcurrentLinkedList.cpp
  collections/ConcurrentLinkedListIter.h
  collections/ConcurrentLinkedListIter.cpp
  collections/HashMap.h
  collections/HashMap.cpp
  collections/HashSet.h
  collections/HashSet.cpp
  collections/LinkedList.h
  collections/LinkedList.cpp
  collections/LinkedListIter.h
  collections/LinkedListIter.cpp
  collections/Queue.h
  collections/Queue.cpp
  collections/QueueConcurrentIter.h
  collections/QueueConcurrentIter.cpp
  collections/QueueIter.h
  collections/QueueIter.cpp
  collections/Vec.h
  collections/Vec.cpp
  collections/common.h
  collections/common.cpp
  collections/hash/hash.h
  collections/hash/hash.cpp
  collections/hash/md5.h
  collections/hash/md5.cpp
  collections/hash/murmur3.h
  collections/hash/murmur3.cpp
  error.h
  error.cpp
  io/FileDescriptorReader.h
  io/FileDescriptorReader.cpp
  io/FileDescriptorWriter.h
  io/FileDescriptorWriter.cpp
  io/common.h
  io/fs/File.h
  io/fs/File.cpp
  io/fs/FileOptions.h
  io/fs/FileOptions.cpp
  io/fs/FileReader.h
  io/fs/FileReader.cpp
  io/fs/Path.h
  io/fs/Path.cpp
  io/net/NetAddress.h
  io/net/NetAddress.cpp
  io/net/TcpListener.h
  io/net/TcpListener.cpp
  io/net/TcpSocket.h
  io/net/TcpSocket.cpp
  math.h
  math.cpp
  mem.h
  mem.cpp
  minmax.h
  minmax.cpp
  rand.h
  rand.cpp
  rangecheck.h
  rangecheck.cpp
  string/String.h
  string/String.cpp
  string/StringConversion.h
  string/StringConversion.cpp
  string/StringIter.h
  string/StringIter.cpp
  string/Utf8Util.h
  string/Utf8Util.cpp
  thread/Mutex.h
  thread/Mutex.cpp
  thread/Thread.h
  thread/Thread.cpp
  thread/ThreadPool.h
  thread/ThreadPool.cpp
  time/DateTime.h
  time/DateTime.cpp
  time/Duration.h
  time/Timestamp.h
  time/Timestamp.cpp
  time/Timer.h
  time/Timer.cpp
  log/Logger.h
  log/Logger.cpp
 "cstd.h" 
 "cstd.cpp"
 )

target_link_libraries(cstd Threads::Threads)

set_property(TARGET cstd PROPERTY CXX_STANDARD 20)
