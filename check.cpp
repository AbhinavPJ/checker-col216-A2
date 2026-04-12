#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
namespace fs = filesystem;
string quote(const string& s) {
  string out = "\"";
  for (char c : s) {
    if (c == '\\' || c == '"')
      out += '\\';
    out += c;
  }
  out += '"';
  return out;
}

int runCommand(const string& cmd, string& output) {
  FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
  if (!pipe) {
    output = "popen failed";
    return -1;
  }

  output.clear();
  char buf[4096];
  while (fgets(buf, sizeof(buf), pipe) != nullptr) output += buf;

  int status = pclose(pipe);
  if (status == -1)
    return -1;
  return WEXITSTATUS(status);
}

string readFile(const fs::path& p) {
  std::ifstream in(p, std::ios::binary);
  if (!in)
    throw std::runtime_error("Unable to open file: " + p.string());
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

void writeFile(const fs::path& p, const string& content) {
  std::ofstream out(p, std::ios::binary);
  if (!out)
    throw std::runtime_error("Unable to write file: " + p.string());
  out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

bool startsWith(const string& s, const string& prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const string& s, const string& suffix) {
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string sanitizeForFilename(const string& s) {
  string out;
  out.reserve(s.size());
  for (char c : s) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_') {
      out += c;
    } else {
      out += '_';
    }
  }
  if (out.empty())
    out = "dump";
  return out;
}

bool isDigits(const string& s) {
  if (s.empty())
    return false;
  for (char c : s) {
    if (c < '0' || c > '9')
      return false;
  }
  return true;
}

bool isCodeFileName(const string& name) {
  if (!startsWith(name, "code") || !endsWith(name, ".txt"))
    return false;
  string mid = name.substr(4, name.size() - 8);
  return isDigits(mid);
}

int testDirIndex(const fs::path& p) {
  string name = p.filename().string();
  if (!startsWith(name, "test"))
    return -1;
  string n = name.substr(4);
  if (!isDigits(n))
    return -1;
  return std::stoi(n);
}

fs::path ansPathForCode(const fs::path& codePath) {
  string name = codePath.filename().string();
  if (isCodeFileName(name)) {
    string number = name.substr(4, name.size() - 8);
    return codePath.parent_path() / ("ans" + number + ".txt");
  }
  return codePath.parent_path() / (codePath.stem().string() + "_ans.txt");
}

int main(int argc, char** argv) {
  bool writeMode = (argc >= 2 && string(argv[1]) == "--write");
  fs::path checkerRoot = fs::current_path();
  fs::path root = checkerRoot.parent_path();

  vector<fs::path> testDirs;
  for (const auto& entry : fs::directory_iterator(checkerRoot)) {
    if (!entry.is_directory())
      continue;
    if (testDirIndex(entry.path()) != -1)
      testDirs.push_back(entry.path());
  }

  std::sort(testDirs.begin(), testDirs.end(),
            [](const fs::path& a, const fs::path& b) {
              return testDirIndex(a) < testDirIndex(b);
            });

  vector<std::pair<fs::path, fs::path>> cases;
  for (const auto& dir : testDirs) {
    vector<fs::path> codes;
    for (const auto& entry : fs::directory_iterator(dir)) {
      if (!entry.is_regular_file())
        continue;
      if (isCodeFileName(entry.path().filename().string())) {
        codes.push_back(entry.path());
      }
    }

    std::sort(codes.begin(), codes.end());
    for (const auto& code : codes) {
      fs::path ans = ansPathForCode(code);
      if (fs::exists(ans) || writeMode)
        cases.push_back({code, ans});
    }
  }

  if (cases.empty()) {
    std::cout << "No test cases\n";
    return 2;
  }

  string out;
  try {
    fs::copy_file(checkerRoot / "main.cpp", root / "main.cpp",
                  fs::copy_options::overwrite_existing);
    int buildRc = runCommand(
        "make -C " + quote(root.string()) + " compile FILE=main.cpp", out);
    if (buildRc != 0) {
      std::cout << "Build/setup failure\nBuild failed:\n" << out;
      if (!out.empty() && out.back() != '\n')
        std::cout << "\n";
      return 2;
    }
  } catch (const std::exception& e) {
    std::cout << "Build/setup failure\n" << e.what() << "\n";
    return 2;
  }

  int fails = 0;
  for (const auto& tc : cases) {
    fs::path codePath = tc.first;
    fs::path ansPath = tc.second;
    string label = codePath.parent_path().filename().string() + "/" +
                   codePath.filename().string();

    long long now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    fs::path tempDir =
        fs::temp_directory_path() / ("col216_checker_" + std::to_string(now));
    fs::path tempCode = tempDir / codePath.filename();

    try {
      fs::create_directories(tempDir);
      fs::copy_file(codePath, tempCode, fs::copy_options::overwrite_existing);

      int prepRc =
          runCommand("python3 " + quote((root / "compiler.py").string()) + " " +
                         quote(tempCode.string()),
                     out);
      if (prepRc != 0) {
        fails++;
        std::cout << "FAIL " << label << "\n"
                  << "Preprocess failed for " << tempCode.filename().string()
                  << "\n"
                  << out;
        if (!out.empty() && out.back() != '\n')
          std::cout << "\n";
        fs::remove_all(tempDir);
        continue;
      }

      string runOutput;
      int runRc = runCommand(
          quote((root / "main").string()) + " " + quote(tempCode.string()),
          runOutput);
      if (runRc != 0) {
        fails++;
        std::cout << "FAIL " << label << " (main returned " << runRc << ")\n";
        std::cout << runOutput;
        if (!runOutput.empty() && runOutput.back() != '\n')
          std::cout << "\n";
        fs::remove_all(tempDir);
        continue;
      }

      bool hasExpected = fs::exists(ansPath);
      string expected;
      if (hasExpected)
        expected = readFile(ansPath);

      if (hasExpected && runOutput == expected) {
        std::cout << "PASS " << label << "\n";
      } else if (writeMode) {
        writeFile(ansPath, runOutput);
        std::cout << (hasExpected ? "WROTE " : "CREATED ") << label << "\n";
      } else {
        fails++;
        if (!hasExpected) {
          std::cout << "FAIL " << label << " (missing expected output file)\n";
        } else {
          fs::path dumpDir = checkerRoot / "debug_dumps";
          fs::create_directories(dumpDir);
          fs::path dumpfile =
              dumpDir / (sanitizeForFilename(label) + "_dump.txt");
          std::cout << "FAIL " << label
                    << " (output mismatch, actual output written to "
                    << dumpfile.string() << ")\n";
          // dump the actual output to dump.txt
          writeFile(dumpfile, runOutput);
        }
      }
    } catch (const std::exception& e) {
      fails++;
      std::cout << "FAIL " << label << "\n" << e.what() << "\n";
    }

    fs::remove_all(tempDir);
  }

  std::cout << "\nFailures: " << fails << "\n";
  return fails == 0 ? 0 : 1;
}
