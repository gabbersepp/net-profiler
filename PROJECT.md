This is a small starter example that enables you to write your own .NET profiler using the .NET Profiling API.

# Projectstructure
This project consists of three projects:

## ProfilerAtl
The profiler. Don't be afraid of C++ code :-)

## App
This can also be replaced by a normal batch file. It only exists to start the app to be profiled and attach the profiler by using some enviornment variables. 

## ErrorThrowingApp
This represents the app to be profiled. It continuously throws an exception.