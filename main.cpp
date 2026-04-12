#include <iostream>
#include <string>

#include "Processor.h"

using namespace std;

static void dumpCycleMemory(const Processor& cpu, int cycle) {
  cout << "cycle " << cycle << ": ";
  for (int i = 0; i < static_cast<int>(cpu.Memory.size()); i++) {
    cout << cpu.Memory[i] << " ";
  }
  cout << "\n";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: ./main <filename.s>\n";
    return 1;
  }

  ProcessorConfig config;
  Processor cpu(config);

  try {
    cpu.loadProgram(argv[1]);
  } catch (...) {
    cerr << "Failed to parse instruction file.\n";
    return 1;
  }

  dumpCycleMemory(cpu, 0);
  while (true) {
    bool keep_running = cpu.step();
    dumpCycleMemory(cpu, cpu.clock_cycle);
    if (!keep_running) {
      break;
    }
  }

  if (cpu.exception) {
    cout << "\n[+] Execution halted due to exception after " << cpu.clock_cycle
         << " cycles.\n";
  } else {
    cout << "\n[+] Execution complete naturally in " << cpu.clock_cycle
         << " cycles.\n";
  }

  cpu.dumpArchitecturalState();
  for (int i = 0; i < static_cast<int>(cpu.Memory.size()); i++) {
    cout << cpu.Memory[i] << " ";
  }
  cout << "\n";

  return 0;
}
