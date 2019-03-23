# Call Graph Profiler

Collects frequency information for edges in the call graph.


# Building

1. Create a new building directory.

        mkdir cgbuild

2. Change into the new directory.

        cd cgbuild

3. Run CMake with the path to the LLVM source.

        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
            -DLLVM_DIR=</path/to/LLVM/build>/lib/cmake/llvm/ ../callgraph-profiler-template

4. Run make inside the build directory:

        make

When you have successfully completed the project, this will produce a tool
for profiling the callgraph called `bin/callgraph-profiler` along with
supporting libraries in `lib/`.

Note, building with a tool like ninja can be done by adding `-G Ninja` to
the cmake invocation and running ninja instead of make.

# Running

Compile the program to be analyzed compiled to bitcode:

    clang -g -c -emit-llvm ../callgraph-profiler-template/test/test.c -o calls.bc

Running the call graph profiler:

    bin/callgraph-profiler calls.bc -o calls
    ./calls

Running an instrumented program like `./calls` in the above example should produce a file called
`profile-results.csv` in the current directory. The format of the file is:

    <caller function name>, <call site file name>, <call site line #>, <callee function name>, <(call site,callee) frequency>

