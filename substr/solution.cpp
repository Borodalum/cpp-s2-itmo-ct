#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

unsigned int findMatching(unsigned int i, int symb, const char* text, const unsigned int* prefix) {
  while (i > 0 && symb != text[i]) {
    i = prefix[i - 1];
  }
  if (symb == text[i]) {
    ++i;
  }
  return i;
}

void endOfFunc(unsigned int* prefix, FILE* file) {
  std::fclose(file);
  std::free(prefix);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::fprintf(stderr, "Expected 2 arguments: <filepath> <pattern>\n");
    return EXIT_FAILURE;
  }

  FILE* file = std::fopen(argv[1], "rb");
  if (file == nullptr) {
    std::fprintf(stderr, "Can't open file \"%s\". Reason: %s\n", argv[1], std::strerror(errno));
    return EXIT_FAILURE;
  }

  char* subtext = argv[2];
  size_t patternLen = strlen(subtext);
  void* p = std::malloc(patternLen * sizeof(unsigned int));
  if (p == nullptr) {
    std::fprintf(stderr, "Failed to allocate memory\n");
    std::fclose(file);
    return EXIT_FAILURE;
  }
  unsigned int* prefix = static_cast<unsigned int*>(p);

  // precalculation
  prefix[0] = 0;
  for (size_t i = 1; i < patternLen - 1; ++i) {
    unsigned int k = prefix[i - 1];
    k = findMatching(k, subtext[i], subtext, prefix);
    prefix[i] = k;
  }

  // KMP
  unsigned int j = 0;
  int curSymb = 0;
  while ((curSymb = std::fgetc(file)) != EOF) {
    j = findMatching(j, curSymb, subtext, prefix);
    if (j >= patternLen) {
      std::fprintf(stdout, "Yes\n");
      endOfFunc(prefix, file);
      return EXIT_SUCCESS;
    }
  }

  // read error
  if (std::ferror(file)) {
    std::fprintf(stderr, "Can't read file \"%s\". Reason: %s\n", argv[1], std::strerror(errno));
    endOfFunc(prefix, file);
    return EXIT_FAILURE;
  }

  // write answer if have no matching
  std::fprintf(stdout, "No\n");
  endOfFunc(prefix, file);

  return EXIT_SUCCESS;
}
