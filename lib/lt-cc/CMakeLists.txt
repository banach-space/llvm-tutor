add_library(lt-cc-static STATIC StaticCallCounter.cpp DynamicCallCounter.cpp)
add_library(lt-cc-shared MODULE StaticCallCounter.cpp DynamicCallCounter.cpp)
