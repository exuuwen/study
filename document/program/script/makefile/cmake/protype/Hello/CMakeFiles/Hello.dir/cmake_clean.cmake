FILE(REMOVE_RECURSE
  "CMakeFiles/Hello.dir/hello.cxx.o"
  "libHello.pdb"
  "libHello.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang CXX)
  INCLUDE(CMakeFiles/Hello.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
