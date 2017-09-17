
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
extern uint64_t CGPROF(id_addr_map)[];
extern uint64_t CGPROF(num_fn);

class CallInfo {
public:
  uint64_t caller;
  uint64_t callee;
  uint64_t line;
  std::string fname;
  bool
  operator==(const struct CallInfo& other) const {
    if (this->caller == other.caller && this->callee == other.callee
        && this->line == other.line
        && this->fname == other.fname) {
      return true;
    }
    return false;
  }

  bool
  operator==(struct CallInfo& other) {
    if (this->caller == other.caller && this->callee == other.callee
        && this->line == other.line
        && this->fname == other.fname) {
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
    if (this->line > other.line) {
      return false;
    }
    if (this->fname < other.fname) {
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
    if (this->line > other.line) {
      return false;
    }
    if (this->fname < other.fname) {
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
CGPROF(count)(uint64_t caller, uint64_t callee, uint64_t line, char* fname) {
  CounterType& call_counter = *call_counter_ptr;
  CallInfo key;
  key.caller = caller;
  key.callee = callee;
  key.line   = line;
  key.fname  = std::string(fname);

  if (call_counter.find(key) == call_counter.end()) {
    call_counter[key] = 1;
  } else {
    call_counter[key]++;
  }
}

void
CGPROF(handle_fp)(uint64_t caller,
                  uint64_t callee_addr,
                  uint64_t line,
                  char* fname) {
  // printf("fp call at %lu\n", callee_addr);
  uint64_t callee;
  for (auto i = 0; i < CGPROF(num_fn); i++) {
    if (CGPROF(id_addr_map)[i] == callee_addr) {
      callee = i;
      CGPROF(count)(caller, callee, line, fname);
      return;
    }
  }
  printf("unknown fp call\n");
}


void
CGPROF(debug_print)() {
  printf("=====================\n"
         "id_addr_map\n"
         "=====================\n");
  for (auto i = 0; i < CGPROF(num_fn); i++) {
    printf("%lu: %lu\n", i, CGPROF(id_addr_map)[i]);
  }
}

void
CGPROF(print)() {
  CounterType& call_counter = *call_counter_ptr;
  // CGPROF(debug_print)();

  // printf("map size: %lu\n", call_counter.size());
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
           call_info.fname.c_str(),
           call_info.line,
           callee_name,
           freq);
  }
}
}
