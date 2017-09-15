
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <utility>
#include <string>

extern "C" {


// This macro allows us to prefix strings so that they are less likely to
// conflict with existing symbol names in the examined programs.
// e.g. CGPROF(entry) yields CaLlPrOfIlEr_entry
#define CGPROF(X) CaLlPrOfIlEr_##X

// TODO: Add your runtime library data structures and functions here.

extern char* CGPROF(fn_names)[];

// extern uint64_t CGPROF(num_fn);

class CallInfo {
public:
  uint64_t caller;
  uint64_t callee;
  uint64_t line;
  // std::string file_name;
  bool
  operator==(const struct CallInfo& other) const {
    if (this->caller == other.caller && this->callee == other.callee
        && this->line == other.line) {
      return true;
    }
    return false;
  }

  bool
  operator==(struct CallInfo& other) {
    if (this->caller == other.caller && this->callee == other.callee
        && this->line == other.line) {
      return true;
    }
    return false;
  }

  bool
  operator<(struct CallInfo& other) {
    if (this->caller < other.caller) {
      return true;
    }
    if (this->caller > other.caller) {
      return false;
    }
    if (this->callee < other.callee) {
      return true;
    }
    if (this->callee > other.callee) {
      return false;
    }
    if (this->line < other.line) {
      return true;
    }
    return false;
  }
  bool
  operator<(const struct CallInfo& other) const {
    if (this->caller < other.caller) {
      return true;
    }
    if (this->caller > other.caller) {
      return false;
    }
    if (this->callee < other.callee) {
      return true;
    }
    if (this->callee > other.callee) {
      return false;
    }
    if (this->line < other.line) {
      return true;
    }
    return false;
  }
};

typedef std::map<CallInfo, uint64_t> CounterType;

CounterType* call_counter_ptr;

void
CGPROF(init)() {
  call_counter_ptr = new CounterType();
}

void
CGPROF(count)(uint64_t caller, uint64_t callee, uint64_t line) {
  CounterType &call_counter = *call_counter_ptr;
  CallInfo key;
  key.caller = caller;
  key.callee = callee;
  key.line   = line;

  if (call_counter.find(key) == call_counter.end()) {
    call_counter[key] = 1;
  } else {
    call_counter[key]++;
  }
  printf("%lu %lu %lu = %lu\n",
         key.caller,
         key.callee,
         key.line,
         call_counter[key]);
}

// void
// CGPROF(debug_print)() {
//   printf("=====================\n"
//          "fn_info\n"
//          "=====================\n");
//   for (auto i = 0; i < CGPROF(num_fn); i++) {
//     auto info = CGPROF(fn_info)[i];
//     printf("%lu %s %s %lu\n", i, info.name, info.fname, info.line);
//   }

//   printf("=====================\n"
//          "raw fn calls\n"
//          "=====================\n");
//   printf("map size: %lu\n", call_counter.size());
//   for (auto it = call_counter.begin(); it != call_counter.end(); it++) {
//     auto call_info = it->first;
//     auto freq      = it->second;
//     auto caller_id = call_info.first;
//     auto callee_id = call_info.second;
//     printf("%lu %lu %lu\n", caller_id, callee_id, freq);
//   }
// }

void
CGPROF(print)() {
  CounterType &call_counter = *call_counter_ptr;
  // call_counter[std::make_pair(42, 42)]     = 42;
  // call_counter[std::make_pair(1337, 1337)] = 1337;
  // CGPROF(debug_print)();

  printf("map size: %lu\n", call_counter.size());
  printf("=====================\n"
         "Function Calls\n"
         "=====================\n");
  for (auto it = call_counter.begin(); it != call_counter.end(); it++) {
    auto call_info   = it->first;
    auto freq        = it->second;
    auto caller_name = CGPROF(fn_names)[call_info.caller];
    auto callee_name = CGPROF(fn_names)[call_info.callee];
    printf("%s %s %lu %s %lu\n",
           caller_name,
           "???.c",
           call_info.line,
           callee_name,
           freq);
    // std::cout << call_info.caller << " "
    //           << "???.c"
    //           << " " << call_info.line << " " << call_info.callee << " " <<
    //           freq
    //           << std::endl;
  }
}
}
