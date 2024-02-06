# CrowCpp Hasher application

A simple template application for demonstrating a Crowcpp webserver. It generates 32, 64 and 128-bit hashes of input strings using the xxhash hasing library. It is based off this [`string hasher`](https://github.com/Ohjurot/WebStringHasher) which uses using [`cpp-httplib`](https://github.com/yhirose/cpp-httplib 
)(web server), [`jsoncpp`](https://github.com/open-source-parsers/jsoncpp) , [`xxHash`](https://github.com/Cyan4973/xxHash) and  [`inja`](https://github.com/pantor/inja). 




[`CrowCpp`](https://github.com/CrowCpp/Crow) is a comprehensive framework and has built-in libraries for json parsing, html templating, base64 encoding, compression etc. I made use of its built-in Json parser and a templating engine rather than inja and jsoncpp. But CrowCpp is also library agnostic and can be used with custom libraries for each of these functionalities. I provide a [`duplicate source`](src\nlohmann-json-inja-impl.cpp) demonstrating it with Nlohmann Json library and Inja templating engine.

`CrowCpp` built-in Json parsing library is convenient but is not as full featured as more established libraries. But for this appication it is good enough. Then for the HTML templating, `crowcpp` uses [`mustache`](https://mustache.github.io/mustache.5.html) which has a fromal specication with lots of features. It also has a useful [`playground`](https://jgonggrijp.gitlab.io/wontache/playground.html) to explore its templating capabilities.
One exploration, it seems that CrowCpp does not support the full mustache specification and this can be a problem. Specifically [`parents, blocks and dynamic names don't seem to be fully supported`](https://github.com/CrowCpp/Crow/issues/761). If this is fixed, it would make templating more robust and significantly easier.


 `Inja` can be used as an alternative as seen in the duplicate source. Note that they have different language structure.


This is a single page web application that sends requests to the server for all the computation involved. Thus request based. To make a more responsive application, you can integrate this into a React(Insert any JS framework) application or a WASM frontend. I will explore this and update accordingly. A self signed SSL key was used in this project for demonstration.

## Dependencies
```sh
# For main code:
Crowcpp
xxhash


# for duplicate source code:
Crowcpp
Nlohmann Json
Inja
xxhash
```

## Dependency management
Libraries can be installed using a  package and dependency manager ;ile  `vcpkg` or `conan` or from source from their respective github repositaries. A `vcpkg.json` is provided manage local dependency for this project. But it can also use globally installed libraries.

### Using vcpkg with vcpkg.json file
To make use of the vcpkg.json file:
```sh
# navigate to project directory
./path_to_vcpkg/vcpkg install

vcpkg integrate install

# Option 1
# Open your CMakeLists.txt file and add the following line at
# the beginning to specify the toolchain file:
set(CMAKE_TOOLCHAIN_FILE "<vcpkg_directory>/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
mkdir build
cd build
cmake ..
cmake --build .

# Option 2
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="path_to_vcpkg_file/vcpkg.cmake" ..
cmake --build .

# Run (Depending on Generator mode)
./Debug/crow-web-hasher
```
I also provided an alternative [`vcpkg.json`](vcpkg-nlohmann-inja.json) file if you want to run the duplicate code. Uncomment the find_library and link commands in the [`CMakeLists.txt`](./CMakeLists.txt) file.

### Vcpkg Instructions without a vcpkg.json file
Install the following libraries globally using vcpkg:
```
Crowcpp
xxhash
Openssl
```

You can navigate to their [`package list`](https://vcpkg.io/en/packages) to browse features. (and Nlohmann and inja to run the duplicate code.)
```bash
# Navigate to vcpkg directory

./vcpkg install crow
./vcpkg install xxhash
./vcpkg install openssl

# navigate to project path
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="path_to_vcpkg_file/vcpkg.cmake" ..

# build Debug or Release depending on generator
cmake --build .

# Run
./Debug/crow-web-hasher
```

### Using Conan
The above can also be done in Conan package manager. 

### Todo: Benchmark 
Benchmark with Drogon and cpp-httplib?



